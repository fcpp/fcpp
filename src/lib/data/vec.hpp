// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file vec.hpp
 * @brief Implementation of the `vec` class representing n-dimensional physical vectors.
 */

#ifndef FCPP_DATA_VEC_H_
#define FCPP_DATA_VEC_H_

#include <cmath>

#include <array>

#include "lib/settings.hpp"


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

    //! @brief Default constructor.
    vec() = default;

    //! @brief Copy constructor.
    vec(vec<n> const&) = default;

    //! @brief Move constructor.
    vec(vec<n>&&) = default;

    //! @brief Copy assignment (restricted to lvalues).
    vec<n>& operator=(vec<n> const&) & = default;

    //! @brief Move assignment (restricted to lvalues).
    vec<n>& operator=(vec<n>&&) & = default;

    //! @brief Pointer to beginning.
    real_t* begin() {
        return data;
    }

    //! @brief Const pointer to beginning.
    real_t const* begin() const {
        return data;
    }

    //! @brief Pointer to end.
    real_t* end() {
        return data + n;
    }

    //! @brief Const pointer to end.
    real_t const* end() const {
        return data + n;
    }

    //! @brief Access to components of the vector.
    real_t& operator[](size_t i) {
        return data[i];
    }

    //! @brief Const access to components of the vector.
    real_t operator[](size_t i) const {
        return data[i];
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & data;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << data;
    }

    //! @brief The internal data as C array.
    real_t data[n];
};


//! @brief Infix vectorial addition.
template <size_t n>
vec<n>& operator+=(vec<n>& x, vec<n> const& y) {
    for (size_t i=0; i<n; ++i) x[i] += y[i];
    return x;
}

//! @brief Infix pointwise vector-number addition.
template <size_t n>
vec<n>& operator+=(vec<n>& x, real_t y) {
    for (size_t i=0; i<n; ++i) x[i] += y;
    return x;
}

//! @brief Vectorial addition (copying the first argument).
template <size_t n>
vec<n> operator+(vec<n> x, vec<n> const& y) {
    return x += y;
}

//! @brief Vectorial addition (copying the second argument).
template <size_t n>
vec<n> operator+(vec<n> const& x, vec<n>&& y) {
    return y += x;
}

//! @brief Pointwise vector-number addition.
template <size_t n>
vec<n> operator+(vec<n> x, real_t y) {
    return x += y;
}

//! @brief Pointwise number-vector addition.
template <size_t n>
vec<n> operator+(real_t x, vec<n> y) {
    return y += x;
}


//! @brief Infix vectorial subtraction.
template <size_t n>
vec<n>& operator-=(vec<n>& x, vec<n> const& y) {
    for (size_t i=0; i<n; ++i) x[i] -= y[i];
    return x;
}

//! @brief Infix pointwise vector-number subtraction.
template <size_t n>
vec<n>& operator-=(vec<n>& x, real_t y) {
    for (size_t i=0; i<n; ++i) x[i] -= y;
    return x;
}

//! @brief Vectorial subtraction (copying the first argument).
template <size_t n>
vec<n> operator-(vec<n> x, vec<n> const& y) {
    return x -= y;
}

//! @brief Vectorial subtraction (copying the second argument).
template <size_t n>
vec<n> operator-(vec<n> const& x, vec<n>&& y) {
    for (size_t i=0; i<n; ++i) y[i] = x[i]-y[i];
    return y;
}

//! @brief Pointwise vector-number subtraction.
template <size_t n>
vec<n> operator-(vec<n> x, real_t y) {
    return x -= y;
}

//! @brief Pointwise number-vector subtraction.
template <size_t n>
vec<n> operator-(real_t x, vec<n> y) {
    for (size_t i=0; i<n; ++i) y[i] = x-y[i];
    return y;
}


//! @brief Unary positive sign application.
template <size_t n>
inline vec<n> const& operator+(vec<n> const& x) {
    return x;
}

//! @brief Unary negative sign application.
template <size_t n>
vec<n> operator-(vec<n> x) {
    for (size_t i=0; i<n; ++i) x[i] = -x[i];
    return x;
}


//! @brief Infix multiplication by a scalar.
template <size_t n>
vec<n>& operator*=(vec<n>& x, real_t y) {
    for (size_t i=0; i<n; ++i) x[i] *= y;
    return x;
}

//! @brief Right-multiplication by a scalar.
template <size_t n>
vec<n> operator*(vec<n> x, real_t y) {
    return x *= y;
}

//! @brief Left-multiplication by a scalar.
template <size_t n>
vec<n> operator*(real_t x, vec<n> y) {
    return y *= x;
}

//! @brief Infix division by a scalar.
template <size_t n>
vec<n>& operator/=(vec<n>& x, real_t y) {
    for (size_t i=0; i<n; ++i) x[i] /= y;
    return x;
}

//! @brief Division by a scalar.
template <size_t n>
vec<n> operator/(vec<n> x, real_t y) {
    return x /= y;
}


//! @brief Scalar multiplication.
template <size_t n>
real_t operator*(vec<n> const& x, vec<n> const& y) {
    real_t res = 0;
    for (size_t i=0; i<n; ++i) res += x[i] * y[i];
    return res;
}

//! @brief Numeric absolute value.
using std::abs;

//! @brief Vector square norm.
template <size_t n>
real_t abs(vec<n> const& x) {
    return x * x;
}

//! @brief Vector norm.
template <size_t n>
real_t norm(vec<n> const& x) {
    return sqrt(abs(x));
}

//! @brief Vector normalisation (moving).
template <size_t n>
vec<n> unit(vec<n>&& x) {
    return x /= norm(x);
}

//! @brief Vector normalisation (copying).
template <size_t n>
vec<n> unit(vec<n> const& x) {
    return x / norm(x);
}

//! @brief Vector distance.
template <size_t n>
real_t distance(vec<n> const& x, vec<n> const& y) {
    return norm(x - y);
}


//! @brief Equality operator.
template <size_t n>
bool operator==(vec<n> const& x, vec<n> const& y) {
    for (size_t i=0; i<n; ++i) if (x.data[i] != y.data[i]) return false;
    return true;
}

//! @brief Inequality operator.
template <size_t n>
bool operator!=(vec<n> const& x, vec<n> const& y) {
    return not (x == y);
}


//! @brief Norm less-than operator.
template <size_t n>
bool operator<(vec<n> const& x, real_t b) {
    return abs(x) < b*b;
}

//! @brief Norm less-than-or-equal operator.
template <size_t n>
bool operator<=(vec<n> const& x, real_t b) {
    return abs(x) <= b*b;
}

//! @brief Norm greater-than operator.
template <size_t n>
bool operator>(vec<n> const& x, real_t b) {
    return abs(x) > b*b;
}

//! @brief Norm greater-than-or-equal operator.
template <size_t n>
bool operator>=(vec<n> const& x, real_t b) {
    return abs(x) >= b*b;
}


//! @brief Creates a vector from its arguments.
template <typename... Ts>
constexpr vec<sizeof...(Ts)> make_vec(Ts... xs) {
    return {real_t(xs)...};
}


}


#endif // FCPP_DATA_VEC_H_
