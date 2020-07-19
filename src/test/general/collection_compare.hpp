// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file collection_compare.hpp
 * @brief Implementation of the case study comparing collection algorithms.
 */

#ifndef FCPP_COLLECTION_COMPARE_H_
#define FCPP_COLLECTION_COMPARE_H_

#include "lib/beautify.hpp"
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
FUN() double generic_distance(ARGS, int algorithm, bool source) { CODE
    if (algorithm == 0) return abf_distance(CALL, source);
    if (algorithm == 1) return bis_distance(CALL, source, 1.0, 50.0);
    if (algorithm == 2) return flex_distance(CALL, source, 0.2, 100.0, 0.1, 10);
    return 0;
}

//! @brief Device counting case study.
FUN() void device_counting(ARGS, bool is_source, double dist) { CODE
    auto adder = [](double x, double y) {
        return x+y;
    };
    auto divider = [](double x, size_t n) {
        return x/n;
    };
    auto multiplier = [](double x, double f) {
        return x*f;
    };
    double spc = sp_collection(CALL, dist, 1.0, 0.0, adder);
    double mpc = mp_collection(CALL, dist, 1.0, 0.0, adder, divider);
    double wmpc = wmp_collection(CALL, dist, 100.0, 1.0, adder, multiplier);
    node.storage(tags::spc_sum{}) = is_source ? spc : 0;
    node.storage(tags::mpc_sum{}) = is_source ? mpc : 0;
    node.storage(tags::wmpc_sum{}) = is_source ? wmpc : 0;
    node.storage(tags::ideal_sum{}) = 1.0;
}

//! @brief Progress tracking case study.
FUN() void progress_tracking(ARGS, bool is_source, device_t source_id, double dist) { CODE
    double value = distance(node.net.node_at(source_id).position(), node.position()) + (500 - node.current_time());
    double threshold = 3.5 / count_hood(CALL);
    
    auto adder = [](double x, double y) {
        return max(x,y);
    };
    auto divider = [](double x, size_t) {
        return x;
    };
    auto multiplier = [&](double x, double f) {
        return f > threshold ? x : 0;
    };
    double spc = sp_collection(CALL, dist, value, 0.0, adder);
    double mpc = mp_collection(CALL, dist, value, 0.0, adder, divider);
    double wmpc = wmp_collection(CALL, dist, 100.0, value, adder, multiplier);
    node.storage(tags::spc_max{}) = is_source ? spc : 0;
    node.storage(tags::mpc_max{}) = is_source ? mpc : 0;
    node.storage(tags::wmpc_max{}) = is_source ? wmpc : 0;
    node.storage(tags::ideal_max{}) = value;
}

//! @brief Main function.
FUN() void collection_compare(ARGS) { CODE
    rectangle_walk(CALL, make_vec(0,0), make_vec(2000,200), 30.5, 1);
    
    device_t source_id = node.current_time() < 250 ? 0 : 1;
    bool is_source = node.uid == source_id;
    int dist_algo = node.storage(tags::algorithm{});
    double dist = generic_distance(CALL, dist_algo, is_source);
    
    device_counting(CALL, is_source, dist);
    progress_tracking(CALL, is_source, source_id, dist);
}


}


//! @brief Main struct calling `collection_compare`.
MAIN(coordination::collection_compare,);


}

#endif // FCPP_COLLECTION_COMPARE_H_
