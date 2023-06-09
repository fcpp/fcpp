// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file collection.hpp
 * @brief Collection of field calculus data collection routines.
 */

#ifndef FCPP_COORDINATION_COLLECTION_H_
#define FCPP_COORDINATION_COLLECTION_H_

#include <cmath>
#include <limits>

#include "lib/coordination/utils.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Gossips distributed data with a given accumulation function.
template <typename node_t, typename T, typename G, typename = common::if_signature<G, T(T,T)>>
T gossip(node_t& node, trace_t call_point, T value, G&& accumulate) {
    return nbr(node, call_point, value, [&](field<T> x){
        return accumulate(fold_hood(node, call_point, accumulate, x), value);
    });
}

//! @brief Gossips distributed data by minimising.
template <typename node_t, typename T>
inline T gossip_min(node_t& node, trace_t call_point, T value) {
    return gossip(node, call_point, value, [](T x, T y){
        return std::min(x, y);
    });
}

//! @brief Gossips distributed data by maximising.
template <typename node_t, typename T>
inline T gossip_max(node_t& node, trace_t call_point, T value) {
    return gossip(node, call_point, value, [](T x, T y){
        return std::max(x, y);
    });
}

//! @brief Gossips distributed data by averaging.
template <typename node_t, typename T>
T gossip_mean(node_t& node, trace_t call_point, T value) {
    return nbr(node, call_point, value, [&](field<T> x){
        return mean_hood(node, call_point, x, value);
    });
}

//! @brief Export list for gossip.
template <typename T> using gossip_t = common::export_list<T>;

//! @brief Export list for gossip_min.
template <typename T> using gossip_min_t = gossip_t<T>;

//! @brief Export list for gossip_max.
template <typename T> using gossip_max_t = gossip_t<T>;

//! @brief Export list for gossip_mean.
template <typename T> using gossip_mean_t = gossip_t<T>;


//! @brief Collects distributed data with a single-path strategy.
template <typename node_t, typename P, typename T, typename U, typename G, typename = common::if_signature<G, T(T,T)>>
T sp_collection(node_t& node, trace_t call_point, P const& distance, T const& value, U const& null, G&& accumulate) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, (T)null, [&](field<T> x){
        device_t parent = get<1>(min_hood( node, 0, make_tuple(nbr(node, 1, distance), nbr_uid(node, 0)) ));
        return fold_hood(node, 0, accumulate, mux(nbr(node, 2, parent) == node.uid, x, (T)null), value);
    });
}

//! @brief Export list for sp_collection.
template <typename P, typename T, typename U = T> using sp_collection_t = common::export_list<T,P,device_t>;

//! @brief Collects distributed data with a multi-path strategy.
template <typename node_t, typename P, typename T, typename U, typename G, typename F, typename = common::if_signature<G, T(T,T)>, typename = common::if_signature<F, T(T,size_t)>>
T mp_collection(node_t& node, trace_t call_point, P const& distance, T const& value, U const& null, G&& accumulate, F&& divide) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, (T)null, [&](field<T> x){
        field<P> nbrdist = nbr(node, 1, distance);
        T v = fold_hood(node, 0, accumulate, mux(nbrdist > distance, x, (T)null), value);
        int n = sum_hood(node, 0, mux(nbrdist < distance, 1, 0), 0);
        return fcpp::make_tuple(divide(v, max(n, 1)), v);
    });
}

//! @brief Export list for mp_collection.
template <typename P, typename T, typename U = T> using mp_collection_t = common::export_list<T,P>;

//! @brief Collects distributed data with a weighted multi-path strategy.
template <typename node_t, typename T, typename G, typename F, typename = common::if_signature<G, T(T,T)>, typename = common::if_signature<F, T(T,real_t)>>
T wmp_collection(node_t& node, trace_t call_point, real_t distance, real_t radius, T const& value, G&& accumulate, F&& multiply) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    field<real_t> nbrdist = nbr(node, 0, distance);
    field<real_t> d = max(radius - node.nbr_dist(), field<real_t>{0});
    field<real_t> p = mux(isinf(distance) or isinf(nbrdist), field<real_t>{0},  distance - nbrdist);
    field<real_t> out_w = max(d * p, field<real_t>{0});
    real_t factor = sum_hood(node, 0, out_w, real_t{0});
    if (factor == 0) factor = 1;
    field<real_t> in_w = nbr(node, 0, out_w / factor);
    return nbr(node, 1, value, [&](field<T> x){
        return fold_hood(node, 0, accumulate, map_hood(multiply, x, in_w), value);
    });
}

//! @brief Export list for wmp_collection.
template <typename T> using wmp_collection_t = common::export_list<T,field<real_t>,real_t>;


//! @brief Collects distributed data with a lossless information-speed-threshold strategy and a idempotent accumulate function.
template <typename node_t, typename T, typename G, typename = common::if_signature<G, T(T,T)>>
T list_idem_collection(node_t& node, trace_t call_point, real_t const& distance, T const& value, real_t radius, real_t speed, T const& null, real_t epsilon, G&& accumulate) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    field<real_t> nbrdist = nbr(node, 0, distance);
    real_t t = node.current_time();
    field<real_t> Tu = nbr(node, 1, node.next_time() + epsilon);
    field<real_t> Pu = nbr(node, 2, distance + speed * (node.next_time() - t));
    field<real_t> maxDistNow = node.nbr_dist() + speed * node.nbr_lag();
    field<real_t> Vwst = mux(isfinite(distance) and maxDistNow < radius, (distance - Pu) / (Tu - t), (real_t)(-INF));
    field<real_t> nbrThreshold = nbr(node, 4, max_hood(node, 0, Vwst, 0));

    return nbr(node, 3, value, [&](field<T> old){
        field<T> nv = mux(isfinite(distance) and nbrdist >= distance + node.nbr_lag() * nbrThreshold, old, null);
        return fold_hood(node, 0, accumulate, nv, value);
    });
}

//! @brief Export list for list_idem_collection.
template <typename T> using list_idem_collection_t = common::export_list<T,real_t>;


//! @brief Collects distributed data with a arithmetic strategy.
template <typename node_t, typename T, typename G, typename = common::if_signature<G, T(T,T)>>
T list_arith_collection(node_t& node, trace_t call_point, real_t const& distance, T const& value, real_t radius, real_t speed, T const& null, real_t epsilon, G&& accumulate) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    real_t t = node.current_time();
    field<real_t> Tu = nbr(node, 1, node.next_time() + epsilon);
    field<real_t> Pu = nbr(node, 2, distance + speed * (node.next_time() - t));
    field<real_t> maxDistNow = node.nbr_dist() + speed * node.nbr_lag();
    field<real_t> Vwst = mux(isfinite(distance) and maxDistNow < radius, (distance - Pu) / (Tu - t), (real_t)(-INF));

    return old(node, 6, value, [&](T x){

        tuple<field<real_t>, unsigned int, T> data = make_tuple(Vwst,node.uid,x);
        tuple<field<real_t>, unsigned int, T> parentData = max_hood(node, 0, nbr(node,4,data));

        device_t parent = get<1>(parentData);
        field<T> sum = mux(nbr_uid(node, 0) == parent, get<2>(parentData), null);
        T v = fold_hood(node, 0, accumulate, sum, value);

        return v;

    });
}

//! @brief Export list for list_arith_collection.
template <typename T> using list_arith_collection_t = common::export_list<T,real_t,tuple<field<double>, unsigned int, int>>;


}


}

#endif // FCPP_COORDINATION_COLLECTION_H_
