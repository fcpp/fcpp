// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file filter.hpp
 * @brief Composable filter predicate classes on values.
 */

#ifndef FCPP_OPTION_FILTER_H_
#define FCPP_OPTION_FILTER_H_

#include <cmath>
#include <cstdint>
#include <limits>
#include <string>


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

    //! @brief Filter representation.
    static std::string name() {
        return "finite";
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

    //! @brief Filter representation.
    static std::string name() {
        return "in [" + std::to_string(L/double(den)) + ".." + std::to_string(U/double(den)) + "]";
    }
};

//! @brief Filters values above `L/den` (included).
template <intmax_t L, intmax_t den = 1>
struct above : public within<L, std::numeric_limits<intmax_t>::max(), den> {
    //! @brief Filter representation.
    static std::string name() {
        return "above " + std::to_string(L/double(den));
    }
};

//! @brief Filters values below `U/den` (included).
template <intmax_t U, intmax_t den = 1>
struct below : public within<std::numeric_limits<intmax_t>::min(), U, den> {
    //! @brief Filter representation.
    static std::string name() {
        return "below " + std::to_string(U/double(den));
    }
};

//! @brief Filters values equal to `V/den`.
template <intmax_t V, intmax_t den = 1>
struct equal : public within<V, V, den> {
    //! @brief Filter representation.
    static std::string name() {
        return "equal to " + std::to_string(V/double(den));
    }
};

//! @brief Negate a filter.
template <typename F>
struct neg : F {
    //! @brief Filter check.
    template <typename V>
    bool operator()(V v) const {
        return not F::operator()(v);
    }

    //! @brief Filter representation.
    static std::string name() {
        return "not " + F::name();
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

    //! @brief Filter representation.
    static std::string name() {
        return F::name() + " or " + G::name();
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

    //! @brief Filter representation.
    static std::string name() {
        return F::name() + " and " + G::name();
    }
};


} // filter


} // fcpp

#endif // FCPP_OPTION_FILTER_H_
