// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file flat_ptr.hpp
 * @brief Implementation of the `flat_ptr` class for handling either a `shared_ptr` or a flat data.
 */

#ifndef FCPP_INTERNAL_FLAT_PTR_H_
#define FCPP_INTERNAL_FLAT_PTR_H_

#include <memory>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing objects of internal use.
namespace internal {


//! @cond INTERNAL
template <typename T, bool is_flat>
class flat_ptr;
//! @endcond


//! @brief Class managing a `T` object exactly as a `shared_ptr`.
template <typename T>
class flat_ptr<T, false> {
  public:
    //! @brief The type of the content.
    typedef T value_type;

    //! @name constructors
    //! @{

    //! @brief Default constructor.
    flat_ptr() {
        m_data.reset(new T());
    }

    //! @brief Default copying constructor.
    flat_ptr(T const& d) {
        m_data.reset(new T(d));
    }

    //! @brief Default moving constructor.
    flat_ptr(T&& d) {
        m_data.reset(new T(d));
    }

    //! @brief Copy constructor.
    flat_ptr(flat_ptr const&) = default;

    //! @brief Move constructor.
    flat_ptr(flat_ptr&&) = default;
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Default copying assignment.
    flat_ptr& operator=(T const& d) {
        m_data.reset(new T(d));
        return *this;
    }

    //! @brief Default moving assignment.
    flat_ptr& operator=(T&& d) {
        m_data.reset(new T(d));
        return *this;
    }

    //! @brief Copy assignment.
    flat_ptr& operator=(flat_ptr const&) = default;

    //! @brief Move assignment.
    flat_ptr& operator=(flat_ptr&&) = default;
    //! @}

    //! @brief Exchanges contents of flat pointers.
    void swap(flat_ptr& m) {
        m_data.swap(m.m_data);
    }

    //! @brief Equality operator.
    bool operator==(flat_ptr const& o) const {
        return *m_data == *(o.m_data);
    }

    //! @brief Access to the content.
    T& operator*() {
        return *m_data.get();
    }

    //! @brief Arrow access to the content.
    T* operator->() {
        return m_data.get();
    }

    //! @brief Const access to the content.
    T const& operator*() const {
        return *m_data.get();
    }

    //! @brief Const arrow access to the content.
    T const* operator->() const {
        return m_data.get();
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & *m_data.get();
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << *m_data.get();
    }

  private:
    //! @brief The content of the class.
    std::shared_ptr<T> m_data;
};


//! @brief Class managing a `T` object directly.
template <typename T>
class flat_ptr<T, true> {
  public:
    //! @brief The type of the content.
    typedef T value_type;

    //! @name constructors
    //! @{

    //! @brief Default constructor.
    flat_ptr() : m_data() {}

    //! @brief Default copying constructor.
    flat_ptr(T const& d) : m_data(d) {}

    //! @brief Default moving constructor.
    flat_ptr(T&& d) : m_data(d) {}

    //! @brief Copy constructor.
    flat_ptr(flat_ptr const&) = default;

    //! @brief Move constructor.
    flat_ptr(flat_ptr&&) = default;
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Default copying assignment.
    flat_ptr& operator=(T const& d) {
        m_data = d;
        return *this;
    }

    //! @brief Default moving assignment.
    flat_ptr& operator=(T&& d) {
        m_data = d;
        return *this;
    }

    //! @brief Copy assignment.
    flat_ptr& operator=(flat_ptr const&) = default;

    //! @brief Move assignment.
    flat_ptr& operator=(flat_ptr&&) = default;
    //! @}

    //! @brief Exchanges contents of flat pointers.
    void swap(flat_ptr& m) {
        using std::swap;
        swap(m_data, m.m_data);
    }

    //! @brief Equality operator.
    bool operator==(flat_ptr const& o) const {
        return m_data == o.m_data;
    }

    //! @brief Access to the content.
    T& operator*() {
        return m_data;
    }

    //! @brief Arrow access to the content.
    T* operator->() {
        return &m_data;
    }

    //! @brief Const access to the content.
    T const& operator*() const {
        return m_data;
    }

    //! @brief Const arrow access to the content.
    T const* operator->() const {
        return &m_data;
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


//! @brief Exchanges contents of flat pointers.
template <typename T, bool is_flat>
void swap(flat_ptr<T, is_flat>& x, flat_ptr<T, is_flat>& y) {
    x.swap(y);
}


}


}

#endif // FCPP_INTERNAL_FLAT_PTR_H_
