// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file slow_distance.hpp
 * @brief Implementation of the case study comparing "share" to "rep+nbr" for AFB distances.
 */

#ifndef FCPP_SLOW_DISTANCE_H_
#define FCPP_SLOW_DISTANCE_H_

#include "lib/coordination/spreading.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @brief Computes the distance from a source through adaptive bellmann-ford with old+nbr.
template <typename node_t, typename G, typename = common::if_signature<G, field<real_t>()>>
real_t slow_distance(node_t& node, trace_t call_point, bool source, G&& metric) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    return old(node, 0, INF, [&] (real_t d) {
        real_t r = min_hood(node, 1, nbr(node, 2, d) + metric());
        return source ? 0 : r;
    });
}

//! @brief Export list for slow_distance.
using slow_distance_t = common::export_list<real_t>;

//! @brief Counts the number of communications with each neighbour.
template <typename node_t>
field<int> connection(node_t& node, trace_t call_point) {
    return nbr(node, call_point, field<int>{0}, [&](field<int> n) {
        return n + mod_other(node, call_point, 1, 0);
    });
}

//! @brief Export list for connection.
using connection_t = common::export_list<field<int>>;


namespace tags {
    //! @brief Ideal distance values.
    struct idealdist {};

    //! @brief Fast distance values.
    struct fastdist {};

    //! @brief Slow distance values.
    struct slowdist {};

    //! @brief Fast distance values error.
    struct fasterr {};

    //! @brief Slow distance values error.
    struct slowerr {};
}


template <typename node_t>
void distance_compare(node_t& node, trace_t call_point) {
    internal::trace_call trace_caller(node.stack_trace, call_point);

    bool source = node.uid == 0;
    auto metric = [&](){
        return node.nbr_dist();
    };
    real_t fastd = abf_distance(node, 0, source, metric);
    real_t slowd = slow_distance(node, 1, source, metric);
    real_t ideal = norm(node.net.node_at(0).position() - node.position());
    node.storage(tags::fastdist{})  = fastd;
    node.storage(tags::slowdist{})  = slowd;
    node.storage(tags::idealdist{}) = ideal;
    node.storage(tags::fasterr{})   = std::abs(fastd - ideal);
    node.storage(tags::slowerr{})   = std::abs(slowd - ideal);
}

//! @brief Export list for distance_compare.
using distance_compare_t = common::export_list<abf_distance_t, slow_distance_t>;


}


struct main {
    template <typename node_t>
    void operator()(node_t& node, times_t) {
        coordination::distance_compare(node, 0);
    }
};


}

#endif // FCPP_SLOW_DISTANCE_H_
