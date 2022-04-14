// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file quaternion.hpp
 * @brief Implementation of the `quaternion` class implementing quaternions.
 */

#ifndef FCPP_COMMON_QUATERNION_H_
#define FCPP_COMMON_QUATERNION_H_

#include <cmath>

#include <array>
#include <ostream>

#include "lib/settings.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


/**
 * @brief Class implementing quaternions.
 */
class quaternion {
  public:
    //! @brief Null quaternion constructor.
    quaternion() = default;

    //! @brief Real quaternion constructor.
    quaternion(real_t r) : m_data{r,0,0,0} {}

    //! @brief General quaternion constructor.
    quaternion(real_t a, real_t b, real_t c, real_t d) : m_data{a,b,c,d} {}

    //! @brief Vector quaternion constructor.
    quaternion(real_t const* v) {
        m_data[0] = 0;
        for (size_t i=0; i<3; ++i) m_data[i+1] = v[i];
    }

    //! @brief Vector quaternion constructor.
    quaternion(std::array<real_t,3> const& v) : quaternion(v.data()) {}

    //! @brief Rotation quaternion constructor.
    quaternion(real_t angle, real_t const* axis) {
        m_data[0] = cos(angle/2);
        real_t s = 0;
        for (size_t i=0; i<3; ++i) s += axis[i]*axis[i];
        s = sin(angle/2) / sqrt(s);
        for (size_t i=0; i<3; ++i) m_data[i+1] = s*axis[i];
    }

    //! @brief Rotation quaternion constructor.
    quaternion(real_t angle, std::array<real_t,3> const& axis) : quaternion(angle, axis.data()) {}

    //! @brief Pointer to beginning.
    inline real_t* begin() {
        return m_data;
    }

    //! @brief Const pointer to beginning.
    inline real_t const* begin() const {
        return m_data;
    }

    //! @brief Pointer to end.
    inline real_t* end() {
        return m_data + 4;
    }

    //! @brief Const pointer to end.
    inline real_t const* end() const {
        return m_data + 4;
    }

    //! @brief Access to components of the vector.
    inline real_t& operator[](size_t i) {
        return m_data[i];
    }

    //! @brief Const access to components of the vector.
    inline real_t operator[](size_t i) const {
        return m_data[i];
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    inline S& serialize(S& s) {
        return s & m_data;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    inline S& serialize(S& s) const {
        return s << m_data;
    }

  private:
    //! @brief The component data as C array.
    real_t m_data[4];
};


//! @brief Squared norm of a quaternion.
real_t abs(quaternion const&);

//! @brief Norm of a quaternion.
real_t norm(quaternion const&);

    //! @brief Output operator.
std::ostream& operator<<(std::ostream&, quaternion const&);

    //! @brief Equality operator.
bool operator==(quaternion const&, quaternion const&);

//! @brief Inequality operator.
bool operator!=(quaternion const&, quaternion const&);


//! @brief Norm less-than operator.
bool operator<(quaternion const&, real_t);

//! @brief Norm less-than-or-equal operator.
bool operator<=(quaternion const&, real_t);

//! @brief Norm greater-than operator.
bool operator>(quaternion const&, real_t);

//! @brief Norm greater-than-or-equal operator.
bool operator>=(quaternion const&, real_t);


//! @brief Unary plus operator.
quaternion const& operator+(quaternion const&);

//! @brief Unary minus operator.
quaternion operator-(quaternion);

//! @brief Conjugation operator.
quaternion operator~(quaternion);

//! @brief Inversion operator.
quaternion operator!(quaternion);


//! @brief Infix addition operator.
quaternion& operator+=(quaternion&, quaternion const&);

//! @brief Addition operator (copying the first argument).
quaternion operator+(quaternion, quaternion const&);

//! @brief Addition operator (copying the second argument).
quaternion operator+(quaternion const&, quaternion&&);


//! @brief Infix subtraction operator.
quaternion& operator-=(quaternion&, quaternion const&);

//! @brief Subtraction operator (copying the first argument).
quaternion operator-(quaternion, quaternion const&);

//! @brief Subtraction operator (copying the second argument).
quaternion operator-(quaternion const&, quaternion&&);


//! @brief Multiplication operator.
quaternion operator*(quaternion const&, quaternion const&);

//! @brief Infix multiplication operator.
quaternion& operator*=(quaternion&, quaternion const&);

//! @brief Division operator.
quaternion operator/(quaternion const&, quaternion);

//! @brief Infix division operator.
quaternion& operator/=(quaternion&, quaternion);


}


}

#endif // FCPP_COMMON_QUATERNION_H_
