// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file spawner.hpp
 * @brief Implementation of the `spawner` component handling automated generation of nodes.
 */

#ifndef FCPP_SIMULATION_SPAWNER_H_
#define FCPP_SIMULATION_SPAWNER_H_

#include <type_traits>
#include <utility>

#include "lib/settings.hpp"
#include "lib/common/distribution.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"
#include "lib/component/timer.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


/**
 * @brief Component handling automated generation of nodes.
 *
 * Nodes generated receive all tags produced by generating distributions, and tag `start` associated to the creation time.
 * Multiple instances may coexist in a composition of components.
 * Requires a `identifier` parent component.
 * If a `randomizer` parent component is not found, `crand` is used as random generator.
 *
 * @param G     A sequence generator type scheduling spawning of nodes.
 * @param Ss    The sequence of tags and corresponding generating distributions (intertwined).
 */
template <typename G, typename... Ss>
struct spawner {
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
        //! @brief Marks that a spawner component is present.
        struct spawner_tag {};
        
        //! @brief Checks if T has a `randomizer_tag`.
        template <typename T, typename = int>
        struct has_rtag : std::false_type {};
        template <typename T>
        struct has_rtag<T, std::conditional_t<true,int,typename T::randomizer_tag>> : std::true_type {};
        
        //! @brief Checks if T has a `identifier_tag`.
        template <typename T, typename = int>
        struct has_itag : std::false_type {};
        template <typename T>
        struct has_itag<T, std::conditional_t<true,int,typename T::identifier_tag>> : std::true_type {};
        
        //! @brief Asserts that P has a `identifier_tag`.
        static_assert(has_itag<P>::value, "missing identifier parent for spawner component");

        //! @brief The local part of the component.
        using node = typename P::node;
        
        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) : P::net(t), m_schedule(get_generator(common::bool_pack<has_rtag<P>::value>(), *this),t), m_distributions(build_distributions(t, typename common::tagged_tuple_t<Ss...>::tags(), typename common::tagged_tuple_t<Ss...>::types())) {}

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
                    times_t t = m_schedule.next();
                    auto&& gen = get_generator(common::bool_pack<has_rtag<P>::value>(), *this);
                    m_schedule.step(gen);
                    P::net::node_emplace(push_time(m_distributions(gen), t));
                } else P::net::update();
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
            
            //! @brief Constructs the tuple of distributions, feeding the initialising tuple to all of them.
            template <typename S, typename T, typename... Ts, typename... Us>
            common::tagged_tuple_t<Ss...> build_distributions(const common::tagged_tuple<S,T>& t, common::type_sequence<Ts...>, common::type_sequence<Us...>) {
                return common::make_tagged_tuple<Ts...>(Us{get_generator(common::bool_pack<has_rtag<P>::value>(), *this),t}...);
            }
            
            //! @brief Adds a `start` time to a given tagged tuple.
            template <typename S, typename T>
            auto push_time(const common::tagged_tuple<S,T>& tup, times_t t) {
                using tt_type = typename common::tagged_tuple<S,T>::template push_back<tags::start, times_t>;
                tt_type tt(tup);
                common::get<tags::start>(tt) = t;
                return tt;
            }

            //! @brief The scheduling of spawning events.
            G m_schedule;
            
            //! @brief The generator tuple.
            common::tagged_tuple_t<Ss...> m_distributions;
        };
    };
};


}


}

#endif // FCPP_SIMULATION_SPAWNER_H_
