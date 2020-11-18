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
#include "lib/internal/trace.hpp"


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
const A& mux(bool b, const A& x, const A& y) {
    return b ? x : y;
}
//! @brief local guard, moving arguments of same type
template <typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
A mux(bool b, A&& x, A&& y) {
    return b ? std::move(x) : std::move(y);
}
//! @brief local guard, arguments of different but compatible types
template <typename A, typename B, typename = std::enable_if_t<std::is_same<to_local<A>, to_local<B>>::value and not std::is_same<A,B>::value>>
to_field<A> mux(bool b, const A& x, const B& y) {
    return b ? to_field<A>{x} : to_field<A>{y};
}
//! @brief field guard
template <typename A, typename B, typename = std::enable_if_t<std::is_same<to_local<A>, to_local<B>>::value>>
to_field<A> mux(field<bool> b, const A& x, const B& y) {
    return map_hood([] (bool b, to_local<A> x, to_local<B> y) -> to_local<A> {
        return b ? x : y;
    }, b, x, y);
}
//! @}


//! @brief Maximum between two local values.
template <typename A, typename = if_local<A>>
inline const A& max(const A& x, const A& y) {
    return std::max(x, y);
}

//! @brief Maximum between two field values.
template <typename A, typename B, typename = std::enable_if_t<common::has_template<field, tuple<A,B>> and std::is_same<to_local<A>, to_local<B>>::value>>
inline to_field<A> max(const A& x, const B& y) {
    return map_hood([] (to_local<A> x, to_local<B> y) -> to_local<A> {
        return std::max(x, y);
    }, x, y);
}

//! @brief Minimum between two local values.
template <typename A, typename = if_local<A>>
inline const A& min(const A& x, const A& y) {
    return std::min(x, y);
}

//! @brief Minimum between two field values.
template <typename A, typename B, typename = std::enable_if_t<common::has_template<field, tuple<A,B>> and std::is_same<to_local<A>, to_local<B>>::value>>
inline to_field<A> min(const A& x, const B& y) {
    return map_hood([] (to_local<A> x, to_local<B> y) -> to_local<A> {
        return std::min(x, y);
    }, x, y);
}


//! @brief Extracts a component from a field of tuple-like structures.
template <size_t n, typename A>
auto get(const field<A>& f) {
    return map_hood([] (const A& x) {
        return get<n>(x);
    }, f);
}


//! @brief Natural logarithm.
using std::log;

//! @brief Pointwise natural logarithm.
inline field<double> log(const field<double>& f) {
    return map_hood([](double x){
        return std::log(x);
    }, f);
}

//! @brief Natural exponentiation.
using std::exp;

//! @brief Pointwise natural exponentiation.
inline field<double> exp(const field<double>& f) {
    return map_hood([](double x){
        return std::exp(x);
    }, f);
}

//! @brief Square root.
using std::sqrt;

//! @brief Pointwise square root.
inline field<double> sqrt(const field<double>& f) {
    return map_hood([](double x){
        return std::sqrt(x);
    }, f);
}

//! @brief Power.
using std::pow;

//! @brief Pointwise power.
//! @{
inline field<double> pow(const field<double>& base, const field<double>& exponent) {
    return map_hood([](double x, double y){
        return std::pow(x, y);
    }, base, exponent);
}
inline field<double> pow(double base, const field<double>& exponent) {
    return map_hood([](double x, double y){
        return std::pow(x, y);
    }, base, exponent);
}
inline field<double> pow(const field<double>& base, double exponent) {
    return map_hood([](double x, double y){
        return std::pow(x, y);
    }, base, exponent);
}
//! @}

//! @brief Check for infinite values.
using std::isinf;

//! @brief Pointwise check for infinite values.
inline field<bool> isinf(const field<double>& f) {
    return map_hood([](double x){
        return std::isinf(x);
    }, f);
}

//! @brief Check for not-a-number values.
using std::isnan;

//! @brief Pointwise check for not-a-number values.
inline field<bool> isnan(const field<double>& f) {
    return map_hood([](double x){
        return std::isnan(x);
    }, f);
}

//! @brief Check for finite values.
using std::isfinite;

//! @brief Pointwise check for finite values.
inline field<bool> isfinite(const field<double>& f) {
    return map_hood([](double x){
        return std::isfinite(x);
    }, f);
}

//! @brief Check for normal values (finite, non-zero and not sub-normal).
using std::isnormal;

//! @brief Pointwise check for normal values (finite, non-zero and not sub-normal).
inline field<bool> isnormal(const field<double>& f) {
    return map_hood([](double x){
        return std::isnormal(x);
    }, f);
}


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


}


}

#endif // FCPP_COORDINATION_UTILS_H_
