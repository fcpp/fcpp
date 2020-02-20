// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file scheduler.hpp
 * @brief Implementation of the `scheduler` component scheduling round executions.
 */

#ifndef FCPP_COMPONENT_SCHEDULER_H_
#define FCPP_COMPONENT_SCHEDULER_H_

#include <algorithm>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Component scheduling round executions.
 * Multiple instances may coexist in a composition of components.
 * Requires a `randomizer` parent component.
 * The `timer` component cannot be a parent of a `scheduler`, otherwise round planning may not work.
 *
 * @param G A sequence generator type.
 */
template <typename G>
struct scheduler {
    /**
     * @brief The actual component.
     * Component functionalities are added to those of the parent by inheritance at multiple levels: the whole component class inherits tag for static checks of correct composition, while `node` and `net` sub-classes inherit actual behaviour.
     * Further parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     * @param P The parent component to inherit from.
     */
    template <typename F, typename P>
    struct component : public P {
        //! @brief Checks if T has a `randomizer_tag`.
        template <typename T, typename = int>
        struct has_rtag : std::false_type {};
        template <typename T>
        struct has_rtag<T, std::conditional_t<true,int,typename T::randomizer_tag>> : std::true_type {};
        
        //! @brief Asserts that P has a `randomizer_tag`.
        static_assert(has_rtag<P>::value, "missing randomizer parent for scheduler component");
        
        //! @brief Checks if T has a `timer_tag`.
        template <typename T, typename = int>
        struct has_ttag : std::false_type {};
        template <typename T>
        struct has_ttag<T, std::conditional_t<true,int,typename T::timer_tag>> : std::true_type {};
        
        //! @brief Asserts that P has no `timer_tag`.
        static_assert(not has_ttag<P>::value, "timer cannot be parent of scheduler component");

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
            node(typename F::net& n, const tagged_tuple<S,T>& t) : P::node(n,t), m_schedule(P::node::generator(),t) {}
            
            /**
             * @brief Returns next event to schedule for the node component.
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_schedule.next(), P::node::next());
            }
            
            //! @brief Updates the internal status of node component.
            void update() {
                if (m_schedule.next() < P::node::next()) {
                    times_t t = P::node::as_final().next();
                    m_schedule.step(P::node::generator());
                    P::node::round(t);
                } else P::node::update();
            }
            
          private: // implementation details
            //! @brief The sequence generator.
            G m_schedule;
        };
        
        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}

#endif // FCPP_COMPONENT_SCHEDULER_H_
