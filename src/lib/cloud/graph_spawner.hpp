// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file graph_spawner.hpp
 * @brief Implementation of the `graph_spawner` component handling generation of nodes from a graph.
 */

#ifndef FCPP_CLOUD_GRAPH_SPAWNER_H_
#define FCPP_CLOUD_GRAPH_SPAWNER_H_

#include <type_traits>
#include <utility>

#include "lib/component/base.hpp"
#include "lib/option/sequence.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of tags and types of attributes describing a node in the input.
    template <typename... Ts>
    struct node_attributes {};

    //! @brief Declaration tag associating to a sequence generator type scheduling spawning of nodes.
    template <typename T>
    struct spawn_schedule {};

    //! @brief Node initialisation tag associating to a starting time of execution.
    struct start;
}


/**
 * @brief Component handling generation of nodes from a graph.
 *
 * Requires a \ref identifier parent component.
 * The \ref timer component cannot be a parent of a \ref spawner otherwise to preserve spawn scheduling.
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::init defines a sequence of node initialisation tags and generating distributions (defaults to the empty sequence).
 * - \ref tags::spawn_schedule defines a sequence generator type scheduling spawning of nodes (defaults to \ref sequence::never).
 *
 * Nodes generated receive all tags produced by generating distributions, and \ref tags::start associated to the creation time.
 */
template <class... Ts>
struct graph_spawner {
    //! @brief Sequence generator type scheduling spawning of nodes.
    using schedule_type = sequence::merge_t<common::option_types<tags::spawn_schedule, Ts...>>;

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
        DECLARE_COMPONENT(graph_spawner);
        REQUIRE_COMPONENT(graph_spawner,identifier);
        AVOID_COMPONENT(graph_spawner,timer);
        // TODO: remove if not needed for graphs
        //CHECK_COMPONENT(randomizer);

        //! @brief The local part of the component.
        using node = typename P::node;

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) :
                P::net(t),
                m_schedule(get_generator(has_randomizer<P>{}, *this),t),
                m_nodesfile( common::get_or<tags::nodesfile>(t, "index") ),
                m_arcsfile( common::get_or<tags::arcsfile>(t, "arcs") )
            {}

            /**
             * @brief Returns next event to schedule for the net component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_schedule.next(), P::net::next());
            }

            //! @brief Updates the internal status of net component.
            void update() {
                if (m_schedule.next() < P::net::next()) {
                    PROFILE_COUNT("graph_spawner");
                    times_t t = m_schedule.next();
                    size_t i = m_schedule.next_sequence();
                    m_schedule.step(get_generator(has_randomizer<P>{}, *this));
                    //call_distribution(i, t, inert_type{});
                } else P::net::update();
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

            //! @brief Adds a `start` time to a given tagged tuple.
            template <typename S, typename T>
            auto push_time(const common::tagged_tuple<S,T>& tup, times_t t) {
                using tt_type = typename common::tagged_tuple<S,T>::template push_back<tags::start, times_t>;
                tt_type tt(tup);
                common::get<tags::start>(tt) = t;
                return tt;
            }

            // //! @brief Emplaces a node generated through the j-th distribution.
            // template <size_t i>
            // inline void call_distribution(size_t j, times_t t, std::index_sequence<i>) {
            //     assert(i == j);
            //     P::net::node_emplace(push_time(std::get<i>(m_distributions)(get_generator(has_randomizer<P>{}, *this)), t));
            // }

            //! @brief The scheduling of spawning events.
            schedule_type m_schedule;
            string m_nodesfile;
            string m_arcsfile;
        };
    };
};


}


}

#endif // FCPP_SIMULATION_SPAWNER_H_
