// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file collection.hpp
 * @brief Collection of field calculus data collection routines.
 */

#ifndef FCPP_COORDINATION_COLLECTION_H_
#define FCPP_COORDINATION_COLLECTION_H_

#include <cmath>
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


//! @brief Collects distributed data with a single-path strategy.
template <typename node_t, typename P, typename T, typename G, typename = common::if_signature<G, T(T,T)>>
T sp_collection(node_t& node, trace_t call_point, const P& distance, const T& value, const T& null, G&& accumulate) {
    data::trace_call trace_caller(node.stack_trace, call_point);
    
    return nbr(node, 0, null, [&](field<T> x){
        device_t parent = get<1>(min_hood( node, 0, nbr(node, 1, make_tuple(distance, node.uid)) ));
        return fold_hood(node, 0, accumulate, mux(nbr(node, 2, parent) == node.uid, x, field<T>{null}), value);
    });
}

//! @brief Collects distributed data with a multi-path strategy.
template <typename node_t, typename P, typename T, typename G, typename F, typename = common::if_signature<G, T(T,T)>, typename = common::if_signature<F, T(T,size_t)>>
T mp_collection(node_t& node, trace_t call_point, const P& distance, const T& value, const T& null, G&& accumulate, F&& divide) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    return nbr(node, 0, null, null, [&](field<T> x){
        field<P> nbrdist = nbr(node, 1, distance);
        T v = fold_hood(node, 1, accumulate, mux(nbrdist > distance, x, field<T>{null}), value);
        int n = sum_hood(node, 1, mux(nbrdist < distance, 1, 0), 0);
        return std::make_pair(divide(v, max(n, 1)), v);
    });
}

//! @brief Collects distributed data with a weighted multi-path strategy.
template <typename node_t, typename T, typename G, typename F, typename = common::if_signature<G, T(T,T)>, typename = common::if_signature<F, T(T,double)>>
T wmp_collection(node_t& node, trace_t call_point, double distance, double radius, const T& value, G&& accumulate, F&& multiply) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    field<double> nbrdist = nbr(node, 0, distance);
    field<double> d = max(radius - node.nbr_dist(), field<double>{0});
    field<double> p = mux(isinf(distance) or isinf(nbrdist), field<double>{0},  distance - nbrdist);
    field<double> out_w = max(d * p, field<double>{0});
    double factor = sum_hood(node, 0, out_w, 0.0);
    if (factor == 0) factor = 1;
    field<double> in_w = nbr(node, 1, out_w / factor);
    return nbr(node, 2, value, [&](field<T> x){
        return fold_hood(node, 2, accumulate, map_hood(node, 2, multiply, x, in_w), value);
    });
}


}


}

#endif // FCPP_COORDINATION_COLLECTION_H_
