// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file scheduler.hpp
 * @brief Implementation of the `scheduler` component scheduling round executions.
 */

#ifndef FCPP_COMPONENT_SCHEDULER_H_
#define FCPP_COMPONENT_SCHEDULER_H_

#include <algorithm>
#include <type_traits>

#include "lib/component/base.hpp"
#include "lib/option/sequence.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a list of sequence generator type scheduling rounds (defaults to \ref sequence::never).
    template <typename... Ts>
    struct round_schedule {};
}


/**
 * @brief Component scheduling round executions.
 *
 * The \ref timer component cannot be a parent of a \ref scheduler otherwise round planning may not work.
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::round_schedule defines a list of sequence generator type scheduling rounds (defaults to \ref sequence::never).
 */
template <class... Ts>
struct scheduler {
    //! @brief Sequence generator type scheduling rounds.
    using schedule_type = sequence::merge_t<common::option_types<tags::round_schedule, Ts...>>;

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
        //! @cond INTERNAL
        DECLARE_COMPONENT(scheduler);
        AVOID_COMPONENT(scheduler,timer);
        CHECK_COMPONENT(randomizer);
        //! @endcond

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
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_schedule(get_generator(has_randomizer<P>{}, *this),t) {}

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
                    m_schedule.step(get_generator(has_randomizer<P>{}, *this), common::tagged_tuple_t<>{});
                    P::node::round(t);
                } else P::node::update();
            }

          private: // implementation details
            //! @brief Returns the `randomizer` generator if available.
            template <typename N>
            inline auto& get_generator(std::true_type, N& n) {
                return n.generator();
            }

            //! @brief Returns a `crand` generator otherwise.
            template <typename N>
            inline crand get_generator(std::false_type, N&) {
                return {};
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
