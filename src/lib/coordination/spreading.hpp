// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file spreading.hpp
 * @brief Collection of field calculus distance estimation routines.
 */

#ifndef FCPP_COORDINATION_SPREADING_H_
#define FCPP_COORDINATION_SPREADING_H_

#include <cmath>

#include <algorithm>
#include <limits>

#include "lib/coordination/utils.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Mediates between an older and newer values.
double damper(double old_v, double new_v, double delta, double factor) {
    if (old_v > factor * new_v or new_v > factor * old_v)
        return new_v;
    double sign = old_v < new_v ? 0.5 : -0.5;
    double diff = abs(new_v - old_v);
    return diff > delta ? new_v - delta * sign : old_v;
}


//! @brief Computes the hop-count distance from a source through adaptive bellmann-ford.
template <typename node_t>
int abf_hops(node_t& node, trace_t call_point, bool source) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, std::numeric_limits<int>::max()-1, [&] (field<int> d) {
        return min_hood(node, 0, d + 1, source ? 0 : std::numeric_limits<int>::max()-1);
    });
}

//! @brief Computes the distance from a source through adaptive bellmann-ford.
template <typename node_t>
double abf_distance(node_t& node, trace_t call_point, bool source) {
    return abf_distance(node, call_point, source, [&](){
        return node.nbr_dist();
    });
}

//! @brief Computes the distance from a source with a custom metric through adaptive bellmann-ford.
template <typename node_t, typename G, typename = common::if_signature<G, field<double>()>>
double abf_distance(node_t& node, trace_t call_point, bool source, G&& metric) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, INF, [&] (field<double> d) {
        return min_hood(node, 0, d + metric(), source ? 0.0 : INF);
    });
}


//! @brief Computes the distance from a source through bounded information speeds.
template <typename node_t>
double bis_distance(node_t& node, trace_t call_point, bool source, double period, double speed) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    tuple<double,double> loc = source ? make_tuple(0.0, 0.0) : make_tuple(INF, INF);
    return get<0>(nbr(node, 0, loc, [&] (field<tuple<double,double>> x) {
        field<double> d = get<0>(x) + node.nbr_dist();
        field<double> t = get<1>(x) + node.nbr_lag();
        return min_hood(node, 0, make_tuple(max(d, (t-period)*speed), t), loc);
    }));
}

//! @brief Computes the distance from a source with a custom metric through bounded information speeds.
template <typename node_t, typename G, typename = common::if_signature<G, field<double>()>>
double bis_distance(node_t& node, trace_t call_point, bool source, double period, double speed, G&& metric) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    tuple<double,double> loc = source ? make_tuple(0.0, 0.0) : make_tuple(INF, INF);
    return get<0>(nbr(node, 0, loc, [&] (field<tuple<double,double>> x) {
        field<double> d = get<0>(x) + metric();
        field<double> t = get<1>(x) + node.nbr_lag();
        return min_hood(node, 0, make_tuple(max(d, (t-period)*speed), t), loc);
    }));
}


//! @brief Computes the distance from a source through flexible gradients.
template <typename node_t>
inline double flex_distance(node_t& node, trace_t call_point, bool source, double epsilon, double radius, double distortion, int frequency) {
    return flex_distance(node, call_point, source, epsilon, radius, distortion, frequency, [&](){
        return node.nbr_dist();
    });
}

//! @brief Computes the distance from a source with a custom metric through flexible gradients.
template <typename node_t, typename G, typename = common::if_signature<G, field<double>()>>
double flex_distance(node_t& node, trace_t call_point, bool source, double epsilon, double radius, double distortion, int frequency, G&& metric) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    double loc = source ? 0.0 : INF;
    return get<0>(nbr(node, 0, make_tuple(loc, 0), [&] (field<tuple<double,int>> x) {
        field<double> dist = max(metric(), field<double>{distortion*radius});
        double old_d = get<0>(self(node, 0, x));
        int    old_c = get<1>(self(node, 0, x));
        double new_d = min_hood(node, 0, get<0>(x) + dist, loc);
        tuple<double,double,double> slopeinfo = max_hood(node, 0, make_tuple((old_d - get<0>(x))/dist, get<0>(x), dist), make_tuple(-INF, INF, 0));
        if (old_d == new_d or new_d == 0.0 or old_c == frequency or
            old_d > max(2*new_d, radius) or new_d > max(2*old_d, radius))
            return make_tuple(new_d, 0);
        if (get<0>(slopeinfo) > 1 + epsilon)
            return make_tuple(get<1>(slopeinfo) + get<2>(slopeinfo) * (1 + epsilon), old_c+1);
        if (get<0>(slopeinfo) < 1 - epsilon)
            return make_tuple(get<1>(slopeinfo) + get<2>(slopeinfo) * (1 - epsilon), old_c+1);
        return make_tuple(old_d, old_c+1);
    }));
}


//! @brief Broadcasts a value following given distances from sources.
template <typename node_t, typename P, typename T>
T broadcast(node_t& node, trace_t call_point, const P& distance, const T& value) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, value, [&] (field<T> x) {
        return get<1>(min_hood(node, 0, make_tuple(nbr(node, 1, distance), x), make_tuple(distance, value)));
    });
}

//! @brief Broadcasts a value following given source markers and distances from sources.
template <typename node_t, typename P, typename T>
inline T broadcast(node_t& node, trace_t call_point, const P& distance, const T& value, bool source, const T& null) {
    return broadcast(node, call_point, distance, source ? value : null);
}


}


}

#endif // FCPP_COORDINATION_SPREADING_H_
