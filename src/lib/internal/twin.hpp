// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file twin.hpp
 * @brief Implementation of the `twin` class for handling two variables, possibly identical.
 */

#ifndef FCPP_INTERNAL_TWIN_H_
#define FCPP_INTERNAL_TWIN_H_

#include <type_traits>
#include <utility>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing objects of internal use.
namespace internal {


//! @cond INTERNAL
template <typename T, bool is_twin>
class twin;
//! @endcond


//! @brief Class offering access to a single value through `first()` and `second()`.
template <typename T>
class twin<T, true> {
  public:
    //! @brief The type of the content.
    typedef T value_type;

    //! @name constructors
    //! @{

    //! @brief Default constructor.
    twin() = default;

    //! @brief Copy constructor.
    twin(twin const&) = default;

    //! @brief Move constructor.
    twin(twin&&) = default;

    //! @brief Initialising constructor.
    template <typename... Ts, typename = std::enable_if_t<not std::is_same<std::tuple<std::decay_t<Ts>...>, std::tuple<twin>>::value>>
    twin(Ts&&... xs) : m_data(std::forward<Ts>(xs)...) {}
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Copy assignment.
    twin& operator=(twin const&) = default;

    //! @brief Move assignment.
    twin& operator=(twin&&) = default;
    //! @}

    //! @brief Exchanges contents of twin objects.
    void swap(twin& m) {
        using std::swap;
        swap(m_data, m.m_data);
    }

    //! @brief Equality operator.
    bool operator==(twin const& o) const {
        return m_data == o.m_data;
    }

    //! @brief Access to the first element.
    T& first() {
        return m_data;
    }

    //! @brief Const access to the first element.
    T const& first() const {
        return m_data;
    }

    //! @brief Access to the second element.
    T& second() {
        return m_data;
    }

    //! @brief Const access to the second element.
    T const& second() const {
        return m_data;
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_data;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_data;
    }

  private:
    //! @brief The content of the class.
    T m_data;
};


//! @brief Class offering access to different values through `first()` and `second()`.
template <typename T>
class twin<T, false> {
  public:
    //! @brief The type of the content.
    typedef T value_type;

    //! @name constructors
    //! @{

    //! @brief Default constructor.
    twin() = default;

    //! @brief Copy constructor.
    twin(twin const&) = default;

    //! @brief Move constructor.
    twin(twin&&) = default;

    //! @brief Initialising constructor.
    template <typename... Ts, typename = std::enable_if_t<not std::is_same<std::tuple<std::decay_t<Ts>...>, std::tuple<twin>>::value>>
    twin(Ts&&... xs) : m_first(std::forward<Ts>(xs)...), m_second(std::forward<Ts>(xs)...) {}
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Copy assignment.
    twin& operator=(twin const&) = default;

    //! @brief Move assignment.
    twin& operator=(twin&&) = default;
    //! @}

    //! @brief Exchanges contents of twin objects.
    void swap(twin& m) {
        using std::swap;
        swap(m_first, m.m_first);
        swap(m_second, m.m_second);
    }

    //! @brief Equality operator.
    bool operator==(twin const& o) const {
        return m_first == o.m_first && m_second == o.m_second;
    }

    //! @brief Access to the first element.
    T& first() {
        return m_first;
    }

    //! @brief Const access to the first element.
    T const& first() const {
        return m_first;
    }

    //! @brief Access to the second element.
    T& second() {
        return m_second;
    }

    //! @brief Const access to the second element.
    T const& second() const {
        return m_second;
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_first & m_second;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_first << m_second;
    }

  private:
    //! @brief The content of the class.
    T m_first, m_second;
};


//! @brief Exchanges contents of twin objects.
template <typename T, bool is_twin>
void swap(twin<T, is_twin>& x, twin<T, is_twin>& y) {
    x.swap(y);
}


}


}

#endif // FCPP_INTERNAL_TWIN_H_
