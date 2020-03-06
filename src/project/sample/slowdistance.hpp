// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file spreading.hpp
 * @brief Implementation of the `spreading` component providing field calculus distance estimation routines.
 */

#ifndef SLOWDISTANCE_H_
#define SLOWDISTANCE_H_

#include <cmath>

#include <limits>

#include "lib/common/array.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


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


//! @brief Component providing a slower version of distance estimation, and comparing it with the faster one.
struct slowdistance {
    template <typename F, typename P>
    struct component : public P {
        class node : public P::node {
          public: // visible by net objects and the main program
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t) {}
            
            //! @brief Performs computations at round middle with current time `t`.
            void round_main(times_t) {
                bool source = P::node::uid == 0;
                auto metric = [this](){
                    return P::node::nbr_dist();
                };
                double fastd = distance(___, source, metric);
                double slowd = slowdistance(___, source, metric);
                double ideal = std::norm(P::node::net.node_at(0).position() - P::node::position());
                storage(tags::fastdist{})  = fastd;
                storage(tags::slowdist{})  = slowd;
                storage(tags::idealdist{}) = ideal;
                storage(tags::fasterr{})   = std::abs(fastd - ideal);
                storage(tags::slowerr{})   = std::abs(slowd - ideal);
            }
            
          protected: // visible by node objects only
            using P::node::old;
            using P::node::nbr;
            using P::node::min_hood;
            using P::node::distance;
            using P::node::storage;
            
            //! @brief Computes the distance from a source through adaptive bellmann-ford with old+nbr.
            template <typename G, typename = common::if_signature<G, field<double>()>>
            double slowdistance(trace_t __, bool source, G&& metric) {
                data::trace_call _(__);

                return old(___, std::numeric_limits<double>::infinity(), [this,source,&metric] (double d) {
                    double r = min_hood(___, nbr(___, d) + metric());
                    return source ? 0.0 : r;
                });
            }
        };
        using net = typename P::net;
    };
};


}


}

#endif // SLOWDISTANCE_H_
