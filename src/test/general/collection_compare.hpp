// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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
FUN real_t generic_distance(ARGS, int algorithm, bool source) { CODE
    if (algorithm == 0) return abf_distance(CALL, source);
    if (algorithm == 1) return bis_distance(CALL, source, 1, 50);
    if (algorithm == 2) return flex_distance(CALL, source, 0.2f, 100, 0.1f, 10);
    return 0;
}

//! @brief Export list for generic_distance.
FUN_EXPORT generic_distance_t = common::export_list<abf_distance_t, bis_distance_t, flex_distance_t>;

//! @brief Device counting case study.
FUN void device_counting(ARGS, bool is_source, real_t dist) { CODE
    real_t value = 1;

    auto adder = [](real_t x, real_t y) {
        return x+y;
    };
    auto divider = [](real_t x, size_t n) {
        return x/n;
    };
    auto multiplier = [](real_t x, real_t f) {
        return x*f;
    };
    real_t spc = sp_collection(CALL, dist, value, 0, adder);
    real_t mpc = mp_collection(CALL, dist, value, 0, adder, divider);
    real_t wmpc = wmp_collection(CALL, dist, 100, value, adder, multiplier);
    node.storage(tags::spc_sum{}) = is_source ? spc : 0;
    node.storage(tags::mpc_sum{}) = is_source ? mpc : 0;
    node.storage(tags::wmpc_sum{}) = is_source ? wmpc : 0;
    node.storage(tags::ideal_sum{}) = value;
}

//! @brief Export list for device_counting.
FUN_EXPORT device_counting_t = common::export_list<sp_collection_t<real_t, real_t>, mp_collection_t<real_t, real_t>, wmp_collection_t<real_t>>;

//! @brief Progress tracking case study.
FUN void progress_tracking(ARGS, bool is_source, device_t source_id, real_t dist) { CODE
    real_t value = distance(node.net.node_at(source_id).position(), node.position()) + (500 - node.current_time());
    real_t threshold = 3.5f / count_hood(CALL);

    auto adder = [](real_t x, real_t y) {
        return max(x,y);
    };
    auto divider = [](real_t x, size_t) {
        return x;
    };
    auto multiplier = [&](real_t x, real_t f) {
        return f > threshold ? x : 0;
    };
    real_t spc = sp_collection(CALL, dist, value, 0, adder);
    real_t mpc = mp_collection(CALL, dist, value, 0, adder, divider);
    real_t wmpc = wmp_collection(CALL, dist, 100, value, adder, multiplier);
    node.storage(tags::spc_max{}) = is_source ? spc : 0;
    node.storage(tags::mpc_max{}) = is_source ? mpc : 0;
    node.storage(tags::wmpc_max{}) = is_source ? wmpc : 0;
    node.storage(tags::ideal_max{}) = value;
}

//! @brief Export list for progress_tracking.
FUN_EXPORT progress_tracking_t = common::export_list<sp_collection_t<real_t, real_t>, mp_collection_t<real_t, real_t>, wmp_collection_t<real_t>>;

//! @brief Main function.
MAIN() {
    rectangle_walk(CALL, make_vec(0,0), make_vec(2000,200), 30.5f, 1);

    device_t source_id = node.current_time() < 250 ? 0 : 1;
    bool is_source = node.uid == source_id;
    int dist_algo = node.storage(tags::algorithm{});
    real_t dist = generic_distance(CALL, dist_algo, is_source);

    device_counting(CALL, is_source, dist);
    progress_tracking(CALL, is_source, source_id, dist);
}

//! @brief Export list for main.
FUN_EXPORT main_t = common::export_list<rectangle_walk_t<2>, generic_distance_t, device_counting_t, progress_tracking_t>;


}


}

#endif // FCPP_COLLECTION_COMPARE_H_
