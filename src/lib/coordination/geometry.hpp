// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file geometry.hpp
 * @brief Collection of field calculus utility functions.
 */

#ifndef FCPP_COORDINATION_GEOMETRY_H_
#define FCPP_COORDINATION_GEOMETRY_H_

#include <algorithm>

#include "lib/coordination/utils.hpp"
#include "lib/data/vec.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Follows a target with a fixed speed, returning the distance from it.
template <typename node_t, size_t n>
real_t follow_target(node_t& node, trace_t, const vec<n>& target, real_t max_v, real_t period) {
    vec<n> delta = target - node.position();
    real_t dist = norm(delta);
    node.velocity() = delta * (dist < 1e-10 ? 0 : std::min(max_v, dist / period) / dist);
    return dist;
}

//! @brief Follows a target with a fixed acceleration and maximum speed, returning the distance from it.
template <typename node_t, size_t n>
real_t follow_target(node_t& node, trace_t, const vec<n>& target, real_t max_v, real_t max_a, real_t period) {
    node.friction() = max_a / max_v; // max_v is limit speed
    vec<n> delta = target - node.position();
    node.propulsion() = delta/period - node.velocity(); // best acceleration if no friction
    node.propulsion() *= std::min(max_a/norm(node.propulsion()), 2/period);
    return norm(delta);
}


//! @cond INTERNAL
//! @brief Generates a random target in a rectangle, given a sequence of indices.
template <typename node_t, size_t n, size_t... is>
vec<n> random_rectangle_target(node_t& node, const vec<n>& low, const vec<n>& hi, std::index_sequence<is...>) {
    return {node.next_real(low[is], hi[is])...};
}
//! @endcond

//! @brief Generates a random target in a rectangle.
template <typename node_t, size_t n>
inline vec<n> random_rectangle_target(node_t& node, trace_t, const vec<n>& low, const vec<n>& hi) {
    return random_rectangle_target(node, low, hi, std::make_index_sequence<n>{});
}

//! @cond INTERNAL
//! @brief Generates a random target within a maximum (rectangular) reach in a rectangle, given a sequence of indices.
template <typename node_t, size_t n, size_t... is>
vec<n> random_rectangle_target(node_t& node, const vec<n>& low, const vec<n>& hi, real_t reach, std::index_sequence<is...>) {
    return {node.next_real(std::max(low[is], node.position()[is]-reach),
                           std::min(hi[is],  node.position()[is]+reach))...};
}
//! @endcond

//! @brief Generates a random target within a maximum (rectangular) reach in a rectangle.
template <typename node_t, size_t n>
inline vec<n> random_rectangle_target(node_t& node, trace_t, const vec<n>& low, const vec<n>& hi, real_t reach) {
    return random_rectangle_target(node, low, hi, reach, std::make_index_sequence<n>{});
}


//! @brief Walks randomly in a rectangle at a fixed speed.
template <typename node_t, size_t n>
inline vec<n> rectangle_walk(node_t& node, trace_t call_point, const vec<n>& low, const vec<n>& hi, real_t max_v, real_t period) {
    vec<n> target = random_rectangle_target(node, call_point, low, hi);
    return old(node, call_point, target, [&](vec<n> t){
        real_t dist = follow_target(node, call_point, t, max_v, period);
        return dist > max_v * period ? t : target;
    });
}

//! @brief Walks randomly within a maximum (rectangular) reach in a rectangle at a fixed speed.
template <typename node_t, size_t n>
inline vec<n> rectangle_walk(node_t& node, trace_t call_point, const vec<n>& low, const vec<n>& hi, real_t reach, real_t max_v, real_t period) {
    vec<n> target = random_rectangle_target(node, call_point, low, hi, reach);
    return old(node, call_point, target, [&](vec<n> t){
        real_t dist = follow_target(node, call_point, t, max_v, period);
        return dist > max_v * period ? t : target;
    });
}


}


}

#endif // FCPP_COORDINATION_GEOMETRY_H_
