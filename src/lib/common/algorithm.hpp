// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file algorithm.hpp
 * @brief Implementation of common algorithms useful in the library.
 */

#ifndef FCPP_COMMON_ALGORITHM_H_
#define FCPP_COMMON_ALGORITHM_H_

#include <algorithm>
#include <iterator>
#include <set>
#include <type_traits>
#include <unordered_set>
#include <vector>
#ifndef FCPP_DISABLE_THREADS
#include <mutex>
#include <thread>
#else
//! @cond INTERNAL
namespace std {
    //! @brief Single-threaded thread interface.
    struct thread {
        thread() = default;
        thread(thread&&) = default;
        thread(thread const&) = delete;
        template <typename F, typename... As>
        thread(F&& f, As&&... as) {
            f(as...);
        }
        inline void join() {}
        static constexpr unsigned int hardware_concurrency() {
            return 1;
        }
    };
    //! @brief Single-threaded mutex interface.
    struct mutex {
        mutex() = default;
        mutex(mutex const&) = delete;
        inline bool try_lock() {
            return true;
        }
        inline void lock() {}
        inline void unlock() {}
    };
    //! @brief Single-threaded lock guard interface.
    template <typename M>
    struct lock_guard {
        lock_guard(M const&) {}
        lock_guard(lock_guard const&) = delete;
    };
}
//! @endcond
#endif

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


//! @brief Namespace of tags for common use.
namespace tags {
    //! @brief Tag for sequential execution policy.
    struct sequential_execution {
        //! @brief Constructor.
        explicit sequential_execution(size_t = 1) {}
    };

    //! @brief Tag for parallel execution policy (with a given number of threads).
    struct parallel_execution {
        //! @brief Constructor.
        explicit parallel_execution(size_t n = std::thread::hardware_concurrency()) : num(n) {}
        //! @brief Parallel threads number.
        size_t num;
    };

    //! @brief Tag for general execution policies depending on the template parameter.
    template <bool parallel>
    using general_execution = std::conditional_t<parallel, parallel_execution, sequential_execution>;

    //! @brief Tag for parallel execution policy, assigning tasks dynamically (with a given number of threads and chunk size).
    struct dynamic_execution {
        //! @brief Constructor.
        explicit dynamic_execution(size_t n = std::thread::hardware_concurrency(), size_t s = 1) : num(n), size(s) {}
        //! @brief Parallel threads number.
        size_t num;
        //! @brief Chunk size.
        size_t size;
    };

    //! @brief Tag for distributed execution policy, assigning tasks dynamically across and within nodes through MPI (with a given number of threads per node, chunk size, static-dynamic percentage, and whether tasks should be shuffled).
    struct distributed_execution {
        //! @brief Constructor.
        explicit distributed_execution(size_t num = std::thread::hardware_concurrency(), size_t size = 1, double dynamic = 1, bool shuffle = false) : num(num), size(size), dynamic(dynamic), shuffle(shuffle) {}
        //! @brief Parallel threads number.
        size_t num;
        //! @brief Size of chunks dynamically assigned to nodes.
        size_t size;
        //! @brief Fraction of tasks to be assigned dynamically across nodes.
        double dynamic;
        //! @brief Whether tasks should be shuffled.
        bool shuffle;
    };
}


//! @cond INTERNAL
namespace details {
    //! @brief Compares two elements with the standard comparator.
    template <typename T>
    bool std_compare (T x, T y) {
        return x < y;
    }

    //! @brief The median between `a`, `b` and `c` is put into `first` (copied from gcc).
    template <typename E, typename C>
    void median_to_first(E first, E a, E b, E c, C comp) {
        if (comp(*a, *b)) {
            if (comp(*b, *c))
                std::iter_swap(first, b);
            else if (comp(*a, *c))
                std::iter_swap(first, c);
            else
                std::iter_swap(first, a);
        }
        else if (comp(*a, *c))
            std::iter_swap(first, a);
        else if (comp(*b, *c))
            std::iter_swap(first, c);
        else
            std::iter_swap(first, b);
    }

    //! @brief Partitions the interval between `first` and `last` around the `pivot` (copied from gcc).
    template <typename E, typename C>
    E pivot_partition(E first, E last, E pivot, C comp)
    {
        while (true) {
            while (comp(*first, *pivot))
                ++first;
            --last;
            while (comp(*pivot, *last))
                --last;
            if (!(first < last))
                return first;
            std::iter_swap(first, last);
            ++first;
        }
    }

    /**
     * @brief Internal function for nth_elements. Adds two parameters:
     *
     * @param offs   An offset to be applied to all indexes.
     * @param bound  A recursion bound before switching to bounded methods.
     */
    template <typename E, typename I, typename C>
    void nth_elements(E efirst, E elast, I ifirst, I ilast, C comp, size_t offs, size_t bound) {
        using std::swap;
        // no indexes
        if (ifirst == ilast) return;
        // one index
        if (ilast-ifirst == 1) {
            int n = *ifirst-offs;
            // is the first: just a min will do
            if (n == 0) {
                E emin = std::min_element(efirst, elast, comp);
                swap(*efirst, *emin);
                return;
            }
            // is the last: just a max will do
            if (n+1 == elast-efirst) {
                E emax = std::max_element(efirst, elast, comp);
                swap(*(elast-1), *emax);
                return;
            }
            // just call nth_element from the library
            nth_element(efirst, efirst+n, elast, comp);
            return;
        }
        // many indices, few elements
        if (8*(ilast-ifirst) > elast-efirst) {
            std::sort(efirst, elast);
            return;
        }
        // quick-select partitioning around
        while (ilast-ifirst > 1 and 8*(ilast-ifirst) <= elast-efirst) {
            if (bound == 0) {
                // being slow, changing strategy
                while (ilast-ifirst > 1 and 8*(ilast-ifirst) <= elast-efirst) {
                    I imid = ifirst + (ilast-ifirst)/2;
                    E emid = efirst + (*imid-offs);
                    nth_element (efirst, emid,  elast,  comp);
                    nth_elements(emid+1, elast, imid+1, ilast, comp, *imid+1, 0);
                    elast = emid;
                    ilast = imid;
                }
                break;
            }
            E emid = efirst + (elast-efirst)/2;
            median_to_first(efirst, efirst+1, emid, elast-1, comp);
            emid = pivot_partition(efirst+1, elast, efirst, comp);
            I imid = std::lower_bound(ifirst, ilast, (emid-efirst)+offs);
            --bound;
            nth_elements(emid, elast, imid, ilast, comp, (emid-efirst)+offs, bound);
            elast = emid;
            ilast = imid;
        }
        nth_elements(efirst, elast, ifirst, ilast, comp, offs, 0);
    }
}
//! @endcond INTERNAL


/**
 * @brief Sort a sequence just enough to fix a set of particular positions using a predicate for comparison.
 * Assumes that the index list is sorted in increasing order, and all indexes are within range 0 .. (elast-efirst-1).
 *
 * @param efirst Random-access iterator to the first element.
 * @param elast  Random-access iterator after the last element.
 * @param ifirst Random-access iterator to the first index.
 * @param ilast  Random-access iterator after the last index.
 * @param comp   A comparison functor.
 */
template <typename E, typename I, typename C>
inline void nth_elements(E efirst, E elast, I ifirst, I ilast, C comp) {
    size_t log = 2;
    while ((1<<log) < (elast-efirst)) ++log;
    details::nth_elements(efirst, elast, ifirst, ilast, comp, 0, 2*log);
}


/**
 * @brief Sort a sequence just enough to fix a set of particular positions.
 * Assumes that the index list is sorted in increasing order, and all indexes are within range 0 .. (elast-efirst-1).
 *
 * @param efirst Random-access iterator to the first element.
 * @param elast  Random-access iterator after the last element.
 * @param ifirst Random-access iterator to the first index.
 * @param ilast  Random-access iterator after the last index.
 */
template <typename E, typename I>
inline void nth_elements(E efirst, E elast, I ifirst, I ilast) {
    nth_elements(efirst, elast, ifirst, ilast, details::std_compare<typename std::iterator_traits<E>::value_type>);
}


/**
 * @brief Bypassable parallel for (sequential version).
 *
 * Executes a function (with index and thread number as arguments) for indices up to `len`.
 * The thread number is guaranteed to be 0 in this version.
 *
 * @param len The maximum index fed to the function.
 * @param f   The function `void(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_for(tags::sequential_execution, size_t len, F&& f) {
    for (size_t i=0; i<len; ++i) f(i,0);
}


#if defined(_OPENMP)
/**
 * @brief Bypassable parallel for (parallel OpenMP version).
 *
 * Executes a function (with index and thread number as arguments) for indices up to `len`.
 * The thread numbers range from zero to `omp_get_num_threads()-1`.
 *
 * @param e   The policy determining the number of threads to be spawned.
 * @param len The maximum index fed to the function.
 * @param f   The function `void(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_for(tags::parallel_execution e, size_t len, F&& f) {
    #pragma omp parallel for num_threads(e.num)
    for (size_t i=0; i<len; ++i) f(i,omp_get_thread_num());
}
#else
/**
 * @brief Bypassable parallel for (standard parallel version).
 *
 * Executes a function (with index and thread number as arguments) for indices up to `len`.
 * The thread numbers range from zero to `n-1`.
 *
 * @param e   The policy determining the number of threads to be spawned.
 * @param len The maximum index fed to the function.
 * @param f   The function `void(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_for(tags::parallel_execution e, size_t len, F&& f) {
    if (e.num == 1) {
        parallel_for(tags::sequential_execution{}, len, f);
        return;
    }
    std::vector<std::thread> pool;
    pool.reserve(e.num);
    for (size_t t=0; t<std::min(e.num,len); ++t)
        pool.emplace_back([=,&f] () {
            for (size_t i=t; i<len; i+=e.num) f(i,t);
        });
    for (std::thread& t : pool) t.join();
}
#endif


#if defined(_OPENMP)
/**
 * @brief Bypassable parallel for (parallel OpenMP version with dynamic scheduling).
 *
 * Executes a function (with index and thread number as arguments) for indices up to `len`.
 * The thread numbers range from zero to `omp_get_num_threads()-1`.
 *
 * @param e   The policy determining the number of threads to be spawned and chunk size.
 * @param len The maximum index fed to the function.
 * @param f   The function `void(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_for(tags::dynamic_execution e, size_t len, F&& f) {
    #pragma omp parallel for num_threads(e.num) schedule(dynamic, e.size)
    for (size_t i=0; i<len; ++i) f(i,omp_get_thread_num());
}
#else
/**
 * @brief Bypassable parallel for (standard parallel version with dynamic scheduling).
 *
 * Executes a function (with index and thread number as arguments) for indices up to `len`.
 * The thread numbers range from zero to `n-1`.
 *
 * @param e   The policy determining the number of threads to be spawned and chunk size.
 * @param len The maximum index fed to the function.
 * @param f   The function `void(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_for(tags::dynamic_execution e, size_t len, F&& f) {
    if (e.num == 1) {
        parallel_for(tags::sequential_execution{}, len, f);
        return;
    }
    std::vector<std::thread> pool;
    pool.reserve(e.num);
    std::mutex m;
    size_t i=0;
    for (size_t t=0; t<std::min(e.num,len); ++t)
        pool.emplace_back([=,&i,&f,&m] () {
            size_t j;
            while (true) {
                m.lock();
                j = i;
                i += e.size;
                m.unlock();
                if (j >= len) break;
                for (size_t k=j; k<j+e.size and k<len; ++k) f(k,t);
            }
        });
    for (std::thread& t : pool) t.join();
}
#endif


/**
 * @brief Bypassable parallel while (sequential version).
 *
 * Executes a function (with index and thread number as argument) until it returns `false`.
 * The thread number is guaranteed to be 0 in this version.
 *
 * @param f The function `bool(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_while(tags::sequential_execution, F&& f) {
    for (size_t i=0; f(i,0); ++i);
}


#if defined(_OPENMP)
/**
 * @brief Bypassable parallel while (parallel OpenMP version).
 *
 * Executes a function (with index and thread number as argument) until it returns `false`.
 * The thread numbers range from zero to `omp_get_num_threads()-1`.
 *
 * @param e The policy determining the number of threads to be spawned.
 * @param f The function `bool(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_while(tags::parallel_execution e, F&& f) {
    #pragma omp parallel num_threads(e.num)
    for (size_t i=omp_get_thread_num(); f(i, omp_get_thread_num()); i+=e.num);
}
#else
/**
 * @brief Bypassable parallel for (standard parallel version).
 *
 * Executes a function (with index and thread number as argument) until it returns `false`.
 * The thread numbers range from zero to `n-1`.
 *
 * @param e The policy determining the number of threads to be spawned.
 * @param f The function `bool(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_while(tags::parallel_execution e, F&& f) {
    if (e.num == 1) {
        parallel_while(tags::sequential_execution{}, f);
        return;
    }
    std::vector<std::thread> pool;
    pool.reserve(e.num);
    for (size_t t=0; t<e.num; ++t)
        pool.emplace_back([=,&f] () {
            for (size_t i=t; f(i,t); i+=e.num);
        });
    for (std::thread& t : pool) t.join();
}
#endif


#if defined(_OPENMP)
/**
 * @brief Bypassable parallel while (parallel OpenMP version with dynamic scheduling).
 *
 * Executes a function (with index and thread number as argument) until it returns `false`.
 * The thread numbers range from zero to `omp_get_num_threads()-1`.
 *
 * @param e The policy determining the number of threads to be spawned and chunk size.
 * @param f The function `bool(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_while(tags::dynamic_execution e, F&& f) {
    #pragma omp parallel num_threads(e.num) schedule(dynamic, e.size)
    for (size_t i=omp_get_thread_num(); f(i, omp_get_thread_num()); i+=e.num);
}
#else
/**
 * @brief Bypassable parallel for (standard parallel version with dynamic scheduling).
 *
 * Executes a function (with index and thread number as argument) until it returns `false`.
 * The thread numbers range from zero to `n-1`.
 *
 * @param e The policy determining the number of threads to be spawned and chunk size.
 * @param f The function `bool(size_t,size_t)` to be executed.
 */
template <typename F>
void parallel_while(tags::dynamic_execution e, F&& f) {
    if (e.num == 1) {
        parallel_while(tags::sequential_execution{}, f);
        return;
    }
    std::vector<std::thread> pool;
    pool.reserve(e.num);
    std::mutex m;
    size_t i=0;
    for (size_t t=0; t<e.num; ++t)
        pool.emplace_back([=,&i,&f,&m] () {
            size_t j;
            while (true) {
                m.lock();
                j = i;
                i += e.size;
                m.unlock();
                for (size_t k=j; k<j+e.size; ++k)
                    if (not f(k,t)) return;
            }
        });
    for (std::thread& t : pool) t.join();
}
#endif


//! @brief Uniform interface for inserting elements into a vector.
template <typename T>
void uniform_insert(std::vector<T>& container, T value) {
    container.push_back(value);
}

//! @brief Uniform interface for inserting elements into a set.
template <typename T>
void uniform_insert(std::set<T>& container, T value) {
    container.insert(value);
}

//! @brief Uniform interface for inserting elements into a multiset.
template <typename T>
void uniform_insert(std::multiset<T>& container, T value) {
    container.insert(value);
}

//! @brief Uniform interface for inserting elements into an unordered set.
template <typename T>
void uniform_insert(std::unordered_set<T>& container, T value) {
    container.insert(value);
}

//! @brief Uniform interface for inserting elements into an unordered multiset.
template <typename T>
void uniform_insert(std::unordered_multiset<T>& container, T value) {
    container.insert(value);
}


}


}

#endif // FCPP_COMMON_ALGORITHM_H_
