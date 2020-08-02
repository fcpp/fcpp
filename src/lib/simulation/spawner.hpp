// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file spawner.hpp
 * @brief Implementation of the `spawner` component handling automated generation of nodes.
 */

#ifndef FCPP_SIMULATION_SPAWNER_H_
#define FCPP_SIMULATION_SPAWNER_H_

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
    //! @brief Declaration tag associating to a sequence of node initialisation tags and generating distributions.
    template <typename... Ts>
    struct init {};

    //! @brief Declaration tag associating to a sequence generator type scheduling spawning of nodes.
    template <typename T>
    struct spawn_schedule {};

    //! @brief Node initialisation tag associating to a starting time of execution.
    struct start;
}


/**
 * @brief Component handling automated generation of nodes.
 *
 * Multiple instances may coexist in a composition of components.
 * Requires a \ref identifier parent component.
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::init defines a sequence of node initialisation tags and generating distributions (defaults to the empty sequence).
 * - \ref tags::spawn_schedule defines a sequence generator type scheduling spawning of nodes (defaults to \ref sequence::never).
 *
 * Nodes generated receive all tags produced by generating distributions, and \ref tags::start associated to the creation time.
 */
template <class... Ts>
struct spawner {
    //! @brief Sequence of node initialisation tags and generating distributions.
    using init_type = common::option_multitypes<tags::init, Ts...>;

    //! @brief Node initialisation tags and generating distributions as tuple of tagged tuples.
    using tuple_type = common::apply_templates<init_type, std::tuple, common::tagged_tuple_t>;

    //! @brief Node initialisation tags and generating distributions as sequence of tagged tuples.
    using inert_type = common::apply_templates<init_type, common::type_sequence, common::tagged_tuple_t>;

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
        DECLARE_COMPONENT(spawner);
        REQUIRE_COMPONENT(spawner,identifier);
        CHECK_COMPONENT(randomizer);

        //! @brief The local part of the component.
        using node = typename P::node;

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) : P::net(t), m_schedule(get_generator(has_randomizer<P>{}, *this),t), m_distributions(build_distributions_tuple(t, inert_type{})) {}

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
                    PROFILE_COUNT("spawner");
                    times_t t = m_schedule.next();
                    size_t i = m_schedule.next_sequence();
                    m_schedule.step(get_generator(has_randomizer<P>{}, *this));
                    call_distribution(i, t, inert_type{});
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

            //! @brief Constructs the tuple of distributions, feeding the initialising tuple to all of them.
            template <typename S, typename T, typename... Us>
            tuple_type build_distributions_tuple(const common::tagged_tuple<S,T>& t, common::type_sequence<Us...>) {
                return {build_distributions(t, typename Us::tags(), typename Us::types())...};
            }

            //! @brief Constructs the tuple of distributions, feeding the initialising tuple to all of them.
            template <typename S, typename T, typename... Ss, typename... Us>
            common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>
            build_distributions(const common::tagged_tuple<S,T>& t, common::type_sequence<Ss...>, common::type_sequence<Us...>) {
                return {Us{get_generator(has_randomizer<P>{}, *this),t}...};
            }

            //! @brief Adds a `start` time to a given tagged tuple.
            template <typename S, typename T>
            auto push_time(const common::tagged_tuple<S,T>& tup, times_t t) {
                using tt_type = typename common::tagged_tuple<S,T>::template push_back<tags::start, times_t>;
                tt_type tt(tup);
                common::get<tags::start>(tt) = t;
                return tt;
            }

            //! @brief Emplaces a node generated through the j-th distribution.
            template <size_t i>
            inline void call_distribution(size_t j, times_t t, std::index_sequence<i>) {
                assert(i == j);
                P::net::node_emplace(push_time(std::get<i>(m_distributions)(get_generator(has_randomizer<P>{}, *this)), t));
            }

            //! @brief Emplaces a node generated through the j-th distribution.
            template <size_t i, size_t... is>
            inline void call_distribution(size_t j, times_t t, std::index_sequence<i, is...>) {
                if (i == j)
                    call_distribution(j, t, std::index_sequence<i>{});
                else
                    call_distribution(j, t, std::index_sequence<is...>{});
            }

            //! @brief Emplaces a node generated through the j-th distribution.
            template <typename... Us>
            void call_distribution(size_t j, times_t t, common::type_sequence<Us...>) {
                return call_distribution(j, t, std::make_index_sequence<sizeof...(Us)>{});
            }

            //! @brief The scheduling of spawning events.
            schedule_type m_schedule;

            //! @brief The generator tuple.
            tuple_type m_distributions;
        };
    };
};


}


}

#endif // FCPP_SIMULATION_SPAWNER_H_
