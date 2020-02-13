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


//! @brief Tags for parallel (with a given number of threads) and sequential execution policies.
//! @{
struct sequential_execution_t {};
template <size_t n>
struct parallel_execution_t {};
constexpr sequential_execution_t sequential_execution {};
template <size_t n>
constexpr parallel_execution_t<n> parallel_execution {};
template <size_t n>
using general_execution_t = std::conditional_t<n >= 2, parallel_execution_t<n>, sequential_execution_t>;
template <size_t n>
constexpr general_execution_t<n> general_execution {};
//! @}


/**
 * @brief Bypassable parallel for (sequential version).
 *
 * @param v A vector of inputs.
 * @param f A void function to be applied on them.
 */
template <typename F>
void parallel_for(sequential_execution_t, size_t len, F&& f) {
    for (size_t i=0; i<len; ++i) f(i);
}


#if defined(_OPENMP)
/**
* @brief Bypassable parallel for (parallel version).
*
* @param v A vector of inputs.
* @param f A void function to be applied on them.
*/
template <size_t n, typename F>
void parallel_for(parallel_execution_t<n>, size_t len, F&& f) {
    #pragma omp parallel for num_threads(n)
    for (size_t i=0; i<len; ++i) f(i);
}
#else
/**
* @brief Bypassable parallel for (parallel version).
*
* @param v A vector of inputs.
* @param f A void function to be applied on them.
*/
template <size_t n, typename F>
void parallel_for(parallel_execution_t<n>, size_t len, F&& f) {
    size_t slice = std::max(len / n, (size_t)1);
    size_t threshold = (n - std::max(int(len - slice*n), 0))*slice;
    std::vector<std::thread> pool;
    pool.reserve(n);
    auto launch = [&f] (size_t a, size_t b) {
        for (size_t i=a; i<b; ++i) f(i);
    };
    for (size_t i=0; i!=len; i+=slice) {
        if (i >= threshold) ++slice;
        pool.emplace_back(launch, i, i+slice);
    }
    for (std::thread& t : pool) t.join();
}
#endif


/**
 * @brief Bypassable parallel for (sequential version).
 *
 * @param v A vector of inputs.
 * @param f A void function to be applied on them.
 */
template <typename F>
void parallel_while(sequential_execution_t, F&& f) {
    while (f());
}


#if defined(_OPENMP)
/**
* @brief Bypassable parallel for (parallel version).
*
* @param v A vector of inputs.
* @param f A void function to be applied on them.
*/
template <size_t n, typename F>
void parallel_while(parallel_execution_t<n>, F&& f) {
    #pragma omp parallel num_threads(n)
    while (f());
}
#else
/**
* @brief Bypassable parallel for (parallel version).
*
* @param v A vector of inputs.
* @param f A void function to be applied on them.
*/
template <size_t n, typename F>
void parallel_while(parallel_execution_t<n>, F&& f) {
    std::vector<std::thread> pool;
    pool.reserve(n);
    auto launch = [&f] () {
        while (f());
    };
    for (size_t i=0; i<n; ++i)
        pool.emplace_back(launch);
    for (std::thread& t : pool) t.join();
}
#endif


}

#endif // FCPP_COMMON_ALGORITHM_H_
