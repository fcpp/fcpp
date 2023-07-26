// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file filter.hpp
 * @brief Composable filter predicate classes on values.
 */

#ifndef FCPP_OPTION_FILTER_H_
#define FCPP_OPTION_FILTER_H_

#include <cmath>
#include <limits>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for filtering functions.
namespace filter {


//! @brief Filters finite values.
struct finite {
    //! @brief Filter check.
    template <typename V>
    bool operator()(V v) const {
        return std::isfinite(v);
    }
};

//! @brief Filters values within `L/den` and `U/den` (included).
template <intmax_t L, intmax_t U, intmax_t den = 1>
struct within {
    //! @brief Filter check.
    template <typename V>
    bool operator()(V v) const {
        v *= den;
        if (L > std::numeric_limits<intmax_t>::min() and L > v) return false;
        if (U < std::numeric_limits<intmax_t>::max() and U < v) return false;
        return true;
    }
};

//! @brief Filters values above `L/den` (included).
template <intmax_t L, intmax_t den = 1>
using above = within<L, std::numeric_limits<intmax_t>::max(), den>;

//! @brief Filters values below `U/den` (included).
template <intmax_t U, intmax_t den = 1>
using below = within<std::numeric_limits<intmax_t>::min(), U, den>;

//! @brief Filters values equal to `V/den`.
template <intmax_t V, intmax_t den = 1>
using equal = within<V, V, den>;

//! @brief Negate a filter.
template <typename F>
struct neg : F {
    //! @brief Filter check.
    template <typename V>
    bool operator()(V v) const {
        return not F::operator()(v);
    }
};

//! @brief Joins filters (or).
template <typename F, typename G>
struct vee : F, G {
    //! @brief Filter check.
    template <typename V>
    bool operator()(V v) const {
        return F::operator()(v) or G::operator()(v);
    }
};

//! @brief Disjoins filters (and).
template <typename F, typename G>
struct wedge : F, G {
    //! @brief Filter check.
    template <typename V>
    bool operator()(V v) const {
        return F::operator()(v) and G::operator()(v);
    }
};


} // filter


} // fcpp

#endif // FCPP_OPTION_FILTER_H_
