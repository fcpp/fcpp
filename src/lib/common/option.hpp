// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file option.hpp
 * @brief Implementation of the `option<T, bool>` class template for handling an optional data.
 */

#ifndef FCPP_COMMON_OPTION_H_
#define FCPP_COMMON_OPTION_H_

#include <cstddef>
#include <type_traits>
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
 * Upon accessing a non-existent value, it silently returns the default value of type T.
 *
 * @param T Type.
 * @param enable Boolean stating whether it should be actually stored (no argument for runtime-optional storing).
 */
template <typename T, int enable = 2>
class option;


//! @brief Class for handling an optional data (empty overload).
template <typename T>
class option<T, false> {
  public:
    //! @brief The contained type.
    using value_type = T;

    //! @brief Forwarding constructor.
    template <typename... Ts, typename = std::enable_if_t<std::is_constructible<T, Ts&&...>::value>>
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

    //! @brief Container size.
    static constexpr size_t size() {
        return 0;
    }

    //! @brief Checking for emptyness.
    static constexpr bool empty() {
        return true;
    }

    //! @brief Container beginning.
    static constexpr T* begin() {
        return nullptr;
    }

    //! @brief Container end.
    static constexpr T* end() {
        return nullptr;
    }

    //! @brief Container beginning.
    static constexpr T const* cbegin() {
        return nullptr;
    }

    //! @brief Container end.
    static constexpr T const* cend() {
        return nullptr;
    }

    //! @brief Data access.
    T front() const {
        return {};
    }

    //! @brief Data access.
    T back() const {
        return {};
    }

    //! @brief Value extraction.
    operator T() const {
        return {};
    }
};


//! @brief Class for handling an optional data (full overload).
template <typename T>
class option<T, true> {
  public:
    //! @brief The contained type.
    using value_type = T;

    //! @brief Forwarding constructor.
    template <typename... Ts, typename = std::enable_if_t<std::is_constructible<T, Ts&&...>::value>>
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

    //! @brief Container size.
    static constexpr size_t size() {
        return 1;
    }

    //! @brief Checking for emptyness.
    static constexpr bool empty() {
        return false;
    }

    //! @brief Container beginning.
    T* begin() {
        return &m_data;
    }

    //! @brief Container end.
    T* end() {
        return &m_data + 1;
    }

    //! @brief Container beginning (const).
    T const* begin() const {
        return &m_data;
    }

    //! @brief Container end (const).
    T const* end() const {
        return &m_data + 1;
    }

    //! @brief Container beginning.
    T const* cbegin() const {
        return &m_data;
    }

    //! @brief Container end.
    T const* cend() const {
        return &m_data + 1;
    }

    //! @brief Data access.
    T& front() {
        return m_data;
    }

    //! @brief Const data access.
    T const& front() const {
        return m_data;
    }

    //! @brief Data access.
    T& back() {
        return m_data;
    }

    //! @brief Const data access.
    T const& back() const {
        return m_data;
    }

    //! @brief Value extraction.
    operator T() const {
        return m_data;
    }

  private:
    //! @brief The actual data stored.
    T m_data;
};


//! @brief Class for handling an optional data (run-time overload).
template <typename T>
class option<T, 2> {
  public:
    //! @brief The contained type.
    using value_type = T;

    //! @brief Empty constructor.
    option() : m_data(), m_some(false) {}

    //! @brief Forwarding constructor.
    template <typename... Ts, typename = std::enable_if_t<std::is_constructible<T, Ts&&...>::value>>
    option(Ts&&... xs) : m_data(std::forward<Ts>(xs)...), m_some(true) {}

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
        return m_some == o.m_some and m_data == o.m_data;
    }

    //! @brief Container size.
    size_t size() const {
        return m_some;
    }

    //! @brief Checking for emptyness.
    bool empty() const {
        return not m_some;
    }

    //! @brief Removing the contained element.
    void clear() {
        m_some = false;
        m_data = {};
    }

    //! @brief Adding an element to be contained.
    template <typename... Ts>
    void emplace(Ts&&... xs) {
        m_some = true;
        m_data = T(std::forward<Ts>(xs)...);
    }

    //! @brief Container beginning.
    T* begin() {
        return &m_data;
    }

    //! @brief Container end.
    T* end() {
        return &m_data + m_some;
    }

    //! @brief Container beginning (const).
    T const* begin() const {
        return &m_data;
    }

    //! @brief Container end (const).
    T const* end() const {
        return &m_data + m_some;
    }

    //! @brief Container beginning.
    T const* cbegin() const {
        return &m_data;
    }

    //! @brief Container end.
    T const* cend() const {
        return &m_data + m_some;
    }

    //! @brief Data access.
    T& front() {
        return m_data;
    }

    //! @brief Const data access.
    T const& front() const {
        return m_data;
    }

    //! @brief Data access.
    T& back() {
        return m_data;
    }

    //! @brief Const data access.
    T const& back() const {
        return m_data;
    }

    //! @brief Value extraction.
    operator T() const {
        return m_data;
    }

  private:
    //! @brief The actual data stored.
    T m_data;
    //! @brief Whether there is a value.
    bool m_some;
};


}


}

#endif // FCPP_COMMON_OPTION_H_
