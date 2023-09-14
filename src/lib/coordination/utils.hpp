// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file utils.hpp
 * @brief Collection of field calculus utility functions.
 */

#ifndef FCPP_COORDINATION_UTILS_H_
#define FCPP_COORDINATION_UTILS_H_

#include <cmath>

#include <algorithm>
#include <limits>

#include "lib/common/algorithm.hpp"
#include "lib/coordination/basics.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @name mux
 *
 * Multiplexer operator, choosing between its arguments based on the value of the first
 * (always evaluating both arguments).
 */
//! @{

//! @brief local guard, arguments of same type
template <typename A>
A const& mux(bool b, A const& x, A const& y) {
    return b ? x : y;
}
//! @brief local guard, moving arguments of same type
template <typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
A mux(bool b, A&& x, A&& y) {
    return b ? std::move(x) : std::move(y);
}
//! @brief local guard, arguments of different but compatible types
template <typename A, typename B, typename = std::enable_if_t<std::is_same<to_local<A>, to_local<B>>::value and not std::is_same<A,B>::value>>
to_field<A> mux(bool b, A const& x, B const& y) {
    return b ? to_field<A>{x} : to_field<A>{y};
}
//! @brief field guard
template <typename A, typename B, typename = std::enable_if_t<std::is_same<to_local<A>, to_local<B>>::value>>
to_field<A> mux(field<bool> b, A const& x, B const& y) {
    return map_hood([] (bool b, to_local<A> x, to_local<B> y) -> to_local<A> {
        return b ? x : y;
    }, b, x, y);
}

//! @}


//! @brief Maximum between two local values.
template <typename A, typename = if_local<A>>
inline A const& max(A const& x, A const& y) {
    return std::max(x, y);
}

//! @brief Maximum between two field values.
template <typename A, typename B, typename = std::enable_if_t<common::has_template<field, tuple<A,B>> and std::is_same<to_local<A>, to_local<B>>::value>>
inline to_field<A> max(A const& x, B const& y) {
    return map_hood([] (to_local<A> x, to_local<B> y) -> to_local<A> {
        return std::max(x, y);
    }, x, y);
}

//! @brief Minimum between two local values.
template <typename A, typename = if_local<A>>
inline A const& min(A const& x, A const& y) {
    return std::min(x, y);
}

//! @brief Minimum between two field values.
template <typename A, typename B, typename = std::enable_if_t<common::has_template<field, tuple<A,B>> and std::is_same<to_local<A>, to_local<B>>::value>>
inline to_field<A> min(A const& x, B const& y) {
    return map_hood([] (to_local<A> x, to_local<B> y) -> to_local<A> {
        return std::min(x, y);
    }, x, y);
}


//! @brief Extracts a component from a field of tuple-like structures.
template <size_t n, typename A>
auto get(field<A> const& f) {
    return map_hood([] (A const& x) {
        return get<n>(x);
    }, f);
}


//! @brief Rounding.
using std::round;

//! @brief Pointwise rounding.
inline field<real_t> round(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::round(x);
    }, f);
}

//! @brief Floor rounding.
using std::floor;

//! @brief Pointwise floor rounding.
inline field<real_t> floor(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::floor(x);
    }, f);
}

//! @brief Ceil rounding.
using std::ceil;

//! @brief Pointwise ceil rounding.
inline field<real_t> ceil(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::ceil(x);
    }, f);
}

//! @brief Natural logarithm.
using std::log;

//! @brief Pointwise natural logarithm.
inline field<real_t> log(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::log(x);
    }, f);
}

//! @brief Natural exponentiation.
using std::exp;

//! @brief Pointwise natural exponentiation.
inline field<real_t> exp(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::exp(x);
    }, f);
}

//! @brief Square root.
using std::sqrt;

//! @brief Pointwise square root.
inline field<real_t> sqrt(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::sqrt(x);
    }, f);
}

//! @brief Power.
using std::pow;

//! @name Pointwise power.
//! @{

//! @brief field arguments
inline field<real_t> pow(field<real_t> const& base, field<real_t> const& exponent) {
    return map_hood([](real_t x, real_t y){
        return std::pow(x, y);
    }, base, exponent);
}
//! @brief real base, field exponent
inline field<real_t> pow(real_t base, field<real_t> const& exponent) {
    return map_hood([](real_t x, real_t y){
        return std::pow(x, y);
    }, base, exponent);
}
//! @brief field base, real exponent
inline field<real_t> pow(field<real_t> const& base, real_t exponent) {
    return map_hood([](real_t x, real_t y){
        return std::pow(x, y);
    }, base, exponent);
}

//! @}

//! @brief Check for infinite values.
using std::isinf;

//! @brief Pointwise check for infinite values.
inline field<bool> isinf(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::isinf(x);
    }, f);
}

//! @brief Check for not-a-number values.
using std::isnan;

//! @brief Pointwise check for not-a-number values.
inline field<bool> isnan(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::isnan(x);
    }, f);
}

//! @brief Check for finite values.
using std::isfinite;

//! @brief Pointwise check for finite values.
inline field<bool> isfinite(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::isfinite(x);
    }, f);
}

//! @brief Check for normal values (finite, non-zero and not sub-normal).
using std::isnormal;

//! @brief Pointwise check for normal values (finite, non-zero and not sub-normal).
inline field<bool> isnormal(field<real_t> const& f) {
    return map_hood([](real_t x){
        return std::isnormal(x);
    }, f);
}


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Reduces a field to a single value by logical and.
template <typename node_t, typename A>
inline to_local<A> all_hood(node_t& node, trace_t call_point, A const& a) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return x and y;
    }, a);
}

//! @brief Reduces a field to a single value by logical and, with a given value for self.
template <typename node_t, typename A, typename B>
inline to_local<A> all_hood(node_t& node, trace_t call_point, A const& a, B const& b) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return x and y;
    }, a, b);
}


//! @brief Reduces a field to a single value by logical or.
template <typename node_t, typename A>
inline to_local<A> any_hood(node_t& node, trace_t call_point, A const& a) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return x or y;
    }, a);
}

//! @brief Reduces a field to a single value by logical or, with a given value for self.
template <typename node_t, typename A, typename B>
inline to_local<A> any_hood(node_t& node, trace_t call_point, A const& a, B const& b) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return x or y;
    }, a, b);
}


//! @brief Reduces a field to a single value by minimum.
template <typename node_t, typename A>
inline to_local<A> min_hood(node_t& node, trace_t call_point, A const& a) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return std::min(x, y);
    }, a);
}

//! @brief Reduces a field to a single value by minimum with a given value for self.
template <typename node_t, typename A, typename B>
inline to_local<A> min_hood(node_t& node, trace_t call_point, A const& a, B const& b) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return std::min(x, y);
    }, a, b);
}


//! @brief Reduces a field to a single value by maximum.
template <typename node_t, typename A>
inline to_local<A> max_hood(node_t& node, trace_t call_point, A const& a) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return std::max(x, y);
    }, a);
}

//! @brief Reduces a field to a single value by maximum with a given value for self.
template <typename node_t, typename A, typename B>
inline to_local<A> max_hood(node_t& node, trace_t call_point, A const& a, B const& b) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return std::max(x, y);
    }, a, b);
}


//! @brief Reduces a field to a single value by addition.
template <typename node_t, typename A>
inline to_local<A> sum_hood(node_t& node, trace_t call_point, A const& a) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return x + y;
    }, a);
}

//! @brief Reduces a field to a single value by addition with a given value for self.
template <typename node_t, typename A, typename B>
inline auto sum_hood(node_t& node, trace_t call_point, A const& a, B const& b) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, auto const& y) {
        return x + y;
    }, a, b);
}


//! @brief Reduces a field to a single value by averaging.
template <typename node_t, typename A>
inline to_local<A> mean_hood(node_t& node, trace_t call_point, A const& a) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, to_local<A> const& y) {
        return x + y;
    }, a) / count_hood(node, call_point);
}

//! @brief Reduces a field to a single value by averaging with a given value for self.
template <typename node_t, typename A, typename B>
inline auto mean_hood(node_t& node, trace_t call_point, A const& a, B const& b) {
    return fold_hood(node, call_point, [] (to_local<A> const& x, auto const& y) {
        return x + y;
    }, a, b) / count_hood(node, call_point);
}


//! @brief Namespace of tags for use in aggregate functions.
namespace tags {
    //! @brief Struct for indicating a missing argument
    struct nothing {};
}

//! @brief Object indicating a missing argument.
constexpr tags::nothing nothing{};

//! @brief Reduces a field to a container of its constituent values skipping self in device order (void version).
template <typename node_t, typename C, typename A>
void list_hood(node_t& node, trace_t call_point, C& c, A const& a, tags::nothing) {
    fold_hood(node, call_point, [&] (to_local<A> const& x, common::unit) -> common::unit {
        common::uniform_insert(c, x);
        return {};
    }, a, common::unit{});
}
//! @brief Reduces a field to a container of its constituent values skipping self in device order (returning version).
template <typename node_t, typename C, typename A>
inline C list_hood(node_t& node, trace_t call_point, C&& c, A const& a, tags::nothing) {
    list_hood(node, call_point, c, a, nothing);
    return std::move(c);
}

//! @brief Reduces a field to a container of its constituent values with a given value for self in device order (void version).
template <typename node_t, typename C, typename A, typename B, typename = std::enable_if_t<not std::is_same<B, tags::nothing>::value>>
void list_hood(node_t& node, trace_t call_point, C& c, A const& a, B const& b) {
    bool done = false;
    fold_hood(node, call_point, [&] (device_t curr, to_local<A> const& x, common::unit) -> common::unit {
        if (curr > node.uid) {
            common::uniform_insert(c, self(node, call_point, b));
            done = true;
        }
        common::uniform_insert(c, x);
        return {};
    }, a, common::unit{});
    if (not done) common::uniform_insert(c, self(node, call_point, b));
}
//! @brief Reduces a field to a container of its constituent values with a given value for self in device order (returning version).
template <typename node_t, typename C, typename A, typename B, typename = std::enable_if_t<not std::is_same<B, tags::nothing>::value>>
inline C list_hood(node_t& node, trace_t call_point, C&& c, A const& a, B const& b) {
    list_hood(node, call_point, c, a, b);
    return std::move(c);
}

//! @brief Reduces a field to a container of its constituent values in device order (void version).
template <typename node_t, typename C, typename A>
inline void list_hood(node_t& node, trace_t call_point, C& c, A const& a) {
    list_hood(node, call_point, c, a, a);
}
//! @brief Reduces a field to a container of its constituent values in device order (returning version).
template <typename node_t, typename C, typename A>
inline C list_hood(node_t& node, trace_t call_point, C&& c, A const& a) {
    list_hood(node, call_point, c, a);
    return std::move(c);
}


}


}

#endif // FCPP_COORDINATION_UTILS_H_
