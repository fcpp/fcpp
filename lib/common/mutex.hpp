// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file mutex.hpp
 * @brief Implementation of the `mutex` class used to manage synchronization in parallel computations and accessory functions (uniform interface for parallel, OpenMP and C++11 threads).
 */

#ifndef FCPP_COMMON_MUTEX_H_
#define FCPP_COMMON_MUTEX_H_

#include <chrono>
#include <mutex>
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
class mutex;


//! @brief Empty mutex interface when `enabled` is false.
template <>
struct mutex<false> {
    //! @brief Default constructor.
    mutex() = default;
    
    //! @brief Deleted copy constructor.
    mutex(const mutex&) = delete;
    
    //! @brief Tries to acquire the lock, returning false if not available.
    inline bool try_lock() {
        return true;
    }
    
    //! @brief Acquires the lock, waiting if not available.
    inline void lock() {}
    
    //! @brief Releases the lock.
    inline void unlock() {}
};


#if defined(_OPENMP)
//! @brief Actual mutex interface when `enabled` is true and OpenMP is available.
template <>
class mutex<true> {
    //! @brief The actual lock.
    omp_lock_t m_lock;

  public:
    //! @brief Default constructor.
    mutex() {
        omp_init_lock(&m_lock);
    }
    
    //! @brief Default destructor.
    ~mutex() {
        omp_destroy_lock(&m_lock);
    }
    
    //! @brief Deleted copy constructor.
    mutex(const mutex&) = delete;

    //! @brief Tries to acquire the lock, returning false if not available.
    inline bool try_lock() {
        return omp_test_lock(&m_lock);
    }

    //! @brief Acquires the lock, waiting if not available.
    inline void lock() {
        omp_set_lock(&m_lock);
    }

    //! @brief Releases the lock.
    inline void unlock() {
        omp_unset_lock(&m_lock);
    }
};
#else
//! @brief Actual mutex interface when `enabled` is true and the standard C++ `mutex` is available.
template <>
struct mutex<true> : public std::mutex {
    using std::mutex::mutex;
};
#endif


using std::adopt_lock_t;
using std::adopt_lock;
using std::try_to_lock_t;
using std::try_to_lock;
using std::defer_lock_t;
using std::defer_lock;


//! @brief Bypassable version of `std::lock_guard` (keeps a mutex locked during its lifetime).
template <bool enabled>
class lock_guard;


//! @brief Bypassed version of `std::lock_guard`.
template <>
struct lock_guard<false> {
    //! @brief Locking constructor.
    explicit lock_guard(mutex<false>&) {}
    
    //! @brief Adopting constructor.
    lock_guard(mutex<false>&, adopt_lock_t) {}
    
    //! @brief Disabled copying.
    lock_guard(const lock_guard&) = delete;
};


//! @brief Imported version of `std::lock_guard`.
template <>
struct lock_guard<true> : public std::lock_guard<mutex<true>> {
    using std::lock_guard<mutex<true>>::lock_guard;
};


//! @brief Bypassable version of `std::unique_lock` (manages a mutex through lifetime).
template <bool enabled>
class unique_lock;


//! @brief Bypassed version of `std::unique_lock`.
template <>
class unique_lock<false> {
    unique_lock() noexcept {}
    
    explicit unique_lock(fcpp::mutex<false>&) {}
    
    unique_lock(fcpp::mutex<false>&, try_to_lock_t) {}
    
    unique_lock(fcpp::mutex<false>&, defer_lock_t) noexcept {}

    unique_lock(fcpp::mutex<false>&, adopt_lock_t) {}
    
    template <class R, class P>
    unique_lock(fcpp::mutex<false>&, const std::chrono::duration<R,P>&) {}
    
    template <class C, class D>
    unique_lock(fcpp::mutex<false>&, const std::chrono::time_point<C,D>&) {}
    
    unique_lock(const unique_lock&) = delete;
    
    unique_lock(unique_lock&&) {}
    
    inline unique_lock& operator=(unique_lock&&) {
        return *this;
    }
    
    inline unique_lock& operator=(const unique_lock&) = delete;
    
    inline void swap (unique_lock&) noexcept {}
    
    inline fcpp::mutex<false>* release() noexcept {
        return new fcpp::mutex<false>();
    }
    
    inline void lock() {}
    
    inline bool try_lock() {
        return true;
    }
    
    template <class R, class P>
    inline bool try_lock_for(const std::chrono::duration<R,P>&) {
        return true;
    }
    
    template <class C, class D>
    inline bool try_lock_until(const std::chrono::time_point<C,D>&) {
        return true;
    }
    
    inline void unlock() {}
    
    inline bool owns_lock() const noexcept {
        return true;
    }
    
    inline explicit operator bool() const noexcept {
        return true;
    }
    
    inline fcpp::mutex<false>* mutex() const noexcept {
        return new fcpp::mutex<false>();
    }
};

void swap(unique_lock<false>&, unique_lock<false>&) noexcept {}


//! @brief Imported version of `std::unique_lock`.
template <>
struct unique_lock<true> : std::unique_lock<fcpp::mutex<true>> {
    using std::unique_lock<fcpp::mutex<true>>::unique_lock;
};


}

#endif // FCPP_COMMON_MUTEX_H_
