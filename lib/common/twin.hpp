// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

/**
 * @file twin.hpp
 * @brief Implementation of the `twin` class for handling two variables, possibly identical.
 */

#ifndef FCPP_COMMON_TWIN_H_
#define FCPP_COMMON_TWIN_H_


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


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
    
  private:
    //! @brief The content of the class.
    T data;
    
  public:
    //! @name constructors
    //@{
    //! @brief Default constructor.
    twin() = default;
    
    //! @brief Copy constructor.
    twin(const twin<T, true>&) = default;
    
    //! @brief Move constructor.
    twin(twin<T, true>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    twin<T, true>& operator=(const twin<T, true>&) = default;
    
    //! @brief Move assignment.
    twin<T, true>& operator=(twin<T, true>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const twin<T, true>& o) const {
        return data == o.data;
    }
    
    //! @brief Access to the first element.
    T& first() {
        return data;
    }

    //! @brief Const access to the first element.
    const T& first() const {
        return data;
    }

    //! @brief Access to the second element.
    T& second() {
        return data;
    }

    //! @brief Const access to the second element.
    const T& second() const {
        return data;
    }
};


//! @brief Class offering access to different values through `first()` and `second()`.
template <typename T>
class twin<T, false> {
  public:
    //! @brief The type of the content.
    typedef T value_type;
    
  private:
    //! @brief The content of the class.
    T data1, data2;
    
  public:
    //! @name constructors
    //@{
    //! @brief Default constructor.
    twin() = default;
    
    //! @brief Copy constructor.
    twin(const twin<T, false>&) = default;
    
    //! @brief Move constructor.
    twin(twin<T, false>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    twin<T, false>& operator=(const twin<T, false>&) = default;
    
    //! @brief Move assignment.
    twin<T, false>& operator=(twin<T, false>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const twin<T, false>& o) const {
        return data1 == o.data1 && data2 == o.data2;
    }
    
    //! @brief Access to the first element.
    T& first() {
        return data1;
    }

    //! @brief Const access to the first element.
    const T& first() const {
        return data1;
    }

    //! @brief Access to the second element.
    T& second() {
        return data2;
    }

    //! @brief Const access to the second element.
    const T& second() const {
        return data2;
    }
};


}

#endif // FCPP_COMMON_TWIN_H_
