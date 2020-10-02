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


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Finds the minimum value, knowing an upper bound to the network diameter.
template <typename node_t, typename T>
T diameter_election(node_t& node, trace_t call_point, const T& value, hops_t diameter) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return get<0>(nbr(node, 0, make_tuple(value, hops_t{0}), [&](field<tuple<T,hops_t>> x){
        tuple<T,hops_t> best = fold_hood(node, 0, [&](tuple<T,hops_t> const& a, tuple<T,hops_t> const& b){
            return get<1>(a) < diameter and a < b ? a : b;
        }, x, make_tuple(value, hops_t{0}));
        get<1>(best) += 1;
        return min(best, make_tuple(value, hops_t{0}));
    }));
}

//! @brief Finds the minimum UID, knowing an upper bound to the network diameter.
template <typename node_t>
inline device_t diameter_election(node_t& node, trace_t call_point, int diameter) {
    return diameter_election(node, call_point, node.uid, diameter);
}


//! @brief Finds the minimum value, without any additional knowledge, and following a given expansion function.
template <typename node_t, typename T, typename G, typename = common::if_signature<G, int(int)>>
T wave_election(node_t& node, trace_t call_point, const T& value, G&& expansion) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

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
inline T wave_election(node_t& node, trace_t call_point, const T& value) {
    return wave_election(node, call_point, value, [](int x){
        // expansion function granting recovery time below (1+√2)x diameter election (for x ≥ 3)
        return std::max(int(2.414213562373095*x+4.6), 6);
    });
}

//! @brief Finds the minimum UID, without any additional knowledge.
template <typename node_t>
inline device_t wave_election(node_t& node, trace_t call_point) {
    return wave_election(node, call_point, node.uid);
}


//! @brief Finds the minimum value, without any additional knowledge, through alternating colors.
template <typename node_t, typename T>
T color_election(node_t& node, trace_t call_point, const T& value) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    using key_type = tuple<bool,T,int,device_t>;
    constexpr size_t disable = 0;
    constexpr size_t leader  = 1;
    constexpr size_t level   = 2;
    constexpr size_t parent  = 3;

    key_type self_key{false, value, 0, node.uid};
    return get<leader>(nbr(node, 0, self_key, [&](field<key_type> nbr_keys) -> key_type {
        key_type const& old_key = self(node, 0, nbr_keys);
        key_type const& parent_key = self(node, 0, nbr_keys, get<parent>(old_key));
        key_type const* pbk = &self_key;
        device_t best_nbr = fold_hood(node, 0, [&](device_t curr, key_type const& key, device_t best){
            if (key < *pbk) {
                pbk = &key;
                return curr;
            } else return best;
        }, nbr_keys, node.uid);
        key_type const& best_nbr_key = *pbk;
        auto same_key_check = [&](key_type const& k, key_type const& h) {
            return get<leader>(k) == get<leader>(h) and get<level>(k) == get<level>(h);
        };
        auto succ_key_check = [&](key_type const& succ, key_type const& prev) {
            return get<leader>(succ) == get<leader>(prev) and get<level>(succ) == get<level>(prev)+1;
        };
        auto better_key_check = [&](key_type const& low, key_type const& hi) {
            return get<leader>(low) < get<leader>(hi) or (get<leader>(low) == get<leader>(hi) and get<level>(low)+1 < get<level>(hi));
        };
        auto true_child_check = [&](key_type const& child) {
            return get<parent>(child) == node.uid and succ_key_check(child, old_key);
        };
        auto false_child_check = [&](key_type const& child) {
            return get<parent>(child) == node.uid and not succ_key_check(child, old_key);
        };
        bool is_true_root = same_key_check(old_key, self_key);
        bool is_true_child = get<leader>(old_key) < value and succ_key_check(old_key, parent_key);
        bool is_false_root = not (is_true_root or is_true_child);
        bool has_false_child = fold_hood(node, 0, [&](key_type const& key, bool holds){
            return holds or false_child_check(key);
        }, nbr_keys, false);
        bool best_nbr_improves = better_key_check(best_nbr_key, old_key);
        if (best_nbr != node.uid and (is_false_root or best_nbr_improves) and (not has_false_child))
            return {true, get<leader>(best_nbr_key), get<level>(best_nbr_key) + 1, best_nbr};
        if (is_false_root)
            return self_key;
        bool has_recruit = fold_hood(node, 0, [&](key_type const& key, bool holds){
            return holds or better_key_check(old_key, key);
        }, nbr_keys, false);
        bool has_similar_child = fold_hood(node, 0, [&](key_type const& key, bool holds){
            return holds or (true_child_check(key) and get<disable>(key) == get<disable>(old_key));
        }, nbr_keys, false);
        bool similar_parent = get<disable>(parent_key) == get<disable>(old_key);
        if (similar_parent and (get<disable>(old_key) or not has_recruit) and not has_similar_child) {
            key_type new_key = old_key;
            get<disable>(new_key) = not get<disable>(new_key);
            return new_key;
        }
        return old_key;
    }));
}

//! @brief Finds the minimum value, without any additional knowledge, through alternating colors.
template <typename node_t>
inline device_t color_election(node_t& node, trace_t call_point) {
    return color_election(node, call_point, node.uid);
}


}


}

#endif // FCPP_COORDINATION_ELECTION_H_
