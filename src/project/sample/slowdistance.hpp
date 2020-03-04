// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file spreading.hpp
 * @brief Implementation of the `spreading` component providing field calculus distance estimation routines.
 */

#ifndef SLOWDISTANCE_H_
#define SLOWDISTANCE_H_

#include <cmath>

#include <functional>
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
                std::function<field<double>()> metric = [this](){
                    return P::node::nbr_dist();
                };
                double fastd = P::node::template distance<___>(source, metric);
                double slowd = slowdistance<___>(source, metric);
                double ideal = std::norm(P::node::net.node_at(0).position() - P::node::position());
                P::node::template storage<tags::fastdist>()  = fastd;
                P::node::template storage<tags::slowdist>()  = slowd;
                P::node::template storage<tags::idealdist>() = ideal;
                P::node::template storage<tags::fasterr>()  = std::abs(fastd - ideal);
                P::node::template storage<tags::slowerr>()  = std::abs(slowd - ideal);
            }
            
          protected: // visible by node objects only
            //! @brief Computes the distance from a source through adaptive bellmann-ford with old+nbr.
            template<trace_t __>
            double slowdistance(bool source, std::function<field<double>()> metric) {
                typename P::node::template trace_call<__> _;

                return P::node::template old<___, double>(std::numeric_limits<double>::infinity(), [this,source,&metric] (double d) {
                    double r = P::node::template min_hood<___>( P::node::template nbr<___>(d) + metric() );
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
