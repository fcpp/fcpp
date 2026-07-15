// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Mediates between an older and newer values.
inline real_t damper(real_t old_v, real_t new_v, real_t delta, real_t factor) {
    if (old_v > factor * new_v or new_v > factor * old_v)
        return new_v;
    real_t sign = old_v < new_v ? 0.5f : -0.5f;
    real_t diff = abs(new_v - old_v);
    return diff > delta ? new_v - delta * sign : old_v;
}


//! @brief Computes the hop-count distance from a source through adaptive bellmann-ford.
template <typename node_t>
hops_t abf_hops(node_t& node, trace_t call_point, bool source) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, std::numeric_limits<hops_t>::max(), [&] (field<hops_t> d) {
        hops_t nd = min_hood(node, 0, d, std::numeric_limits<hops_t>::max()-1) + 1;
        return source ? hops_t{0} : nd;
    });
}

//! @brief Export list for abf_hops.
using abf_hops_t = common::export_list<hops_t>;

//! @brief Computes the distance from a source with a custom metric through adaptive bellmann-ford.
template <typename node_t, typename G, typename = common::if_signature<G, field<real_t>()>>
real_t abf_distance(node_t& node, trace_t call_point, bool source, G&& metric) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, INF, [&] (field<real_t> d) {
        return min_hood(node, 0, d + metric(), source ? 0 : INF);
    });
}

//! @brief Computes the distance from a source through adaptive bellmann-ford.
template <typename node_t>
real_t abf_distance(node_t& node, trace_t call_point, bool source) {
    return abf_distance(node, call_point, source, [&](){
        return node.nbr_dist();
    });
}

//! @brief Export list for abf_distance.
using abf_distance_t = common::export_list<real_t>;


//! @brief Computes the distance from a source with a custom metric through bounded information speeds.
template <typename node_t, typename G, typename = common::if_signature<G, field<real_t>()>>
real_t bis_distance(node_t& node, trace_t call_point, bool source, times_t period, real_t speed, G&& metric) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    tuple<real_t,times_t> loc = source ? tuple<real_t,times_t>(0, 0) : make_tuple(INF, TIME_MAX);
    return get<0>(nbr(node, 0, loc, [&] (field<tuple<real_t,times_t>> x) {
        field<real_t> d = get<0>(x) + metric();
        field<times_t> t = get<1>(x) + node.nbr_lag();
        return min_hood(node, 0, make_tuple(max(d, (t-period)*speed), t), loc);
    }));
}

//! @brief Computes the distance from a source through bounded information speeds.
template <typename node_t>
inline real_t bis_distance(node_t& node, trace_t call_point, bool source, times_t period, real_t speed) {
    return bis_distance(node, call_point, source, period, speed, [&](){
        return node.nbr_dist();
    });
}

//! @brief Export list for bis_distance.
using bis_distance_t = common::export_list<tuple<real_t,times_t>>;


//! @brief Computes the distance from a source with a custom metric through flexible gradients.
template <typename node_t, typename G, typename = common::if_signature<G, field<real_t>()>>
real_t flex_distance(node_t& node, trace_t call_point, bool source, real_t epsilon, real_t radius, real_t distortion, int frequency, G&& metric) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    real_t loc = source ? 0 : INF;
    return get<0>(nbr(node, 0, make_tuple(loc, 0), [&] (field<tuple<real_t,int>> x) {
        field<real_t> dist = max(metric(), field<real_t>{distortion*radius});
        real_t old_d = get<0>(self(node, 0, x));
        int    old_c = get<1>(self(node, 0, x));
        real_t new_d = min_hood(node, 0, get<0>(x) + dist, loc);
        tuple<real_t,real_t,real_t> slopeinfo = max_hood(node, 0, make_tuple((old_d - get<0>(x))/dist, get<0>(x), dist), make_tuple(-INF, INF, 0));
        if (old_d == new_d or new_d == 0 or old_c == frequency or
            old_d > max(2*new_d, radius) or new_d > max(2*old_d, radius))
            return make_tuple(new_d, 0);
        if (get<0>(slopeinfo) > 1 + epsilon)
            return make_tuple(get<1>(slopeinfo) + get<2>(slopeinfo) * (1 + epsilon), old_c+1);
        if (get<0>(slopeinfo) < 1 - epsilon)
            return make_tuple(get<1>(slopeinfo) + get<2>(slopeinfo) * (1 - epsilon), old_c+1);
        return make_tuple(old_d, old_c+1);
    }));
}

//! @brief Computes the distance from a source through flexible gradients.
template <typename node_t>
inline real_t flex_distance(node_t& node, trace_t call_point, bool source, real_t epsilon, real_t radius, real_t distortion, int frequency) {
    return flex_distance(node, call_point, source, epsilon, radius, distortion, frequency, [&](){
        return node.nbr_dist();
    });
}

//! @brief Export list for flex_distance.
using flex_distance_t = common::export_list<tuple<real_t,int>>;


//! @brief Broadcasts a value following given distances from sources.
template <typename node_t, typename P, typename T>
T broadcast(node_t& node, trace_t call_point, P const& distance, T const& value) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, value, [&] (field<T> x) {
        return get<1>(min_hood(node, 0, make_tuple(nbr(node, 1, distance), x), make_tuple(distance, value)));
    });
}

//! @brief Broadcasts a value following given source markers and distances from sources.
template <typename node_t, typename P, typename T>
inline T broadcast(node_t& node, trace_t call_point, P const& distance, T const& value, bool source, T const& null) {
    return broadcast(node, call_point, distance, source ? value : null);
}

//! @brief Export list for broadcast.
template <typename P, typename T> using broadcast_t = common::export_list<P, T>;


//! @brief Broadcasts values from the `k` closest sources, calculating distances with a custom metric using BIS, storing the result in an unordered map.
template <typename node_t, typename T, typename G, typename = common::if_signature<G, field<real_t>()>, typename V = std::unordered_map<device_t, tuple<real_t,real_t,T>>>
V bis_ksource_broadcast(node_t& node, trace_t call_point, bool source, T value, int k, times_t period, real_t speed, G&& metric) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    auto map_merge = [&](V v1, V const& v2){
        for (auto const& p : v2) {
            if (not v1.count(p.first) or p.second < v1.at(p.first))
                v1[p.first] = p.second;
        }
        return v1;
    };
    auto map_add = [&](V v, real_t d, times_t t){
        for (auto& p : v) {
            get<1>(p.second) += t;
            get<0>(p.second) = max(get<0>(p.second)+d, (get<1>(p.second)-period)*speed);
        }
        return v;
    };

    V loc;
    if (source) loc[node.uid] = make_tuple(0, 0, value);
    return nbr(node, 0, loc, [&] (field<V> n) {
        mod_hood(map_add, n, metric(), node.nbr_lag());
        V res = fold_hood(node, 0, map_merge, n, loc);
        if (res.size() > k) {
            std::vector<std::pair<tuple<real_t,real_t,T>, device_t>> v;
            for (auto const& p : res)
                v.emplace_back(p.second, p.first);
            std::sort(v.begin(), v.end());
            res.clear();
            for (int i=0; i<k; ++i)
                res.emplace(v[i].second, v[i].first);
        }
        return res;
    });
}
//! @brief Broadcasts values from the `k` closest sources, calculating distances using BIS, storing the result in an unordered map.
template <typename node_t, typename T, typename V = std::unordered_map<device_t, tuple<real_t,real_t,T>>>
inline V bis_ksource_broadcast(node_t& node, trace_t call_point, bool source, T value, int k, times_t period, real_t speed) {
    return bis_ksource_broadcast(node, call_point, source, value, k, period, speed, [&](){
        return node.nbr_dist();
    });
}
//! @brief Export list for bis_ksource_broadcast.
template <typename T> using bis_ksource_broadcast_t = common::export_list<std::unordered_map<device_t, tuple<real_t,real_t,T>>>;


}


}

#endif // FCPP_COORDINATION_SPREADING_H_
