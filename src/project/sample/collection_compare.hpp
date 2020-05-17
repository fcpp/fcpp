// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file collection_compare.hpp
 * @brief Implementation of the `slowdistance` case study.
 */

#ifndef COLLECTION_COMPARE_H_
#define COLLECTION_COMPARE_H_

#include "lib/coordination/collection.hpp"
#include "lib/coordination/geometry.hpp"
#include "lib/coordination/spreading.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


namespace tags {
    //! @brief Desired distance algorithm.
    struct algorithm {};

    //! @brief Output values.
    //! @{
    struct spc_sum {};
    struct mpc_sum {};
    struct wmpc_sum {};
    struct ideal_sum {};
    struct spc_max {};
    struct mpc_max {};
    struct wmpc_max {};
    struct ideal_max {};
    //! @}
}


//! @brief Computes the distance from a source through adaptive bellmann-ford with old+nbr.
template <typename node_t>
double generic_distance(node_t& node, trace_t call_point, int algorithm, bool source) {
    data::trace_call trace_caller(node.stack_trace, call_point);
    
    if (algorithm == 0) return abf_distance(node, 0, source);
    if (algorithm == 1) return bis_distance(node, 1, source, 1.0, 50.0);
    if (algorithm == 2) return flex_distance(node, 2, source, 0.2, 100.0, 0.1, 10);
    return 0;
}

//! @brief Device counting case study.
template <typename node_t>
void device_counting(node_t& node, trace_t call_point, bool is_source, double distance) {
    data::trace_call trace_caller(node.stack_trace, call_point);
    
    auto adder = [](double x, double y) {
        return x+y;
    };
    auto divider = [](double x, size_t n) {
        return x/n;
    };
    auto multiplier = [](double x, double f) {
        return x*f;
    };
    double spc = sp_collection(node, 0, distance, 1.0, 0.0, adder);
    double mpc = mp_collection(node, 1, distance, 1.0, 0.0, adder, divider);
    double wmpc = wmp_collection(node, 2, distance, 100.0, 1.0, adder, multiplier);
    node.storage(tags::spc_sum{}) = is_source ? spc : 0;
    node.storage(tags::mpc_sum{}) = is_source ? mpc : 0;
    node.storage(tags::wmpc_sum{}) = is_source ? wmpc : 0;
    node.storage(tags::ideal_sum{}) = 1.0;
}

//! @brief Progress tracking case study.
template <typename node_t>
void progress_tracking(node_t& node, trace_t call_point, bool is_source, device_t source_id, double distance) {
    data::trace_call trace_caller(node.stack_trace, call_point);
    
    double value = std::distance(node.net.node_at(source_id).position(), node.position()) + (500 - node.current_time());
    double threshold = 3.5 / count_hood(node, 0);
    
    auto adder = [](double x, double y) {
        return max(x,y);
    };
    auto divider = [](double x, size_t) {
        return x;
    };
    auto multiplier = [&](double x, double f) {
        return f > threshold ? x : 0;
    };
    double spc = sp_collection(node, 0, distance, value, 0.0, adder);
    double mpc = mp_collection(node, 1, distance, value, 0.0, adder, divider);
    double wmpc = wmp_collection(node, 2, distance, 100.0, value, adder, multiplier);
    node.storage(tags::spc_max{}) = is_source ? spc : 0;
    node.storage(tags::mpc_max{}) = is_source ? mpc : 0;
    node.storage(tags::wmpc_max{}) = is_source ? wmpc : 0;
    node.storage(tags::ideal_max{}) = value;
}

//! @brief Main function.
template <typename node_t>
void collection_compare(node_t& node, trace_t call_point) {
    data::trace_call trace_caller(node.stack_trace, call_point);
    
    rectangle_walk(node, 0, std::array<double, 2>{0,0}, std::array<double, 2>{2000,200}, 30.5, 1);
    
    device_t source_id = node.current_time() < 250 ? 0 : 1;
    bool is_source = node.uid == source_id;
    int dist_algo = node.storage(tags::algorithm{});
    double distance = generic_distance(node, 1, dist_algo, is_source);
    
    device_counting(node, 2, is_source, distance);
    progress_tracking(node, 3, is_source, source_id, distance);
}


}


//! @brief Main struct calling `collection_compare`.
struct main {
    template <typename node_t>
    void operator()(node_t& node, times_t) {
        coordination::collection_compare(node, 0);
    }
};


}

#endif // COLLECTION_COMPARE_H_
