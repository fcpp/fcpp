// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file mutex.hpp
 * @brief Implementation of the `mutex` class used to manage synchronization in parallel computations and accessory functions (uniform interface for sequential execution and OpenMP and C++14 threads).
 */

#ifndef FCPP_COMMON_MUTEX_H_
#define FCPP_COMMON_MUTEX_H_

#include <cstdint>

#include <chrono>
#include <mutex>
//#include <shared_mutex>
#if defined(_OPENMP)
#include <omp.h>
#endif


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


/**
 * @brief Provides an uniform object interface for an OpenMP or C++14 mutex.
 *
 * @param enabled Whether the mutex will actually perform anything.
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


///**
// * @brief Provides an uniform object interface for an OpenMP or C++14 mutex.
// *
// * @param enabled Whether the mutex will actually perform anything.
// */
//template <bool enabled>
//class shared_mutex;
//
//
////! @brief Empty mutex interface when `enabled` is false.
//template <>
//struct shared_mutex<false> {
//    //! @brief Default constructor.
//    shared_mutex() = default;
//
//    //! @brief Deleted copy constructor.
//    shared_mutex(const shared_mutex&) = delete;
//
//    //! @brief Tries to acquire the exclusive lock, returning false if not available.
//    inline bool try_lock() {
//        return true;
//    }
//
//    //! @brief Acquires the exclusive lock, waiting if not available.
//    inline void lock() {}
//
//    //! @brief Releases the exclusive lock.
//    inline void unlock() {}
//
//    //! @brief Tries to acquire the shared lock, returning false if not available.
//    inline bool try_lock_shared() {
//        return true;
//    }
//
//    //! @brief Acquires the shared lock, waiting if not available.
//    inline void lock_shared() {}
//
//    //! @brief Releases the shared lock.
//    inline void unlock_shared() {}
//};
//
//
//#if defined(_OPENMP)
////! @brief Actual mutex interface when `enabled` is true and OpenMP is available.
//template <>
//class shared_mutex<true> {
//    //! @brief The actual lock.
//    omp_lock_t m_lock;
//    //! @brief The number of shared locks aquired.
//    uint16_t m_counter;
//
//  public:
//    //! @brief Default constructor.
//    shared_mutex() {
//        omp_init_lock(&m_lock);
//    }
//
//    //! @brief Default destructor.
//    ~shared_mutex() {
//        omp_destroy_lock(&m_lock);
//    }
//
//    //! @brief Deleted copy constructor.
//    shared_mutex(const shared_mutex&) = delete;
//
//    //! @brief Tries to acquire the exclusive lock, returning false if not available.
//    inline bool try_lock() {
//        if (not omp_test_lock(&m_lock)) return false;
//        if (m_counter == 0) return true;
//        omp_unset_lock(&m_lock);
//        return false;
//    }
//
//    //! @brief Acquires the exclusive lock, waiting if not available.
//    inline void lock() {
//        while (true) {
//            omp_set_lock(&m_lock);
//            if (m_counter == 0) return;
//            omp_unset_lock(&m_lock);
//        }
//    }
//
//    //! @brief Releases the exclusive lock.
//    inline void unlock() {
//        omp_unset_lock(&m_lock);
//    }
//
//    //! @brief Tries to acquire the shared lock, returning false if not available.
//    inline bool try_lock_shared() {
//        if (not omp_test_lock(&m_lock)) return false;
//        ++m_counter;
//        omp_unset_lock(&m_lock);
//    }
//
//    //! @brief Acquires the shared lock, waiting if not available.
//    inline void lock_shared() {
//        omp_set_lock(&m_lock);
//        ++m_counter;
//        omp_unset_lock(&m_lock);
//    }
//
//    //! @brief Releases the shared lock.
//    inline void unlock_shared() {
//        omp_set_lock(&m_lock);
//        --m_counter;
//        omp_unset_lock(&m_lock);
//    }
//};
//#elif __cplusplus >= 201700
////! @brief Actual mutex interface when `enabled` is true and the standard C++17 `shared_mutex` is available.
//template <>
//struct shared_mutex<true> : public std::shared_mutex {
//    using std::shared_mutex::shared_mutex;
//};
//#else
////! @brief Actual mutex interface when `enabled` is true and the standard C++14 `shared_timed_mutex` is available.
//template <>
//struct shared_mutex<true> : public std::shared_timed_mutex {
//    using std::shared_timed_mutex::shared_timed_mutex;
//};
//#endif


//! @brief Bypassable version of `std::lock_guard` (keeps a mutex locked during its lifetime).
template <bool enabled>
class lock_guard;


//! @brief Bypassed version of `std::lock_guard`.
template <>
struct lock_guard<false> {
    //! @brief Locking constructor.
    explicit lock_guard(mutex<false>&) {}

    //! @brief Adopting constructor.
    lock_guard(mutex<false>&, std::adopt_lock_t) {}

    //! @brief Disabled copying.
    lock_guard(const lock_guard&) = delete;
};


//! @brief Imported version of `std::lock_guard`.
template <>
struct lock_guard<true> : public std::lock_guard<mutex<true>> {
    using std::lock_guard<mutex<true>>::lock_guard;
};


//! @brief Bypassable unlocker of a mutex during its lifetime.
template <bool enabled>
class unlock_guard;


//! @brief Bypassed version of `unlock_guard`.
template <>
struct unlock_guard<false> {
    //! @brief Locking constructor.
    explicit unlock_guard(mutex<false>&) {}

    //! @brief Adopting constructor.
    unlock_guard(mutex<false>&, std::adopt_lock_t) {}

    //! @brief Disabled copying.
    unlock_guard(const unlock_guard&) = delete;
};


//! @brief Active version of `unlock_guard`.
template <>
struct unlock_guard<true> {
    //! @brief Unlocking constructor.
    explicit unlock_guard(mutex<true>& m) : m_mutex(m) {
        m_mutex.unlock();
    }

    //! @brief Adopting constructor.
    unlock_guard(mutex<true>& m, std::adopt_lock_t) : m_mutex(m) {}

    //! @brief Disabled copying.
    unlock_guard(const unlock_guard&) = delete;

    //! @brief Locking destructor.
    ~unlock_guard() {
        m_mutex.lock();
    }

  private:
    mutex<true>& m_mutex;
};


//! @brief Bypassable version of `std::unique_lock` (manages a mutex through lifetime).
template <bool enabled>
class unique_lock;


//! @brief Bypassed version of `std::unique_lock`.
template <>
struct unique_lock<false> {
    unique_lock() noexcept {}

    explicit unique_lock(common::mutex<false>&) {}

    unique_lock(common::mutex<false>&, std::try_to_lock_t) {}

    unique_lock(common::mutex<false>&, std::defer_lock_t) noexcept {}

    unique_lock(common::mutex<false>&, std::adopt_lock_t) {}

    template <class R, class P>
    unique_lock(common::mutex<false>&, const std::chrono::duration<R,P>&) {}

    template <class C, class D>
    unique_lock(common::mutex<false>&, const std::chrono::time_point<C,D>&) {}

    unique_lock(const unique_lock&) = delete;

    unique_lock(unique_lock&&) {}

    inline unique_lock& operator=(unique_lock&&) {
        return *this;
    }

    inline unique_lock& operator=(const unique_lock&) = delete;

    inline void swap(unique_lock&) noexcept {}

    inline common::mutex<false>* release() noexcept {
        return new common::mutex<false>();
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

    inline common::mutex<false>* mutex() const noexcept {
        return new common::mutex<false>();
    }
};

inline void swap(unique_lock<false>&, unique_lock<false>&) noexcept {}


//! @brief Imported version of `std::unique_lock`.
template <>
struct unique_lock<true> : std::unique_lock<mutex<true>> {
    using std::unique_lock<common::mutex<true>>::unique_lock;
};


////! @brief Bypassable version of `std::shared_lock` (manages a shared_mutex through lifetime).
//template <bool enabled>
//class shared_lock;
//
//
////! @brief Bypassed version of `std::shared_lock`.
//template <>
//struct shared_lock<false> {
//    shared_lock() noexcept {}
//
//    explicit shared_lock(common::shared_mutex<false>&) {}
//
//    shared_lock(common::shared_mutex<false>&, std::try_to_lock_t) {}
//
//    shared_lock(common::shared_mutex<false>&, std::defer_lock_t) noexcept {}
//
//    shared_lock(common::shared_mutex<false>&, std::adopt_lock_t) {}
//
//    template <class R, class P>
//    shared_lock(common::shared_mutex<false>&, const std::chrono::duration<R,P>&) {}
//
//    template <class C, class D>
//    shared_lock(common::shared_mutex<false>&, const std::chrono::time_point<C,D>&) {}
//
//    shared_lock(const shared_lock&) = delete;
//
//    shared_lock(shared_lock&&) {}
//
//    inline shared_lock& operator=(shared_lock&&) {
//        return *this;
//    }
//
//    inline shared_lock& operator=(const shared_lock&) = delete;
//
//    inline void swap(shared_lock&) noexcept {}
//
//    inline common::shared_mutex<false>* release() noexcept {
//        return new common::shared_mutex<false>();
//    }
//
//    inline void lock() {}
//
//    inline bool try_lock() {
//        return true;
//    }
//
//    template <class R, class P>
//    inline bool try_lock_for(const std::chrono::duration<R,P>&) {
//        return true;
//    }
//
//    template <class C, class D>
//    inline bool try_lock_until(const std::chrono::time_point<C,D>&) {
//        return true;
//    }
//
//    inline void unlock() {}
//
//    inline bool owns_lock() const noexcept {
//        return true;
//    }
//
//    inline explicit operator bool() const noexcept {
//        return true;
//    }
//
//    inline common::shared_mutex<false>* mutex() const noexcept {
//        return new common::shared_mutex<false>();
//    }
//};
//
//inline void swap(shared_lock<false>&, shared_lock<false>&) noexcept {}
//
//
////! @brief Imported version of `std::shared_lock`.
//template <>
//struct shared_lock<true> : std::shared_lock<shared_mutex<true>> {
//    using std::shared_lock<common::shared_mutex<true>>::shared_lock;
//};


template <typename... Ts>
void lock(mutex<false>&, Ts&...) {}

template <typename... Ts>
void lock(mutex<true>& a, Ts&... ms) {
    std::lock(a, ms...);
}

template <typename... Ts>
int try_lock(mutex<false>&, Ts&...) {
    return -1;
}

template <typename... Ts>
int try_lock(mutex<true>& a, Ts&... ms) {
    return std::try_lock(a, ms...);
}


}


}

#endif // FCPP_COMMON_MUTEX_H_
