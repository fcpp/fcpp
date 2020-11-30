// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

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


//! @brief Number of rounds elapsed since the last true `value`.
template <typename node_t>
inline int round_since(node_t& node, trace_t call_point, bool value) {
    return value ? 0 : counter(node, call_point);
}

//! @brief Time elapsed since the last true `value`.
template <typename node_t>
inline times_t time_since(node_t& node, trace_t call_point, bool value) {
    return value ? 0 : counter(node, call_point, node.current_time() - node.previous_time(), 0);
}


//! @brief Makes a varying value constant after a given time `t`.
template <typename node_t, typename T>
inline T constant_after(node_t& node, trace_t call_point, T value, times_t t) {
    return old(node, call_point, value, [&](T o){
        return node.current_time() < t ? value : o;
    });
}

//! @brief Makes a varying value constant.
template <typename node_t, typename T>
inline T constant(node_t& node, trace_t call_point, T value) {
    return old(node, call_point, value, [&](T o){
        return o;
    });
}


//! @brief Toggles a variable with a starting point when `change` holds.
template <typename node_t>
bool toggle(node_t& node, trace_t call_point, bool change, bool start = false) {
    return old(node, call_point, start, [&](bool o){
        return o != change;
    });
}

//! @brief Toggles a variable with a starting point whenever `change` becomes true.
template <typename node_t>
inline bool toggle_filter(node_t& node, trace_t call_point, bool change, bool start = false) {
    return (old(node, call_point, (int8_t)start, [&](int8_t o) -> int8_t{
        return (((o&1) == 1) != (change and not ((o&2) == 2))) + 2 * change;
    })&1) == 1;
}


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

//! @brief An exponential decay fading from initial to value across time and space.
template <typename node_t, typename U, typename T>
T shared_decay(node_t& node, trace_t call_point, U initial, T value, real_t factor) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, (T)initial, [&](field<T> x){
        return value + (1-factor) * mean_hood(node, 0, x - nbr(node, 1, value));
    });
}


//! @brief Maintains a shared clock across the network.
template <typename node_t>
inline times_t shared_clock(node_t& node, trace_t call_point) {
    return nbr(node, call_point, times_t{0}, [&](field<times_t> x){
        return max_hood(node, call_point, node.previous_time() == TIME_MIN ? node.current_time() : x + node.nbr_lag());
    });
}


}


}

#endif // FCPP_COORDINATION_TIME_H_
