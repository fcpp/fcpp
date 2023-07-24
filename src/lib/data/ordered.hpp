// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file ordered.hpp
 * @brief Implementation of the `ordered` class adding trivial ordering to a base type.
 */

#ifndef FCPP_DATA_ORDERED_H_
#define FCPP_DATA_ORDERED_H_

#include <utility>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Class adding trivial ordering to a base type.
 *
 * Useful in tuples to enable lexycographic ordering.
 * The order defined is so that every object is assumed equal.
 *
 * @param T The base type.
 */
template <typename T>
struct ordered {
    //! @brief The base type.
    using type = T;

    //! @brief Default constructor.
    ordered() = default;

    //! @brief Copy constructor.
    ordered(ordered const&) = default;

    //! @brief Move constructor.
    ordered(ordered&&) = default;

    //! @brief Constructor from the base type.
    ordered(T v) : data(std::move(v)) {}

    //! @brief Copy assignment.
    ordered& operator=(ordered const&) = default;

    //! @brief Move assignment.
    ordered& operator=(ordered&&) = default;

    //! @brief Copy assignment from the base type.
    ordered& operator=(T const& x) {
        data = x;
        return *this;
    }

    //! @brief Move assignment from the base type.
    ordered& operator=(T&& x) {
        data = std::move(x);
        return *this;
    }

    //! @brief Conversion to the base type.
    operator T() const {
        return data;
    }

    //! @brief Less-than operator.
    bool operator<(ordered<T> const& o) const {
        return false;
    }

    //! @brief Greater-than operator.
    bool operator>(ordered<T> const& o) const {
        return false;
    }

    //! @brief Less-or-equal operator.
    bool operator<=(ordered<T> const& o) const {
        return true;
    }

    //! @brief Greater-or-equal operator.
    bool operator>=(ordered<T> const& o) const {
        return true;
    }

    //! @brief Equality operator.
    bool operator==(ordered<T> const& o) const {
        return true;
    }

    //! @brief Inequality operator.
    bool operator!=(ordered<T> const& o) const {
        return false;
    }

    //! @brief The data to which ordering is added.
    T data;
};

//! @brief Converts a value to a trivially ordered value.
template <typename T>
ordered<T> make_ordered(T v) {
    return std::move(v);
}


}


#endif // FCPP_DATA_ORDERED_H_
