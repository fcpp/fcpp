// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "lib/common/quaternion.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


//! @brief Squared norm of a quaternion.
real_t abs(quaternion const& a) {
    real_t r = 0;
    for (size_t i=0; i<4; ++i) r += a[i]*a[i];
    return r;
}

//! @brief Norm of a quaternion.
real_t norm(quaternion const& a) {
    return sqrt(abs(a));
}

//! @brief Output operator.
std::ostream& operator<<(std::ostream& o, quaternion const& a) {
    o << a[0] << " + " << a[1] << "i + " << a[2] << "j + " << a[3] << "k";
    return o;
};

//! @brief Equality operator.
bool operator==(quaternion const& a, quaternion const& b) {
    for (size_t i=0; i<4; ++i) if (a[i] != b[i]) return false;
    return true;
}

//! @brief Inequality operator.
bool operator!=(quaternion const& a, quaternion const& b) {
    return not (a == b);
}

//! @brief Norm comparison operators.
//! @{
bool operator<(quaternion const& a, real_t b) {
    return abs(a) < b*b;
}

bool operator<=(quaternion const& a, real_t b) {
    return abs(a) <= b*b;
}

bool operator>(quaternion const& a, real_t b) {
    return abs(a) > b*b;
}

bool operator>=(quaternion const& a, real_t b) {
    return abs(a) >= b*b;
}
//! @}

//! @brief Unary plus operator.
quaternion const& operator+(quaternion const& a) {
    return a;
}

//! @brief Unary minus operator.
quaternion operator-(quaternion a) {
    for (size_t i=0; i<4; ++i) a[i] = -a[i];
    return a;
}

//! @brief Conjugation operator.
quaternion operator~(quaternion a) {
    for (size_t i=1; i<4; ++i) a[i] = -a[i];
    return a;
}

//! @brief Inversion operator.
quaternion operator!(quaternion a) {
    real_t k = -1/abs(a);
    a[0] *= -k;
    for (size_t i=1; i<4; ++i) a[i] *= k;
    return a;
}

//! @brief Addition operator.
//! @{
quaternion& operator+=(quaternion& a, quaternion const& b) {
    for (size_t i=0; i<4; ++i) a[i] += b[i];
    return a;
}

quaternion operator+(quaternion a, quaternion const& b) {
    return a += b;
}

quaternion operator+(quaternion const& a, quaternion&& b) {
    return b += a;
}
//! @}

//! @brief Subtraction operator.
//! @{
quaternion& operator-=(quaternion& a, quaternion const& b) {
    for (size_t i=0; i<4; ++i) a[i] -= b[i];
    return a;
}

quaternion operator-(quaternion a, quaternion const& b) {
    return a -= b;
}

quaternion operator-(quaternion const& a, quaternion&& b) {
    for (size_t i=0; i<4; ++i) b[i] = a[i] - b[i];
    return b;
}
//! @}

//! @brief Multiplication operator.
//! @{
quaternion operator*(quaternion const& a, quaternion const& b) {
    quaternion c;
    c[0] = a[0]*b[0];
    for (size_t i=1; i<4; ++i) {
        c[0] -= a[i]*b[i];
        c[i] = a[0]*b[i] + a[i]*b[0] + a[i%3+1]*b[(i+1)%3+1] - a[(i+1)%3+1]*b[i%3+1];
    }
    return c;
}

quaternion& operator*=(quaternion& a, quaternion const& b) {
    return a = a*b;
}
//! @}

//! @brief Division operator.
//! @{
quaternion operator/(quaternion const& a, quaternion b) {
    return a * !std::move(b);
}

quaternion& operator/=(quaternion& a, quaternion b) {
    return a = a/std::move(b);
}
//! @}


}


}
