// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file vec.hpp
 * @brief Implementation of the `vec` class representing n-dimensional physical vectors.
 */

#ifndef FCPP_DATA_VEC_H_
#define FCPP_DATA_VEC_H_

#include <cmath>

#include <array>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Class representing n-dimensional physical vectors.
 *
 * @param n Dimensionality of the vectors.
 */
template <size_t n>
struct vec {
    //! @brief The dimensionality of the vectors.
    constexpr static size_t dimension = n;
    
    //! @brief Pointer to beginning.
    double* begin() {
        return data;
    }
    
    //! @brief Const pointer to beginning.
    double const* begin() const {
        return data;
    }

    //! @brief Pointer to end.
    double* end() {
        return data + n;
    }

    //! @brief Const pointer to end.
    double const* end() const {
        return data + n;
    }

    //! @brief Access to components of the vector.
    double& operator[](size_t i) {
        return data[i];
    }
    
    //! @brief Const access to components of the vector.
    double operator[](size_t i) const {
        return data[i];
    }
    
    //! @brief Equality operator.
    bool operator==(vec const& o) const {
        for (size_t i=0; i<n; ++i) if (data[i] != o.data[i]) return false;
        return true;
    }
    
    //! @brief Inequality operator.
    bool operator!=(vec const& o) const {
        for (size_t i=0; i<n; ++i) if (data[i] != o.data[i]) return true;
        return false;
    }
    
    //! @brief The internal data as C array.
    double data[n];
};


//! @brief Vectorial addition.
//! @{
template <size_t n>
vec<n>& operator+=(vec<n>& x, const vec<n>& y) {
    for (size_t i=0; i<n; ++i) x[i] += y[i];
    return x;
}

template <size_t n>
vec<n>& operator+=(vec<n>& x, double y) {
    for (size_t i=0; i<n; ++i) x[i] += y;
    return x;
}

template <size_t n>
vec<n> operator+(vec<n> x, const vec<n>& y) {
    return x += y;
}

template <size_t n>
vec<n> operator+(const vec<n>& x, vec<n>&& y) {
    return y += x;
}

template <size_t n>
vec<n> operator+(vec<n> x, double y) {
    return x += y;
}

template <size_t n>
vec<n> operator+(double x, vec<n> y) {
    return y += x;
}
//! @}


//! @brief Vectorial subtraction.
//! @{
template <size_t n>
vec<n>& operator-=(vec<n>& x, const vec<n>& y) {
    for (size_t i=0; i<n; ++i) x[i] -= y[i];
    return x;
}

template <size_t n>
vec<n>& operator-=(vec<n>& x, double y) {
    for (size_t i=0; i<n; ++i) x[i] -= y;
    return x;
}

template <size_t n>
vec<n> operator-(vec<n> x, const vec<n>& y) {
    return x -= y;
}

template <size_t n>
vec<n> operator-(const vec<n>& x, vec<n>&& y) {
    for (size_t i=0; i<n; ++i) y[i] = x[i]-y[i];
    return y;
}

template <size_t n>
vec<n> operator-(vec<n> x, double y) {
    return x -= y;
}

template <size_t n>
vec<n> operator-(double x, vec<n> y) {
    for (size_t i=0; i<n; ++i) y[i] = x-y[i];
    return y;
}
//! @}


//! @brief Unary sign application.
//! @{
template <size_t n>
inline vec<n> const& operator+(vec<n> const& x) {
    return x;
}
template <size_t n>
vec<n> operator-(vec<n> x) {
    for (size_t i=0; i<n; ++i) x[i] = -x[i];
    return x;
}
//! @}


//! @brief Multiplication and division by a scalar.
//! @{
template <size_t n>
vec<n>& operator*=(vec<n>& x, double y) {
    for (size_t i=0; i<n; ++i) x[i] *= y;
    return x;
}

template <size_t n>
vec<n> operator*(vec<n> x, double y) {
    return x *= y;
}

template <size_t n>
vec<n> operator*(double x, vec<n> y) {
    return y *= x;
}

template <size_t n>
vec<n>& operator/=(vec<n>& x, double y) {
    for (size_t i=0; i<n; ++i) x[i] /= y;
    return x;
}

template <size_t n>
vec<n> operator/(vec<n> x, double y) {
    return x /= y;
}
//! @}


//! @brief Scalar multiplication, vector norm and normalisation.
//! @{
template <size_t n>
double operator*(const vec<n>& x, const vec<n>& y) {
    double res = 0;
    for (size_t i=0; i<n; ++i) res += x[i] * y[i];
    return res;
}

template <size_t n>
double norm(const vec<n>& x) {
    return sqrt(x * x);
}

template <size_t n>
double unit(const vec<n>& x) {
    return x / norm(x);
}

template <size_t n>
double distance(const vec<n>& x, const vec<n>& y) {
    return norm(x - y);
}
//! @}


//! @brief Creates a vector from its arguments.
template <typename... Ts>
vec<sizeof...(Ts)> make_vec(Ts... xs) {
    return {double(xs)...};
}


}


#endif // FCPP_DATA_VEC_H_
