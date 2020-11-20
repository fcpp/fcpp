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
inline B counter(node_t& node, trace_t call_point, A&& a, B&& b) {
    return old(node, call_point, b, [&](B x) -> B{
        return x+a;
    });
}

//! @brief A counter increasing by a given amount at every round.
template <typename node_t, typename A>
inline A counter(node_t& node, trace_t call_point, A&& a) {
    return counter(node, call_point, a, A{});
}

//! @brief A counter increasing by one at every round.
template <typename node_t>
inline int counter(node_t& node, trace_t call_point) {
    return counter(node, call_point, 1, 0);
}


//! @brief An exponential filter dampening changes of a value across time.
template <typename node_t, typename T>
T exponential_filter(node_t& node, trace_t call_point, T initial, T value, double factor) {
    return old(node, call_point, initial, [&](T x){
        return value + (1-factor) * (x - value);
    });
}

//! @brief An exponential filter dampening changes of a value across time.
template <typename node_t, typename T>
inline T exponential_filter(node_t& node, trace_t call_point, T value, double factor) {
    return exponential_filter(node, call_point, value, value, factor);
}


//! @brief An exponential filter dampening changes of a value across time and space.
template <typename node_t, typename T>
T shared_filter(node_t& node, trace_t call_point, T initial, T value, double factor) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return old(node, 0, initial, [&](T x){
        return value + (1-factor) * mean_hood(node, 0, nbr(node, 1, x) - nbr(node, 2, value), x - value);
    });
}

//! @brief An exponential filter dampening changes of a value across time and space.
template <typename node_t, typename T>
T shared_filter(node_t& node, trace_t call_point, T value, double factor) {
    return shared_filter(node, call_point, value, value, factor);
}

//! @brief An exponential decay fading from initial to value across time and space.
template <typename node_t, typename T>
T shared_decay(node_t& node, trace_t call_point, T initial, T value, double factor) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, initial, [&](field<T> x){
        return value + (1-factor) * mean_hood(node, 0, x - nbr(node, 1, value));
    });
}


//! @brief Maintains a shared clock across the network.
template <typename node_t>
times_t shared_clock(node_t& node, trace_t call_point) {
    return nbr(node, call_point, 0, [&](field<times_t> x){
        return max_hood(node, call_point, node.previous_time() == TIME_MIN ? node.current_time() : x + node.nbr_lag());
    });
}


}


}

#endif // FCPP_COORDINATION_TIME_H_
