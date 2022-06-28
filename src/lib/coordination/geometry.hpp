// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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
real_t follow_target(node_t& node, trace_t, vec<n> const& target, real_t max_v, real_t period) {
    vec<n> delta = target - node.position();
    real_t dist = norm(delta);
    node.velocity() = delta * (dist < 1e-10 ? 0 : std::min(max_v, dist / period) / dist);
    return dist;
}

//! @brief Follows a target with a fixed acceleration and maximum speed, returning the distance from it.
template <typename node_t, size_t n>
real_t follow_target(node_t& node, trace_t, vec<n> const& target, real_t max_v, real_t max_a, real_t period) {
    node.friction() = max_a / max_v; // max_v is limit speed
    vec<n> delta = target - node.position();
    node.propulsion() = delta/period - node.velocity(); // best acceleration if no friction
    node.propulsion() *= std::min(max_a/norm(node.propulsion()), 2/period);
    return norm(delta);
}


//! @brief Follows a path with a fixed speed, returning the index of the next target and distance to it.
template <typename node_t, typename T>
tuple<size_t,real_t> follow_path(node_t& node, trace_t call_point, T const& path, real_t max_v, real_t period) {
    return old(node, call_point, size_t{0}, [&](size_t i) {
        real_t dist = follow_target(node, call_point, path[i], max_v, period);
        size_t ni = i < path.size() - 1 and dist < max_v * period ? i+1 : i;
        return make_tuple(make_tuple(i,dist), ni);
    });
}

//! @brief Follows a path with a fixed acceleration and speed, returning the index of the next target and distance to it.
template <typename node_t, typename T>
tuple<size_t,real_t> follow_path(node_t& node, trace_t call_point, T const& path, real_t max_v, real_t max_a, real_t period) {
    return old(node, call_point, size_t{0}, [&](size_t i) {
        real_t dist = follow_target(node, call_point, path[i], max_v, max_a, period);
        size_t ni = i < path.size() - 1 and dist < max_v * period ? i+1 : i;
        return make_tuple(make_tuple(ni,dist), ni);
    });
}

//! @brief Export list for follow_path.
using follow_path_t = common::export_list<size_t>;


//! @cond INTERNAL
//! @brief Generates a random target in a rectangle, given a sequence of indices.
template <typename node_t, size_t n, size_t... is>
vec<n> random_rectangle_target(node_t& node, vec<n> const& low, vec<n> const& hi, std::index_sequence<is...>) {
    return {node.next_real(low[is], hi[is])...};
}
//! @endcond

//! @brief Generates a random target in a rectangle.
template <typename node_t, size_t n>
inline vec<n> random_rectangle_target(node_t& node, trace_t, vec<n> const& low, vec<n> const& hi) {
    return random_rectangle_target(node, low, hi, std::make_index_sequence<n>{});
}

//! @cond INTERNAL
//! @brief Generates a random target within a maximum (rectangular) reach in a rectangle, given a sequence of indices.
template <typename node_t, size_t n, size_t... is>
vec<n> random_rectangle_target(node_t& node, vec<n> const& low, vec<n> const& hi, real_t reach, std::index_sequence<is...>) {
    return {node.next_real(std::max(low[is], node.position()[is]-reach),
                           std::min(hi[is],  node.position()[is]+reach))...};
}
//! @endcond

//! @brief Generates a random target within a maximum (rectangular) reach in a rectangle.
template <typename node_t, size_t n>
inline vec<n> random_rectangle_target(node_t& node, trace_t, vec<n> const& low, vec<n> const& hi, real_t reach) {
    return random_rectangle_target(node, low, hi, reach, std::make_index_sequence<n>{});
}


//! @brief Walks randomly in a rectangle at a fixed speed.
template <typename node_t, size_t n>
inline vec<n> rectangle_walk(node_t& node, trace_t call_point, vec<n> const& low, vec<n> const& hi, real_t max_v, real_t period) {
    vec<n> target = random_rectangle_target(node, call_point, low, hi);
    return old(node, call_point, target, [&](vec<n> t){
        real_t dist = follow_target(node, call_point, t, max_v, period);
        return dist > max_v * period ? t : target;
    });
}

//! @brief Walks randomly within a maximum (rectangular) reach in a rectangle at a fixed speed.
template <typename node_t, size_t n>
inline vec<n> rectangle_walk(node_t& node, trace_t call_point, vec<n> const& low, vec<n> const& hi, real_t reach, real_t max_v, real_t period) {
    vec<n> target = random_rectangle_target(node, call_point, low, hi, reach);
    return old(node, call_point, target, [&](vec<n> t){
        real_t dist = follow_target(node, call_point, t, max_v, period);
        return dist > max_v * period ? t : target;
    });
}

//! @brief Export list for rectangle_walk.
template <size_t n> using rectangle_walk_t = common::export_list<vec<n>>;


/**
 * @brief Computes the elastic force tying a node to a point.
 *
 * The result is designed to be set as node.propulsion().
 */
template <typename node_t, size_t n>
inline vec<n> point_elastic_force(node_t& node, trace_t, vec<n> const& point, real_t length, real_t strength) {
    vec<n> v = point - node.position();
    real_t d = norm(v);
    return v * ((1-length/d)*strength);
}

//! @brief Export list for point_elastic_force.
using point_elastic_force_t = common::export_list<>;

/**
 * @brief Computes the elastic force tying a node to a line p -- q.
 *
 * The result is designed to be set as node.propulsion().
 */
template <typename node_t, size_t n>
inline vec<n> line_elastic_force(node_t& node, trace_t, vec<n> const& p, vec<n> const& q, real_t length, real_t strength) {
    vec<n> l = q - p;
    vec<n> v = node.position() - p;
    v = ((v * l) / abs(l)) * l - v;
    real_t d = norm(v);
    return v * ((1-length/d)*strength);
}

//! @brief Export list for line_elastic_force.
using line_elastic_force_t = common::export_list<>;

/**
 * @brief Computes the elastic force tying a node to a plane in p with perpendicular q.
 *
 * The result is designed to be set as node.propulsion().
 */
template <typename node_t, size_t n>
inline vec<n> plane_elastic_force(node_t& node, trace_t, vec<n> const& p, vec<n> const& q, real_t length, real_t strength) {
    vec<n> v = (((p - node.position()) * q) / abs(q)) * q;
    real_t d = norm(v);
    return v * ((1-length/d)*strength);
}

//! @brief Export list for plane_elastic_force.
using plane_elastic_force_t = common::export_list<>;

/**
 * @brief Computes the total elastic forces tying a node to its neighbours.
 *
 * The length and strength arguments need to be convertible to `real_t` or `field<real_t>`.
 * The result is designed to be set as node.propulsion().
 */
template <typename node_t, typename A, typename B>
inline typename node_t::position_type neighbour_elastic_force(node_t& node, trace_t call_point, A const& length, B const& strength) {
    using vec_t = typename node_t::position_type;
    return sum_hood(node, call_point, map_hood([](vec_t v, real_t l, real_t s){
        real_t d = norm(v);
        return v * ((1-l/d)*s);
    }, node.nbr_vec(), length, strength), vec_t{});
}

//! @brief Export list for neighbour_elastic_force.
using neighbour_elastic_force_t = common::export_list<>;

/**
 * @brief Computes the gravitational force tying a node to a point.
 *
 * Can model a repulsive force if mass is negative.
 * The result is designed to be set as node.propulsion().
 */
template <typename node_t, size_t n>
inline vec<n> point_gravitational_force(node_t& node, trace_t, vec<n> const& point, real_t mass) {
    vec<n> v = point - node.position();
    real_t d = norm(v);
    return v * (mass / (d*d*d));
}

//! @brief Export list for point_gravitational_force.
using point_gravitational_force_t = common::export_list<>;

/**
 * @brief Computes the total gravitational force tying a node to its neighbours.
 *
 * Can model a repulsive force if mass is negative.
 * The result is designed to be set as node.propulsion().
 */
template <typename node_t>
inline typename node_t::position_type neighbour_gravitational_force(node_t& node, trace_t call_point, real_t mass) {
    using vec_t = typename node_t::position_type;
    return sum_hood(node, call_point, map_hood([](vec_t v, real_t m){
        real_t d = norm(v);
        return v * (m / (d*d*d));
    }, node.nbr_vec(), nbr(node, call_point, mass)), vec_t{});
}

//! @brief Export list for neighbour_gravitational_force.
using neighbour_gravitational_force_t = common::export_list<real_t>;

/**
 * @brief Computes the total charged force tying a node to its neighbours.
 *
 * Whether nodes are attracted or repulsed depends on the product of their charges.
 * The result is designed to be set as node.propulsion().
 */
template <typename node_t>
inline typename node_t::position_type neighbour_charged_force(node_t& node, trace_t call_point, real_t mass, real_t charge) {
    using vec_t = typename node_t::position_type;
    return sum_hood(node, call_point, map_hood([=](vec_t v, real_t c){
        real_t d = norm(v);
        return v * (-(c*charge) / (mass*d*d*d));
    }, node.nbr_vec(), nbr(node, call_point, charge)), vec_t{});
}

//! @brief Export list for neighbour_charged_force.
using neighbour_charged_force_t = common::export_list<real_t>;


}


}

#endif // FCPP_COORDINATION_GEOMETRY_H_
