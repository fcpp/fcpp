// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file utils.hpp
 * @brief Collection of field calculus utility functions.
 */

#ifndef FCPP_COORDINATION_UTILS_H_
#define FCPP_COORDINATION_UTILS_H_

#include <cmath>

#include <algorithm>
#include <limits>

#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Reduces a field to a single value by minimum.
template <typename node_t, typename A>
inline to_local<A> min_hood(node_t& node, trace_t call_point, const A& a) {
    return fold_hood(node, call_point, [] (const to_local<A>& x, const to_local<A>& y) -> to_local<A> {
        return std::min(x, y);
    }, a);
}

//! @brief Reduces a field to a single value by minimum with a default value for self.
template <typename node_t, typename A, typename B>
inline to_local<A> min_hood(node_t& node, trace_t call_point, const A& a, const B& b) {
    return fold_hood(node, call_point, [] (const to_local<A>& x, const to_local<A>& y) -> to_local<A> {
        return std::min(x, y);
    }, a, b);
}


//! @brief Reduces a field to a single value by maximum.
template <typename node_t, typename A>
inline to_local<A> max_hood(node_t& node, trace_t call_point, const A& a) {
    return fold_hood(node, call_point, [] (const to_local<A>& x, const to_local<A>& y) -> to_local<A> {
        return std::max(x, y);
    }, a);
}

//! @brief Reduces a field to a single value by maximum with a default value for self.
template <typename node_t, typename A, typename B>
inline to_local<A> max_hood(node_t& node, trace_t call_point, const A& a, const B& b) {
    return fold_hood(node, call_point, [] (const to_local<A>& x, const to_local<A>& y) -> to_local<A> {
        return std::max(x, y);
    }, a, b);
}


//! @brief Reduces a field to a single value by addition.
template <typename node_t, typename A>
inline to_local<A> sum_hood(node_t& node, trace_t call_point, const A& a) {
    return fold_hood(node, call_point, [] (const to_local<A>& x, const to_local<A>& y) -> to_local<A> {
        return x + y;
    }, a);
}

//! @brief Reduces a field to a single value by addition with a default value for self.
template <typename node_t, typename A, typename B>
inline to_local<A> sum_hood(node_t& node, trace_t call_point, const A& a, const B& b) {
    return fold_hood(node, call_point, [] (const to_local<A>& x, const to_local<A>& y) -> to_local<A> {
        return x + y;
    }, a, b);
}


//! @brief A counter increasing by one at every round.
template <typename node_t>
inline int counter(node_t& node, trace_t call_point) {
    return counter(node, call_point, 1, 0);
}

//! @brief A counter increasing by a given amount at every round.
template <typename node_t, typename A>
inline A counter(node_t& node, trace_t call_point, A&& a) {
    return counter(node, call_point, a, A{});
}

//! @brief A counter increasing by a given amount at every round, starting from a given amount.
template <typename node_t, typename A, typename B>
inline auto counter(node_t& node, trace_t call_point, A&& a, B&& b) {
    return old(node, call_point, b, [&](int x){
        return x+a;
    });
}


//! @brief Pointwise check for infinite values.
field<bool> isinf(const field<double>& f) {
    return details::map_hood([](double x){
        return std::isinf(x);
    }, f);
}

//! @brief Pointwise check for not-a-number values.
field<bool> isnan(const field<double>& f) {
    return details::map_hood([](double x){
        return std::isnan(x);
    }, f);
}

//! @brief Pointwise check for finite values.
field<bool> isfinite(const field<double>& f) {
    return details::map_hood([](double x){
        return std::isfinite(x);
    }, f);
}

//! @brief Pointwise check for normal values (finite, non-zero and not sub-normal).
field<bool> isnormal(const field<double>& f) {
    return details::map_hood([](double x){
        return std::isnormal(x);
    }, f);
}


}


}

#endif // FCPP_COORDINATION_UTILS_H_
