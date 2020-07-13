// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

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
    
  private:
    //! @brief The content of the class.
    std::shared_ptr<T> m_data;
    
  public:
    //! @name constructors
    //@{
    //! @brief Default constructor.
    flat_ptr() {
        m_data.reset(new T());
    }
    
    //! @brief Default copying constructor.
    flat_ptr(const T& d) {
        m_data.reset(new T(d));
    };
    
    //! @brief Default moving constructor.
    flat_ptr(T&& d) {
        m_data.reset(new T(d));
    };
    
    //! @brief Copy constructor.
    flat_ptr(const flat_ptr<T, false>&) = default;
    
    //! @brief Move constructor.
    flat_ptr(flat_ptr<T, false>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Default copying assignment.
    flat_ptr<T, false>& operator=(const T& d) {
        m_data.reset(new T(d));
        return *this;
    };
    
    //! @brief Default moving assignment.
    flat_ptr<T, false>& operator=(T&& d) {
        m_data.reset(new T(d));
        return *this;
    };
    
    //! @brief Copy assignment.
    flat_ptr<T, false>& operator=(const flat_ptr<T, false>&) = default;
    
    //! @brief Move assignment.
    flat_ptr<T, false>& operator=(flat_ptr<T, false>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const flat_ptr<T, false>& o) const {
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
    const T& operator*() const {
        return *m_data.get();
    }
    
    //! @brief Const arrow access to the content.
    const T* operator->() const {
        return m_data.get();
    }
};


//! @brief Class managing a `T` object directly.
template <typename T>
class flat_ptr<T, true> {
  public:
    //! @brief The type of the content.
    typedef T value_type;
    
  private:
    //! @brief The content of the class.
    T m_data;
    
  public:
    //! @name constructors
    //@{
    //! @brief Default constructor.
    flat_ptr() = default;
    
    //! @brief Default copying constructor.
    flat_ptr(const T& d) : m_data(d) {};
    
    //! @brief Default moving constructor.
    flat_ptr(T&& d) : m_data(d) {};
    
    //! @brief Copy constructor.
    flat_ptr(const flat_ptr<T, true>&) = default;
    
    //! @brief Move constructor.
    flat_ptr(flat_ptr<T, true>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Default copying assignment.
    flat_ptr<T, true>& operator=(const T& d) {
        m_data = d;
        return *this;
    };
    
    //! @brief Default moving assignment.
    flat_ptr<T, true>& operator=(T&& d) {
        m_data = d;
        return *this;
    };
    
    //! @brief Copy assignment.
    flat_ptr<T, true>& operator=(const flat_ptr<T, true>&) = default;
    
    //! @brief Move assignment.
    flat_ptr<T, true>& operator=(flat_ptr<T, true>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const flat_ptr<T, true>& o) const {
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
    const T& operator*() const {
        return m_data;
    }
    
    //! @brief Const arrow access to the content.
    const T* operator->() const {
        return &m_data;
    }
};


}


}

#endif // FCPP_INTERNAL_FLAT_PTR_H_
