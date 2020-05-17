// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file array.hpp
 * @brief Implementation of pointwise operations for the `std::array` class interpreted as physical vector.
 */

#ifndef FCPP_COMMON_ARRAY_H_
#define FCPP_COMMON_ARRAY_H_

#include <cmath>

#include <array>


/**
 * @brief Namespace of the C++ standard library.
 */
namespace std {


//! @brief Vectorial addition.
//! @{
template <typename T, size_t n>
array<T,n>& operator+=(array<T,n>& x, const array<T,n>& y) {
    for (size_t i=0; i<n; ++i) x[i] += y[i];
    return x;
}

template <typename T, size_t n>
array<T,n>& operator+=(array<T,n>& x, const T& y) {
    for (size_t i=0; i<n; ++i) x[i] += y;
    return x;
}

template <typename T, size_t n>
array<T,n> operator+(array<T,n> x, const array<T,n>& y) {
    return x += y;
}

template <typename T, size_t n>
array<T,n> operator+(const array<T,n>& x, array<T,n>&& y) {
    return y += x;
}

template <typename T, size_t n>
array<T,n> operator+(array<T,n> x, const T& y) {
    return x += y;
}

template <typename T, size_t n>
array<T,n> operator+(const T& x, array<T,n> y) {
    return y += x;
}
//! @}


//! @brief Vectorial subtraction.
//! @{
template <typename T, size_t n>
array<T,n>& operator-=(array<T,n>& x, const array<T,n>& y) {
    for (size_t i=0; i<n; ++i) x[i] -= y[i];
    return x;
}

template <typename T, size_t n>
array<T,n>& operator-=(array<T,n>& x, const T& y) {
    for (size_t i=0; i<n; ++i) x[i] -= y;
    return x;
}

template <typename T, size_t n>
array<T,n> operator-(array<T,n> x, const array<T,n>& y) {
    return x -= y;
}

template <typename T, size_t n>
array<T,n> operator-(const array<T,n>& x, array<T,n>&& y) {
    for (size_t i=0; i<n; ++i) y[i] = x[i]-y[i];
    return y;
}

template <typename T, size_t n>
array<T,n> operator-(array<T,n> x, const T& y) {
    return x -= y;
}

template <typename T, size_t n>
array<T,n> operator-(const T& x, array<T,n> y) {
    for (size_t i=0; i<n; ++i) y[i] = x-y[i];
    return y;
}
//! @}


//! @brief Multiplication by a scalar.
//! @{
template <typename T, size_t n>
array<T,n>& operator*=(array<T,n>& x, const T& y) {
    for (size_t i=0; i<n; ++i) x[i] *= y;
    return x;
}

template <typename T, size_t n>
array<T,n> operator*(array<T,n> x, const T& y) {
    return x *= y;
}

template <typename T, size_t n>
array<T,n> operator*(const T& x, array<T,n> y) {
    return y *= x;
}
//! @}


//! @brief Scalar multiplication, vector norm and normalisation.
//! @{
template <typename T, size_t n>
T operator*(const array<T,n>& x, const array<T,n>& y) {
    T res(0);
    for (size_t i=0; i<n; ++i) res += x[i] * y[i];
    return res;
}

template <typename T, size_t n>
T norm(const array<T,n>& x) {
    return sqrt(x * x);
}

template <typename T, size_t n>
T unit(const array<T,n>& x) {
    return x / norm(x);
}

template <typename T, size_t n>
T distance(const array<T,n>& x, const array<T,n>& y) {
    return norm(x - y);
}
//! @}


//! @brief Creates an array from its arguments.
template <typename T, typename... Ts>
array<T, sizeof...(Ts)+1> make_array(T x, Ts... xs) {
    return array<T, sizeof...(Ts)+1>{x, ((T)xs)...};
}


}


#endif // FCPP_COMMON_ARRAY_H_
