// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file algorithm.hpp
 * @brief Implementation of common algorithms useful in the library.
 */

#ifndef FCPP_COMMON_ALGORITHM_H_
#define FCPP_COMMON_ALGORITHM_H_

#include <algorithm>
#include <iterator>
#include <thread>
#include <type_traits>
#include <vector>


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
        sequential_execution() = default;
        sequential_execution(size_t) {}
    };

    //! @brief Tag for parallel execution policy (with a given number of threads).
    struct parallel_execution {
        parallel_execution() = default;
        parallel_execution(size_t n) : num(n) {}
        size_t num;
    };

    //! @brief Tag for general execution policies depending on the template parameter.
    template <bool parallel>
    using general_execution = std::conditional_t<parallel, parallel_execution, sequential_execution>;
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
        // no indexes
        if (ifirst == ilast) return;
        // one index
        if (ilast-ifirst == 1) {
            int n = *ifirst-offs;
            // is the first: just a min will do
            if (n == 0) {
                E emin = std::min_element(efirst, elast, comp);
                std::swap(*efirst, *emin);
                return;
            }
            // is the last: just a max will do
            if (n+1 == elast-efirst) {
                E emax = std::max_element(efirst, elast, comp);
                std::swap(*(elast-1), *emax);
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
        parallel_for(tags::sequential_execution(), len, f);
        return;
    }
    std::vector<std::thread> pool;
    pool.reserve(e.num);
    for (size_t t=0; t<e.num; ++t)
        pool.emplace_back([=,&f] () {
            for (size_t i=t; i<len; i+=e.num) f(i,t);
        });
    for (std::thread& t : pool) t.join();
}
#endif


/**
 * @brief Bypassable parallel while (sequential version).
 *
 * Executes a function (with the thread number as argument) until it returns `false`.
 * The thread number is guaranteed to be 0 in this version.
 *
 * @param f The function `bool(size_t)` to be executed.
 */
template <typename F>
void parallel_while(tags::sequential_execution, F&& f) {
    while (f(0));
}


#if defined(_OPENMP)
/**
 * @brief Bypassable parallel while (parallel OpenMP version).
 *
 * Executes a function (with the thread number as argument) until it returns `false`.
 * The thread numbers range from zero to `omp_get_num_threads()-1`.
 *
 * @param e The policy determining the number of threads to be spawned.
 * @param f The function `bool(size_t)` to be executed.
 */
template <typename F>
void parallel_while(tags::parallel_execution e, F&& f) {
    #pragma omp parallel num_threads(e.num)
    while (f(omp_get_thread_num()));
}
#else
/**
 * @brief Bypassable parallel for (standard parallel version).
 *
 * Executes a function (with the thread number as argument) until it returns `false`.
 * The thread numbers range from zero to `n-1`.
 *
 * @param e The policy determining the number of threads to be spawned.
 * @param f The function `bool(size_t)` to be executed.
 */
template <typename F>
void parallel_while(tags::parallel_execution e, F&& f) {
    if (e.num == 1) {
        parallel_while(tags::sequential_execution(), f);
        return;
    }
    std::vector<std::thread> pool;
    pool.reserve(e.num);
    for (size_t i=0; i<e.num; ++i)
        pool.emplace_back([&f,i] () {
            while (f(i));
        });
    for (std::thread& t : pool) t.join();
}
#endif


}


}

#endif // FCPP_COMMON_ALGORITHM_H_
