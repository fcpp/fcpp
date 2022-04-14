// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file time.hpp
 * @brief Collection of field calculus time evolution functions.
 */

#ifndef FCPP_COORDINATION_TIME_H_
#define FCPP_COORDINATION_TIME_H_

#include <cmath>

#include <algorithm>
#include <limits>

#include "lib/coordination/utils.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief A counter increasing by a given amount at every round, starting from a given amount.
template <typename node_t, typename A, typename B>
inline A counter(node_t& node, trace_t call_point, A const& a, B const& b) {
    return old(node, call_point, (A)b, [&](A x) -> A{
        return x+a;
    });
}

//! @brief A counter increasing by a given amount at every round.
template <typename node_t, typename A>
inline A counter(node_t& node, trace_t call_point, A const& a) {
    return counter(node, call_point, a, A{});
}

//! @brief A counter increasing by one at every round.
template <typename node_t>
inline int counter(node_t& node, trace_t call_point) {
    return counter(node, call_point, 1, 0);
}

//! @brief Export list for counter.
template <typename A = int, typename B = A> using counter_t = common::export_list<A>;


//! @brief Number of rounds elapsed since the last true `value`.
template <typename node_t>
inline int round_since(node_t& node, trace_t call_point, bool value) {
    return value ? 0 : counter(node, call_point);
}

//! @brief Export list for round_since.
using round_since_t = common::export_list<int>;

//! @brief Time elapsed since the last true `value`.
template <typename node_t>
inline times_t time_since(node_t& node, trace_t call_point, bool value) {
    return value ? 0 : counter(node, call_point, node.current_time() - node.previous_time(), 0);
}

//! @brief Export list for time_since.
using time_since_t = common::export_list<times_t>;


//! @brief Makes a varying value constant after a given time `t`.
template <typename node_t, typename T>
inline T constant_after(node_t& node, trace_t call_point, T value, times_t t) {
    return old(node, call_point, value, [&](T o){
        return node.current_time() < t ? value : o;
    });
}

//! @brief Export list for constant_after.
template <typename T> using constant_after_t = common::export_list<T>;

//! @brief Makes a varying value constant.
template <typename node_t, typename T>
inline T constant(node_t& node, trace_t call_point, T value) {
    return old(node, call_point, value, [&](T o){
        return o;
    });
}

//! @brief Export list for constant.
template <typename T> using constant_t = common::export_list<T>;


//! @brief Toggles a variable with a starting point when `change` holds.
template <typename node_t>
bool toggle(node_t& node, trace_t call_point, bool change, bool start = false) {
    return old(node, call_point, start, [&](bool o){
        return o != change;
    });
}

//! @brief Export list for toggle.
using toggle_t = common::export_list<bool>;

//! @brief Toggles a variable with a starting point whenever `change` becomes true.
template <typename node_t>
inline bool toggle_filter(node_t& node, trace_t call_point, bool change, bool start = false) {
    return (old(node, call_point, (int8_t)start, [&](int8_t o) -> int8_t{
        return (((o&1) == 1) != (change and not ((o&2) == 2))) + 2 * change;
    })&1) == 1;
}

//! @brief Export list for toggle_filter.
using toggle_filter_t = common::export_list<int8_t>;


//! @brief Delays a value by `n` rounds (or as much as possibile given the last output value).
template <typename node_t, typename T>
T delay(node_t& node, trace_t call_point, T value, size_t n) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    for (internal::trace_cycle i{node.stack_trace}; i <= n; ++i) {
        T newval = old(node, i, value);
        if (i < n) value = newval;
    }
    return value;
}

//! @brief Export list for delay.
template <typename T> using delay_t = common::export_list<T>;


//! @brief An exponential filter dampening changes of a value across time.
template <typename node_t, typename U, typename T>
inline T exponential_filter(node_t& node, trace_t call_point, U initial, T value, real_t factor) {
    return old(node, call_point, (T)initial, [&](T x){
        return value + (1-factor) * (x - value);
    });
}

//! @brief An exponential filter dampening changes of a value across time.
template <typename node_t, typename T>
inline T exponential_filter(node_t& node, trace_t call_point, T value, real_t factor) {
    return exponential_filter(node, call_point, value, value, factor);
}

//! @brief Export list for exponential_filter.
template <typename T, typename U = T> using exponential_filter_t = common::export_list<T>;


//! @brief An exponential filter dampening changes of a value across time and space.
template <typename node_t, typename U, typename T>
T shared_filter(node_t& node, trace_t call_point, U initial, T value, real_t factor) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return old(node, 0, (T)initial, [&](T x){
        return value + (1-factor) * mean_hood(node, 0, nbr(node, 1, x) - nbr(node, 2, value), x - value);
    });
}

//! @brief An exponential filter dampening changes of a value across time and space.
template <typename node_t, typename T>
inline T shared_filter(node_t& node, trace_t call_point, T value, real_t factor) {
    return shared_filter(node, call_point, value, value, factor);
}

//! @brief Export list for shared_filter.
template <typename T, typename U = T> using shared_filter_t = common::export_list<T>;

//! @brief An exponential decay fading from initial to value across time and space.
template <typename node_t, typename U, typename T>
T shared_decay(node_t& node, trace_t call_point, U initial, T value, real_t factor) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, (T)initial, [&](field<T> x){
        return value + (1-factor) * mean_hood(node, 0, x - nbr(node, 1, value));
    });
}

//! @brief Export list for shared_decay.
template <typename T, typename U = T> using shared_decay_t = common::export_list<T>;


//! @brief Persists a non-null value for a given time.
template <typename node_t, typename T>
T timed_decay(node_t& node, trace_t call_point, T value, T null, times_t dt) {
    using tuple_type = tuple<T, times_t>;

    tuple_type v(value, node.current_time());
    return get<0>(old(node, call_point, v, [&](tuple_type o){
        return get<0>(v) == null and node.current_time() < get<1>(o) + dt ? o : v;
    }));
}

//! @brief Export list for timed_decay.
template <typename T> using timed_decay_t = common::export_list<tuple<T,times_t>>;


//! @brief Maintains a shared clock across the network.
template <typename node_t>
inline times_t shared_clock(node_t& node, trace_t call_point) {
    return nbr(node, call_point, times_t{0}, [&](field<times_t> x){
        return max_hood(node, call_point, node.previous_time() == TIME_MIN ? node.current_time() : x + node.nbr_lag());
    });
}

//! @brief Export list for shared_clock.
using shared_clock_t = common::export_list<times_t>;


}


}

#endif // FCPP_COORDINATION_TIME_H_
