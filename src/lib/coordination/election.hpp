// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file election.hpp
 * @brief Collection of field calculus leader election routines.
 */

#ifndef FCPP_COORDINATION_ELECTION_H_
#define FCPP_COORDINATION_ELECTION_H_

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


//! @brief Finds the minimum value, knowing an upper bound to the network diameter.
template <typename node_t, typename T>
T diameter_election(node_t& node, trace_t call_point, const T& value, int diameter) {
    data::trace_call trace_caller(node.stack_trace, call_point);
    
    return get<0>(nbr(node, 0, make_tuple(value, 0), [&](field<tuple<T,int>> x){
        tuple<T,int> best = fold_hood(node, 0, [&](tuple<T,int> const& a, tuple<T,int> const& b){
            return get<1>(a) < diameter and a < b ? a : b;
        }, x, make_tuple(value, -1));
        get<1>(best) += 1;
        return best;
    }));
}

//! @brief Finds the minimum UID, knowing an upper bound to the network diameter.
template <typename node_t>
device_t diameter_election(node_t& node, trace_t call_point, int diameter) {
    return diameter_election(node, call_point, node.uid, diameter);
}


//! @brief Finds the minimum value, without any additional knowledge, and following a given expansion function.
template <typename node_t, typename T, typename G, typename = common::if_signature<G, int(int)>>
T wave_election(node_t& node, trace_t call_point, const T& value, G&& expansion) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    return get<0>(nbr(node, 0, make_tuple(value, 0, -expansion(0), 0), [&](field<tuple<T,int,int,int>> x){
        tuple<T,int,int,int> next = fold_hood(node, 0, [](tuple<T,int,int,int> const& a, tuple<T,int,int,int> const& b){
            return get<1>(a) < -get<2>(a) and a < b ? a : b;
        }, x, make_tuple(value, -1, -expansion(0), 0));
        get<1>(next) += 1;
        get<3>(next) = max(fold_hood(node, 0, [&](tuple<T,int,int,int> const& a, int b){
            return get<1>(a) > get<1>(next) and get<3>(a) > b ? get<3>(a) : b;
        }, x, 0), get<1>(next));
        if (get<1>(next) == 0) get<2>(next) = -expansion(get<3>(next));
        return next;
    }));
}

//! @brief Finds the minimum value, without any additional knowledge.
template <typename node_t, typename T>
T wave_election(node_t& node, trace_t call_point, const T& value) {
    return wave_election(node, call_point, value, [](int x){
        // expansion function granting recovery time below (1+√2)x diameter election (for x ≥ 3)
        return std::max(int(2.414213562373095*x+4.6), 6);
    });
}

//! @brief Finds the minimum UID, without any additional knowledge.
template <typename node_t>
device_t wave_election(node_t& node, trace_t call_point) {
    return wave_election(node, call_point, node.uid);
}


}


}

#endif // FCPP_COORDINATION_ELECTION_H_
