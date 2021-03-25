// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file option.hpp
 * @brief Implementation of the `option<T, bool>` class template for handling an optional data.
 */

#ifndef FCPP_COMMON_OPTION_H_
#define FCPP_COMMON_OPTION_H_

#include <utility>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


/**
 * @brief Class for handling an optional data.
 *
 * @param T Type.
 * @param enable Whether it should be actually stored.
 */
template <typename T, bool enable>
class option;


//! @brief Class for handling an optional data (empty overload).
template <typename T>
class option<T, false> {
  public:
    //! @brief Forwarding constructor.
    template <typename... Ts>
    option(Ts&&...) {}

    //! @brief Copy constructor.
    option(option const&) = default;

    //! @brief Move constructor.
    option(option&&) = default;

    //! @brief Copy assignment.
    option& operator=(option const&) = default;

    //! @brief Move assignment.
    option& operator=(option&&) = default;

    //! @brief Equality operator.
    bool operator==(option const&) const {
        return true;
    }

    //! @brief Data access.
    T front() const {
        return {};
    }
};


//! @brief Class for handling an optional data (full overload).
template <typename T>
class option<T, true> {
  public:
    //! @brief Forwarding constructor.
    template <typename... Ts>
    option(Ts&&... xs) : m_data(std::forward<Ts>(xs)...) {}

    //! @brief Copy constructor.
    option(option const&) = default;

    //! @brief Move constructor.
    option(option&&) = default;

    //! @brief Copy assignment.
    option& operator=(option const&) = default;

    //! @brief Move assignment.
    option& operator=(option&&) = default;

    //! @brief Equality operator.
    bool operator==(option const& o) const {
        return m_data == o.m_data;
    }

    //! @brief Data access.
    T& front() {
        return m_data;
    }

    //! @brief Const data access.
    T const& front() const {
        return m_data;
    }

  private:
    //! @brief The actual data stored.
    T m_data;
};


}


}

#endif // FCPP_COMMON_OPTION_H_
