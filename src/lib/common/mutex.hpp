// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file mutex.hpp
 * @brief Implementation of the `mutex` class used to manage synchronization in parallel computations and accessory functions (uniform interface for sequential execution and OpenMP and C++14 threads).
 */

#ifndef FCPP_COMMON_MUTEX_H_
#define FCPP_COMMON_MUTEX_H_

#include <cstdint>

#include <chrono>
#ifndef FCPP_DISABLE_THREADS
#include <mutex>
#include <shared_mutex>
#include <thread>
#endif
#if defined(_OPENMP)
#include <omp.h>
#endif

//! @cond INTERNAL
#ifdef FCPP_DISABLE_THREADS
namespace std {
    struct adopt_lock_t {};
    struct defer_lock_t {};
    struct try_to_lock_t {};

    constexpr adopt_lock_t adopt_lock{};
    constexpr defer_lock_t defer_lock{};
    constexpr try_to_lock_t try_to_lock{};

    namespace this_thread {
        inline void yield() {}
        template <typename C, typename D>
        void sleep_until(chrono::time_point<C,D> const&) {}
        template <typename R, typename P>
        void sleep_for(chrono::duration<R,P> const&) {}
    };

    template <typename M>
    struct lock_guard;
    template <typename M>
    struct unique_lock;
    template <typename M>
    struct shared_lock;
}
#endif
//! @endcond


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
struct mutex;


//! @brief Empty mutex interface when `enabled` is false.
template <>
struct mutex<false> {
    //! @brief Default constructor.
    mutex() = default;

    //! @brief Deleted copy constructor.
    mutex(mutex const&) = delete;

    //! @brief Tries to acquire the lock, returning false if not available.
    inline bool try_lock() {
        return true;
    }

    //! @brief Acquires the lock, waiting if not available.
    inline void lock() {}

    //! @brief Releases the lock.
    inline void unlock() {}
};


#ifdef FCPP_DISABLE_THREADS
//! @brief Disabled mutex interface when threads are not available.
template <>
struct mutex<true> : public mutex<false> {};
#elif defined(_OPENMP)
//! @brief Actual mutex interface when `enabled` is true and OpenMP is available.
template <>
struct mutex<true> {
    //! @brief Default constructor.
    mutex() {
        omp_init_lock(&m_lock);
    }

    //! @brief Default destructor.
    ~mutex() {
        omp_destroy_lock(&m_lock);
    }

    //! @brief Deleted copy constructor.
    mutex(mutex const&) = delete;

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

  private:
    //! @brief The actual lock.
    omp_lock_t m_lock;
};
#else
//! @brief Actual mutex interface when `enabled` is true and the standard C++ `mutex` is available.
template <>
struct mutex<true> : public std::mutex {};
#endif


/**
 * @brief Provides an uniform object interface for an OpenMP or C++14 mutex.
 *
 * @param enabled Whether the mutex will actually perform anything.
 */
template <bool enabled>
struct shared_mutex;


//! @brief Empty mutex interface when `enabled` is false.
template <>
struct shared_mutex<false> {
    //! @brief Default constructor.
    shared_mutex() = default;

    //! @brief Deleted copy constructor.
    shared_mutex(shared_mutex const&) = delete;

    //! @brief Tries to acquire the exclusive lock, returning false if not available.
    inline bool try_lock() {
        return true;
    }

    //! @brief Acquires the exclusive lock, waiting if not available.
    inline void lock() {}

    //! @brief Releases the exclusive lock.
    inline void unlock() {}

    //! @brief Tries to acquire the shared lock, returning false if not available.
    inline bool try_lock_shared() {
        return true;
    }

    //! @brief Acquires the shared lock, waiting if not available.
    inline void lock_shared() {}

    //! @brief Releases the shared lock.
    inline void unlock_shared() {}
};


#ifdef FCPP_DISABLE_THREADS
//! @brief Disabled shared mutex interface when threads are not available.
template <>
struct shared_mutex<true> : public shared_mutex<false> {};
#elif defined(_OPENMP)
//! @brief Actual shared mutex interface when `enabled` is true and OpenMP is available.
template <>
struct shared_mutex<true> {
    //! @brief Default constructor.
    shared_mutex() {
        omp_init_lock(&m_lock);
    }

    //! @brief Default destructor.
    ~shared_mutex() {
        omp_destroy_lock(&m_lock);
    }

    //! @brief Deleted copy constructor.
    shared_mutex(shared_mutex const&) = delete;

    //! @brief Tries to acquire the exclusive lock, returning false if not available.
    inline bool try_lock() {
        if (not omp_test_lock(&m_lock)) return false;
        if (m_counter == 0) return true;
        omp_unset_lock(&m_lock);
        return false;
    }

    //! @brief Acquires the exclusive lock, waiting if not available.
    inline void lock() {
        while (true) {
            omp_set_lock(&m_lock);
            if (m_counter == 0) return;
            omp_unset_lock(&m_lock);
        }
    }

    //! @brief Releases the exclusive lock.
    inline void unlock() {
        omp_unset_lock(&m_lock);
    }

    //! @brief Tries to acquire the shared lock, returning false if not available.
    inline bool try_lock_shared() {
        if (not omp_test_lock(&m_lock)) return false;
        ++m_counter;
        omp_unset_lock(&m_lock);
    }

    //! @brief Acquires the shared lock, waiting if not available.
    inline void lock_shared() {
        omp_set_lock(&m_lock);
        ++m_counter;
        omp_unset_lock(&m_lock);
    }

    //! @brief Releases the shared lock.
    inline void unlock_shared() {
        omp_set_lock(&m_lock);
        --m_counter;
        omp_unset_lock(&m_lock);
    }

  private:
    //! @brief The actual lock.
    omp_lock_t m_lock;
    //! @brief The number of shared locks aquired.
    uint16_t m_counter;
};
#elif __cplusplus >= 201700
//! @brief Actual shared mutex interface when `enabled` is true and the standard C++17 `shared_mutex` is available.
template <>
struct shared_mutex<true> : public std::shared_mutex {};
#else
//! @brief Actual shared mutex interface when `enabled` is true and the standard C++14 `shared_timed_mutex` is available.
template <>
struct shared_mutex<true> : public std::shared_timed_mutex {};
#endif


//! @cond INTERNAL
namespace details {
    //! @brief Unlocker of a mutex during its lifetime.
    template <typename M>
    struct unlock_guard {
        //! @brief Unlocking constructor.
        explicit unlock_guard(M& m) : m_mutex(m) {
            m_mutex.unlock();
        }

        //! @brief Adopting constructor.
        unlock_guard(M& m, std::adopt_lock_t) : m_mutex(m) {}

        //! @brief Disabled copying.
        unlock_guard(unlock_guard const&) = delete;

        //! @brief Locking destructor.
        ~unlock_guard() {
            m_mutex.lock();
        }

      private:
        M& m_mutex;
    };

    //! @brief Shared locker of a mutex during its lifetime.
    template <typename M>
    struct shared_guard {
        //! @brief Locking constructor.
        explicit shared_guard(M& m) : m_mutex(m) {
            m_mutex.lock_shared();
        }

        //! @brief Adopting constructor.
        shared_guard(M& m, std::adopt_lock_t) : m_mutex(m) {}

        //! @brief Disabled copying.
        shared_guard(shared_guard const&) = delete;

        //! @brief Locking destructor.
        ~shared_guard() {
            m_mutex.unlock_shared();
        }

      private:
        M& m_mutex;
    };

    //! @brief Bypassed version of a guard (lock or unlock; and unique, shared or exclusive).
    template <typename M>
    struct bypassed_guard {
        //! @brief Locking constructor.
        explicit bypassed_guard(M&) {}

        //! @brief Adopting constructor.
        bypassed_guard(M&, std::adopt_lock_t) {}

        //! @brief Disabled copying.
        bypassed_guard(bypassed_guard const&) = delete;
    };

    //! @brief Bypassed version of a lock (unique, shared or exclusive).
    template <typename M>
    struct bypassed_lock {
        bypassed_lock() noexcept {}

        explicit bypassed_lock(M&) {}

        bypassed_lock(M&, std::try_to_lock_t) {}

        bypassed_lock(M&, std::defer_lock_t) noexcept {}

        bypassed_lock(M&, std::adopt_lock_t) {}

        template <class R, class P>
        bypassed_lock(M&, std::chrono::duration<R,P> const&) {}

        template <class C, class D>
        bypassed_lock(M&, std::chrono::time_point<C,D> const&) {}

        bypassed_lock(bypassed_lock const&) = delete;

        bypassed_lock(bypassed_lock&&) {}

        inline bypassed_lock& operator=(bypassed_lock&&) {
            return *this;
        }

        inline bypassed_lock& operator=(bypassed_lock const&) = delete;

        inline void swap(bypassed_lock&) noexcept {}

        inline M* release() noexcept {
            return new M();
        }

        inline void lock() {}

        inline bool try_lock() {
            return true;
        }

        template <class R, class P>
        inline bool try_lock_for(std::chrono::duration<R,P> const&) {
            return true;
        }

        template <class C, class D>
        inline bool try_lock_until(std::chrono::time_point<C,D> const&) {
            return true;
        }

        inline void unlock() {}

        inline bool owns_lock() const noexcept {
            return true;
        }

        inline explicit operator bool() const noexcept {
            return true;
        }

        inline M* mutex() const noexcept {
            return new M();
        }
    };

    //! @brief Template selecting between a bypassed and an active version of a locker class.
    template <bool enabled, template<bool> class M, template<class> class B, template<class> class A>
    using generic_locker = std::conditional_t<
        enabled,
#ifdef FCPP_DISABLE_THREADS
        B<M<enabled>>,
#else
        A<M<enabled>>,
#endif
        B<M<enabled>>
    >;
}
//! @endcond


//! @brief Bypassable version of `std::lock_guard` (keeps a mutex locked during its lifetime).
template <bool enabled>
using lock_guard = details::generic_locker<enabled, mutex, details::bypassed_guard, std::lock_guard>;


//! @brief Bypassable unlocker of a mutex during its lifetime.
template <bool enabled>
using unlock_guard = details::generic_locker<enabled, mutex, details::bypassed_guard, details::unlock_guard>;


//! @brief Bypassable version of a shared lock guard on a shared mutex.
template <bool enabled>
using shared_guard = details::generic_locker<enabled, shared_mutex, details::bypassed_guard, details::shared_guard>;


//! @brief Bypassable version of a exclusive lock guard on a shared mutex.
template <bool enabled>
using exclusive_guard = details::generic_locker<enabled, shared_mutex, details::bypassed_guard, std::lock_guard>;


//! @brief Bypassable version of `std::unique_lock` (manages a mutex through lifetime).
template <bool enabled>
using unique_lock = details::generic_locker<enabled, mutex, details::bypassed_lock, std::unique_lock>;


//! @brief Bypassable version of `std::shared_lock` (manages a shared_mutex through lifetime).
template <bool enabled>
using shared_lock = details::generic_locker<enabled, shared_mutex, details::bypassed_lock, std::shared_lock>;


//! @brief Bypassable version of `std::unique_lock` on a shared mutex.
template <bool enabled>
using exclusive_lock = details::generic_locker<enabled, shared_mutex, details::bypassed_lock, std::unique_lock>;


//! @brief Function locking multiple mutexes (bypassed version).
template <typename... Ts>
inline void lock(mutex<false>&, Ts&...) {}

#ifdef FCPP_DISABLE_THREADS
//! @brief Function locking multiple mutexes (active version).
template <typename... Ts>
inline void lock(mutex<true>&, Ts&...) {}
#else
//! @brief Function locking multiple mutexes (active version).
template <typename... Ts>
inline void lock(mutex<true>& a, Ts&... ms) {
    std::lock(a, ms...);
}
#endif

//! @brief Function trying to lock multiple mutexes (bypassed version).
template <typename... Ts>
inline int try_lock(mutex<false>&, Ts&...) {
    return -1;
}

#ifdef FCPP_DISABLE_THREADS
//! @brief Function trying to lock multiple mutexes (active version).
template <typename... Ts>
inline int try_lock(mutex<true>&, Ts&...) {
    return -1;
}
#else
//! @brief Function trying to lock multiple mutexes (active version).
template <typename... Ts>
inline int try_lock(mutex<true>& a, Ts&... ms) {
    return std::try_lock(a, ms...);
}
#endif


}


}

#endif // FCPP_COMMON_MUTEX_H_
