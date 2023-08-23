// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file spawner.hpp
 * @brief Implementation of the `spawner` component handling automated generation of nodes.
 */

#ifndef FCPP_SIMULATION_SPAWNER_H_
#define FCPP_SIMULATION_SPAWNER_H_

#include <tuple>
#include <type_traits>
#include <utility>

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
    //! @brief Declaration tag associating to a sequence of node initialisation tags and generating distributions (defaults to the empty sequence).
    template <typename... Ts>
    struct init {};

    //! @brief Declaration tag associating to a sequence generator type scheduling spawning of nodes (defaults to \ref sequence::never).
    template <typename T>
    struct spawn_schedule {};

    //! @brief Node initialisation tag associating to a starting time of execution (defaults to \ref TIME_MAX).
    struct start;
}


/**
 * @brief Component handling automated generation of nodes.
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
        //! @cond INTERNAL
        DECLARE_COMPONENT(spawner);
        REQUIRE_COMPONENT(spawner,identifier);
        AVOID_COMPONENT(spawner,timer);
        CHECK_COMPONENT(randomizer);
        //! @endcond

        //! @brief The local part of the component.
        using node = typename P::node;

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) : P::net(t), m_schedule(get_generator(has_randomizer<P>{}, *this),t), m_distributions(build_distributions_tuple(t, inert_type{})) {}

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
                    while (t == m_schedule.next()) {
                        size_t i = m_schedule.next_sequence();
                        m_schedule.step(get_generator(has_randomizer<P>{}, *this), common::tagged_tuple_t<>{});
                        call_distribution(i, t, std::index_sequence<0, std::tuple_size<tuple_type>::value>{});
                    }
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
            tuple_type build_distributions_tuple(common::tagged_tuple<S,T> const& t, common::type_sequence<Us...>) {
                return {build_distributions(t, typename Us::tags(), typename Us::types())...};
            }

            //! @brief Constructs the tuple of distributions, feeding the initialising tuple to all of them.
            template <typename S, typename T, typename... Ss, typename... Us>
            common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>
            build_distributions(common::tagged_tuple<S,T> const& t, common::type_sequence<Ss...>, common::type_sequence<Us...>) {
                return {Us{get_generator(has_randomizer<P>{}, *this),t}...};
            }

            //! @brief Builds node with a distribution tagged tuple among an interval of possible ones.
            template <size_t i, size_t j>
            inline std::enable_if_t<(j > i+1)> call_distribution(size_t l, times_t t, std::index_sequence<i, j>) {
                constexpr size_t k = (i + j) / 2;
                if (l < k)
                    call_distribution(l, t, std::index_sequence<i, k>{});
                else
                    call_distribution(l, t, std::index_sequence<k, j>{});
            }

            //! @brief Builds node with a given distribution tagged tuple.
            template <size_t i, size_t j>
            inline std::enable_if_t<j == i+1> call_distribution(size_t l, times_t t, std::index_sequence<i, j>) {
                assert(l == i);
                using dist_type = std::decay_t<decltype(std::get<i>(m_distributions))>;
                using res_type = std::decay_t<decltype(std::declval<dist_type>()(crand{}, common::tagged_tuple_t<>{}))>;
                using full_type = common::tagged_tuple_cat<common::tagged_tuple_t<tags::start, times_t>, res_type>;
                using tag_type = typename dist_type::tags;
                full_type tt;
                common::get<tags::start>(tt) = t;
                call_distribution(std::get<i>(m_distributions), get_generator(has_randomizer<P>{}, *this), tt, tag_type{});
                P::net::node_emplace(tt);
            }

            //! @brief Calls a distribution, updating the tuple of results (empty case).
            template <typename D, typename G, typename T>
            inline void call_distribution(D&, G&&, T&, common::type_sequence<>) {}

            //! @brief Calls a distribution, updating the tuple of results (general case).
            template <typename D, typename G, typename T, typename S, typename... Ss>
            inline void call_distribution(D& d, G&& g, T& t, common::type_sequence<S, Ss...>) {
                common::get<S>(t) = common::get<S>(d)(g, t);
                call_distribution(d, g, t, common::type_sequence<Ss...>{});
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
