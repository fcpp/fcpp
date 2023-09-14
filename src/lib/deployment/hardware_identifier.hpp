// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file hardware_identifier.hpp
 * @brief Implementation of the `hardware_identifier` component handling node creation and indexing for deployed systems.
 */

#ifndef FCPP_DEPLOYMENT_HARDWARE_IDENTIFIER_H_
#define FCPP_DEPLOYMENT_HARDWARE_IDENTIFIER_H_

#include <cassert>
#include <type_traits>

#include "lib/component/base.hpp"
#include "lib/deployment/os.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration flag associating to whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
    template <bool b>
    struct parallel;

    //! @brief Node initialisation tag associating to a starting time of execution (defaults to \ref TIME_MAX).
    struct start;

    //! @brief Node initialisation tag associating to a `device_t` unique identifier (required).
    struct uid;
}


/**
 * @brief Component handling node creation and indexing.
 *
 * The \ref timer component cannot be a parent of a \ref hardware_identifier to preserve node scheduling.
 *
 * Creates a single node, with `uid` and `start` determined through system calls.
 * It also substitutes the `spawner` component.
 *
 * <b>Declaration flags:</b>
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 */
template <class... Ts>
struct hardware_identifier {
    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

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
        DECLARE_COMPONENT(identifier);
        DECLARE_COMPONENT(spawner);
        AVOID_COMPONENT(identifier,timer);
        //! @endcond

        //! @brief The local part of the component.
        using node = typename P::node;

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief The type of nodes.
            using node_type = typename F::node;

            //! @brief The type of node locks.
            using lock_type = common::unique_lock<parallel>;

            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) : P::net(t), m_node(P::net::as_final(), push_start_uid(t)) {}

            /**
             * @brief Returns next event to schedule for the net component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_node.next(), P::net::next());
            }

            //! @brief Updates the internal status of net component.
            void update() {
                if (m_node.next() < P::net::next()) {
                    common::lock_guard<parallel> l(m_node.mutex);
                    m_node.update();
                }
                else P::net::update();
            }

            //! @brief Returns the total number of nodes.
            static constexpr size_t node_size() {
                return 1;
            }

            //! @brief Returns whether a node with a certain device identifier exists.
            inline size_t node_count(device_t uid) const {
                return m_node.uid == uid;
            }

            //! @brief Const access to the node with a given device identifier.
            inline node_type const& node_at(device_t uid) const {
                assert(m_node.uid == uid);
                return m_node;
            }

            //! @brief Access to the node with a given device identifier (given a lock for the node's mutex).
            node_type& node_at(device_t uid, lock_type& l) {
                assert(m_node.uid == uid);
                l = lock_type(m_node.mutex);
                return m_node;
            }

          private: // implementation details
            //! @brief Adds a tagged value to a tagged tuple if not already present.
            //! @{
            template <typename S, typename T, typename Ss, typename Us>
            auto maybe_push(common::tagged_tuple<Ss,Us> const& t, T const&, common::type_sequence<S>) {
                return t;
            }
            template <typename S, typename T, typename Ss, typename Us>
            auto maybe_push(common::tagged_tuple<Ss,Us> const& t, T const& x, common::type_sequence<>) {
                using tt_type = typename common::tagged_tuple<Ss,Us>::template push_back<S, T>;
                tt_type tt(t);
                common::get<S>(tt) = x;
                return tt;
            }
            template <typename S, typename T, typename Ss, typename Us>
            inline auto maybe_push(common::tagged_tuple<Ss,Us> const& t, T const& x) {
                return maybe_push<S>(t, x, typename Ss::template intersect<S>{});
            }
            //! @}

            //! @brief Adds a `start` time and `uid` to a given tagged tuple.
            template <typename Ss, typename Us>
            auto push_start_uid(common::tagged_tuple<Ss,Us> const& t) {
                return maybe_push<tags::uid>(maybe_push<tags::start>(t, times_t{0}), os::uid());
            }

            //! @brief The node.
            node_type m_node;
        };
    };
};


}


}

#endif // FCPP_DEPLOYMENT_HARDWARE_IDENTIFIER_H_
