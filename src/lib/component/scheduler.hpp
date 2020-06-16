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
#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence generator type scheduling rounds.
    template <typename T>
    struct round_schedule {};
}


/**
 * @brief Component scheduling round executions.
 *
 * Multiple instances may coexist in a composition of components.
 * The \ref timer component cannot be a parent of a \ref scheduler otherwise round planning may not work.
 * If a \ref randomizer parent component is not found, \ref random::crand is used as random generator.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::round_schedule defines a sequence generator type scheduling rounds (defaults to \ref random::sequence_never).
 */
template <class... Ts>
struct scheduler {
    //! @brief Sequence generator type scheduling rounds.
    using schedule_type = common::option_type<tags::round_schedule, random::sequence_never, Ts...>;

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
        //! @brief Checks if T has a `randomizer_tag`.
        template <typename T, typename = int>
        struct has_rtag : std::false_type {};
        template <typename T>
        struct has_rtag<T, std::conditional_t<true,int,typename T::randomizer_tag>> : std::true_type {};
        
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
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_schedule(get_generator(common::bool_pack<has_rtag<P>::value>(), *this),t) {}
            
            /**
             * @brief Returns next event to schedule for the node component.
             * 
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_schedule.next(), P::node::next());
            }
            
            //! @brief Updates the internal status of node component.
            void update() {
                if (m_schedule.next() < P::node::next()) {
                    times_t t = P::node::as_final().next();
                    m_schedule.step(get_generator(common::bool_pack<has_rtag<P>::value>(), *this));
                    P::node::round(t);
                } else P::node::update();
            }
            
          private: // implementation details
            //! @brief Returns the `randomizer` generator if available.
            template <typename N>
            inline auto& get_generator(common::bool_pack<true>, N& n) {
                return n.generator();
            }

            //! @brief Returns a `crand` generator otherwise.
            template <typename N>
            inline random::crand get_generator(common::bool_pack<false>, N&) {
                return random::crand();
            }
            
            //! @brief The sequence generator.
            schedule_type m_schedule;
        };
        
        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_COMPONENT_SCHEDULER_H_
