// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file lock.hpp
 * @brief Implementation of the class used to manage synchronization in parallel computations.
 */

#ifndef FCPP_COMMON_LOCK_H_
#define FCPP_COMMON_LOCK_H_

#if defined(_OPENMP)
#include <omp.h>
#endif


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Provides an object interface for an OpenMP lock.
 *
 * @param enabled Whether the lock will actually perform anything.
 */
template <bool enabled>
class lock;


//! @brief Empty interface when `enabled` is false.
template <>
struct lock<false> {
    inline bool test() {
        return true;
    }
    inline void set() {}
    inline void unset() {}
};


#ifdef _OPENMP
//! @brief Actual interface when `enabled` is true and OpenMP is available.
template <>
class lock<true> {
    // The actual lock.
    omp_lock_t m_lock;

  public:
    //! @brief Initialises the lock.
    lock() {
        omp_init_lock(&m_lock);
    }
    
    //! @brief Destroys the lock.
    ~lock() {
        omp_destroy_lock(&m_lock);
    }
    
    //! @brief Tries to acquire the lock, returning false if not available.
    inline bool test() {
        return omp_test_lock(&m_lock);
    }

    //! @brief Acquires the lock, waiting if not available.
    inline void set() {
        omp_set_lock(&m_lock);
    }

    //! @brief Releases the lock.
    inline void unset() {
        omp_unset_lock(&m_lock);
    }
};
#endif


}

#endif // FCPP_COMMON_LOCK_H_
