// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file time.hpp
 * @brief Collection of field calculus time evolution functions.
 */

#ifndef FCPP_COORDINATION_TIME_H_
#define FCPP_COORDINATION_TIME_H_

#include <cmath>

#include <algorithm>
#include <limits>

#include "lib/data/field.hpp"
#include "lib/internal/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief A counter increasing by a given amount at every round, starting from a given amount.
template <typename node_t, typename A, typename B>
inline B counter(node_t& node, trace_t call_point, A&& a, B&& b) {
    return old(node, call_point, b, [&](B x) -> B{
        return x+a;
    });
}

//! @brief A counter increasing by a given amount at every round.
template <typename node_t, typename A>
inline A counter(node_t& node, trace_t call_point, A&& a) {
    return counter(node, call_point, a, A{});
}

//! @brief A counter increasing by one at every round.
template <typename node_t>
inline int counter(node_t& node, trace_t call_point) {
    return counter(node, call_point, 1, 0);
}


}


}

#endif // FCPP_COORDINATION_TIME_H_
