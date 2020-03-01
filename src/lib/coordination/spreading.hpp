// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file spreading.hpp
 * @brief Implementation of the `spreading` component providing field calculus distance estimation routines.
 */

#ifndef FCPP_COORDINATION_SPREADING_H_
#define FCPP_COORDINATION_SPREADING_H_

#include <algorithm>
#include <functional>
#include <limits>

#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


/**
 * @brief Component providing field calculus distance estimation routines.
 *
 * Must be unique in a composition of components.
 */
struct spreading {
    /**
     * @brief The actual component.
     *
     * Component functionalities are added to those of the parent by inheritance at multiple levels: the whole component class inherits tag for static checks of correct composition, while `node` and `net` sub-classes inherit actual behaviour.
     * Further parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     * @param P The parent component to inherit from.
     */
    template <typename F, typename P>
    struct component : public P {
        //! @brief Marks that a storage component is present.
        struct spreading_tag {};
        
        //! @brief Checks if T has a `spreading_tag`.
        template <typename T, typename = int>
        struct has_stag : std::false_type {};
        template <typename T>
        struct has_stag<T, std::conditional_t<true,int,typename T::spreading_tag>> : std::true_type {};
        
        //! @brief Asserts that P has no `spreading_tag`.
        static_assert(not has_stag<P>::value, "cannot combine multiple spreading components");

        //! @brief The local part of the component.
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
            
          protected: // visible by node objects only
            //! @brief Reduces the values in the domain of a field to a single value by minimum.
            template <trace_t __, typename A>
            A min_hood(const field<A>& f) {
                typename P::node::template trace_call<__> _;

                return P::node::template fold_hood<___>([] (A x, A y) {
                    return std::min(x, y);
                }, f);
            }

            //! @brief Computes the distance from a source through adaptive bellmann-ford.
            template<trace_t __>
            double distance(bool source, std::function<field<double>()> metric) {
                typename P::node::template trace_call<__> _;

                return P::node::template nbr<___, double>(std::numeric_limits<double>::infinity(), [this,source,&metric] (fcpp::field<double> d) {
                    double r = min_hood<___>( d + metric() );
                    return source ? 0.0 : r;
                });
            }
        };
        
        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_COORDINATION_SPREADING_H_
