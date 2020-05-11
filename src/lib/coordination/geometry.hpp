// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file geometry.hpp
 * @brief Collection of field calculus utility functions.
 */

#ifndef FCPP_COORDINATION_GEOMETRY_H_
#define FCPP_COORDINATION_GEOMETRY_H_

#include <algorithm>

#include "lib/common/array.hpp"
#include "lib/coordination/utils.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Follows a target with a fixed speed, returning the distance from it.
template <typename node_t, size_t n>
double follow_target(node_t& node, trace_t call_point, const std::array<double, n>& target, double max_v, double period) {
    std::array<double, n> delta = target - node.position();
    double dist = std::norm(delta);
    max_v = std::min(max_v, dist / period);
    node.velocity() = delta * (max_v / dist);
    return dist;
}


//! @cond INTERNAL
//! @brief Generates a random target in a rectangle, given a sequence of indices.
template <typename node_t, size_t n, size_t... is>
std::array<double, n> random_rectangle_target(node_t& node, trace_t call_point, const std::array<double, n>& low, const std::array<double, n>& hi, std::index_sequence<is...>) {
    return {node.next_double(low[is], hi[is])...};
}
//! @endcond

//! @brief Generates a random target in a rectangle.
template <typename node_t, size_t n>
inline std::array<double, n> random_rectangle_target(node_t& node, trace_t call_point, const std::array<double, n>& low, const std::array<double, n>& hi) {
    return random_rectangle_target(node, call_point, low, hi, std::make_index_sequence<n>{});
}

//! @cond INTERNAL
//! @brief Generates a random target within a maximum (rectangular) reach in a rectangle, given a sequence of indices.
template <typename node_t, size_t n, size_t... is>
std::array<double, n> random_rectangle_target(node_t& node, trace_t call_point, const std::array<double, n>& low, const std::array<double, n>& hi, std::index_sequence<is...>, double reach) {
    return {node.next_double(std::max(low[is], node.position()[is]-reach),
                             std::min(hi[is],  node.position()[is]+reach))...};
}
//! @endcond

//! @brief Generates a random target within a maximum (rectangular) reach in a rectangle.
template <typename node_t, size_t n>
inline std::array<double, n> random_rectangle_target(node_t& node, trace_t call_point, const std::array<double, n>& low, const std::array<double, n>& hi, double reach) {
    return random_rectangle_target(node, call_point, low, hi, reach, std::make_index_sequence<n>{});
}


//! @brief Walks randomly in a rectangle at a fixed speed.
template <typename node_t, size_t n>
inline auto rectangle_walk(node_t& node, trace_t call_point, const std::array<double, n>& low, const std::array<double, n>& hi, double max_v, double period) {
    std::array<double, n> target = random_rectangle_target(node, call_point, low, hi);
    return old(node, call_point, target, [&](std::array<double, n> t){
        double dist = follow_target(node, call_point, t, max_v, period);
        return dist > max_v * period ? t : target;
    });
}

//! @brief Walks randomly within a maximum (rectangular) reach in a rectangle at a fixed speed.
template <typename node_t, size_t n>
inline auto rectangle_walk(node_t& node, trace_t call_point, const std::array<double, n>& low, const std::array<double, n>& hi, double reach, double max_v, double period) {
    std::array<double, n> target = random_rectangle_target(node, call_point, low, hi, reach);
    return old(node, call_point, target, [&](std::array<double, n> t){
        double dist = follow_target(node, call_point, t, max_v, period);
        return dist > max_v * period ? t : target;
    });
}


}


}

#endif // FCPP_COORDINATION_GEOMETRY_H_
