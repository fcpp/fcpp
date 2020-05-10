// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file spreading.hpp
 * @brief Collection of field calculus distance estimation routines.
 */

#ifndef FCPP_COORDINATION_SPREADING_H_
#define FCPP_COORDINATION_SPREADING_H_

#include <algorithm>
#include <limits>

#include "lib/common/traits.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Reduces the values in the domain of a field to a single value by minimum.
template <typename node_t, typename A>
A min_hood(node_t& node, trace_t call_point, const field<A>& f) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    return fold_hood(node, 0, [] (A x, A y) {
        return std::min(x, y);
    }, f);
}

//! @brief Computes the distance from a source through adaptive bellmann-ford.
template <typename node_t, typename G, typename = common::if_signature<G, field<double>()>>
double distance(node_t& node, trace_t call_point, bool source, G&& metric) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, std::numeric_limits<double>::infinity(), [&] (fcpp::field<double> d) {
        double r = min_hood(node, 1, d + metric());
        return source ? 0.0 : r;
    });
}


}


}

#endif // FCPP_COORDINATION_SPREADING_H_
