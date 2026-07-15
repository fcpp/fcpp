// Copyright © 2025 Giorgio Audrito. All Rights Reserved.

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

    //! @brief Converting constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    option(option<A, false> const&) {}

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
    explicit operator T() const {
        return {};
    }

    //! @brief Serialises the content to/from a given output stream.
    template <typename S>
    S& serialize(S& s) const {
        return s;
    }
};


//! @brief Class for handling an optional data (full overload).
template <typename T>
class option<T, true> {
    //! @cond INTERNAL
    //! @brief Class friendships
    //! @{
    template <typename A, int enable>
    friend class option;
    //! @endcond

  public:
    //! @brief The contained type.
    using value_type = T;

    //! @brief Empty constructor.
    option() = default;

    //! @brief Forwarding constructor (single parameter).
    template <typename T1, typename = std::enable_if_t<std::is_constructible<T, T1&&>::value and not std::is_same<std::decay_t<T1>, option>::value>>
    option(T1&& x1) : m_data(std::forward<T1>(x1)) {}

    //! @brief Forwarding constructor (multiple parameters).
    template <typename T1, typename... Ts, typename = std::enable_if_t<std::is_constructible<T, T1&&, Ts&&...>::value and sizeof...(Ts)>>
    option(T1&& x1, Ts&&... xs) : m_data(std::forward<T1>(x1), std::forward<Ts>(xs)...) {}

    //! @brief Converting constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    option(option<A, true> const& x) : m_data(x.m_data) {}

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
    explicit operator T() const {
        return m_data;
    }

    //! @brief Serialises the content from a given input stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_data;
    }

    //! @brief Serialises the content to a given output stream.
    template <typename S>
    S& serialize(S& s) const {
        return s & m_data;
    }

  private:
    //! @brief The actual data stored.
    T m_data;
};


//! @brief Class for handling an optional data (run-time overload).
template <typename T>
class option<T, 2> {
    //! @cond INTERNAL
    //! @brief Class friendships
    //! @{
    template <typename A, int enable>
    friend class option;
    //! @endcond

  public:
    //! @brief The contained type.
    using value_type = T;

    //! @brief Empty constructor.
    option() : m_data(), m_some(false) {}

    //! @brief Forwarding constructor (single parameter).
    template <typename T1, typename = std::enable_if_t<std::is_constructible<T, T1&&>::value and not std::is_same<std::decay_t<T1>, option>::value>>
    option(T1&& x1) : m_data(std::forward<T1>(x1)), m_some(true) {}

    //! @brief Forwarding constructor (multiple parameters).
    template <typename T1, typename... Ts, typename = std::enable_if_t<std::is_constructible<T, T1&&, Ts&&...>::value and sizeof...(Ts)>>
    option(T1&& x1, Ts&&... xs) : m_data(std::forward<T1>(x1), std::forward<Ts>(xs)...), m_some(true) {}

    //! @brief Converting constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    option(option<A> const& x) : m_data(x.m_data), m_some(x.m_some) {}

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
        return m_some == o.m_some and (not m_some or m_data == o.m_data);
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
    explicit operator T() const {
        return m_data;
    }

    //! @brief Serialises the content from a given input stream.
    template <typename S>
    S& serialize(S& s) {
        s & m_some;
        if (m_some) s & m_data;
        return s;
    }

    //! @brief Serialises the content to a given output stream.
    template <typename S>
    S& serialize(S& s) const {
        s & m_some;
        if (m_some) s & m_data;
        return s;
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
