// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file hardware_connector.hpp
 * @brief Implementation of the `hardware_connector` component handling message exchanges between nodes.
 */

#ifndef FCPP_DEPLOYMENT_HARDWARE_CONNECTOR_H_
#define FCPP_DEPLOYMENT_HARDWARE_CONNECTOR_H_

#include <cmath>

#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lib/common/serialize.hpp"
#include "lib/component/base.hpp"
#include "lib/data/field.hpp"
#include "lib/deployment/os.hpp"
#include "lib/option/distribution.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a connector class (defaults to \ref os::async_retry_network "os::async_retry_network<message_push>").
    template <typename T>
    struct connector;

    //! @brief Declaration tag associating to a delay generator for sending messages after rounds (defaults to zero delay through \ref distribution::constant_n "distribution::constant_n<times_t, 0>").
    template <typename T>
    struct delay;

    //! @brief Declaration flag associating to whether incoming messages are pushed or pulled (defaults to \ref FCPP_MESSAGE_PUSH).
    template <bool b>
    struct message_push;

    //! @brief Declaration flag associating to whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
    template <bool b>
    struct parallel;

    //! @brief Node initialisation tag associating to communication power (defaults to `connector_type::data_type{}`).
    struct connection_data;
}


/**
 * @brief Component handling exchanges of messages through an hardware interface.
 *
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 * Any \ref simulated_connector component cannot be a parent of a \ref timer otherwise round planning may block message exchange.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::connector defines the connector class (defaults to \ref os::async_retry_network "os::async_retry_network<message_push>").
 * - \ref tags::delay defines the delay generator for sending messages after rounds (defaults to zero delay through \ref distribution::constant_n "distribution::constant_n<times_t, 0>").
 *
 * <b>Declaration flags:</b>
 * - \ref tags::message_push defines whether incoming messages are pushed or pulled (defaults to \ref FCPP_MESSAGE_PUSH).
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::connection_data associates to communication power (defaults to `connector_type::data_type{}`).
 */
template <class... Ts>
struct hardware_connector {
    //! @brief Whether incoming messages are pushed or pulled.
    constexpr static bool message_push = common::option_flag<tags::message_push, FCPP_MESSAGE_PUSH, Ts...>;

    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief Delay generator for sending messages after rounds.
    using delay_type = common::option_type<tags::delay, distribution::constant_n<times_t, 0>, Ts...>;

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
        DECLARE_COMPONENT(connector);
        CHECK_COMPONENT(randomizer);
        CHECK_COMPONENT(calculus);
        //! @endcond

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            //! @brief Network interface class wrapper.
            using connector_type = typename common::option_type<tags::connector, os::async_retry_network<message_push>, Ts...>::template network<node>;

            //! @brief The type of settings data regulating connection.
            using connection_data_type = typename connector_type::data_type;

            //! @{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_delay(get_generator(has_randomizer<P>{}, *this),t), m_send(TIME_MAX), m_nbr_dist(INF), m_nbr_msg_size(0), m_network(*this, common::get_or<tags::connection_data>(t, connection_data_type{})) {}

            //! @brief Connector data.
            connection_data_type& connector_data() {
                return m_network.data();
            }

            //! @brief Connector data (const access).
            connection_data_type const& connector_data() const {
                return m_network.data();
            }

            //! @brief Returns the time of the next sending of messages.
            times_t send_time() const {
                return m_send;
            }

            //! @brief Plans the time of the next sending of messages (`TIME_MAX` to prevent sending).
            void send_time(times_t t) {
                m_send = t;
            }

            //! @brief Disable the next sending of messages (shorthand to `send_time(TIME_MAX)`).
            void disable_send() {
                m_send = TIME_MAX;
            }

            /**
             * @brief Returns next event to schedule for the node component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_send, P::node::next());
            }

            //! @brief Updates the internal status of node component.
            void update() {
                if (m_send < P::node::next()) {
                    PROFILE_COUNT("connector");
                    common::osstream os;
                    typename F::node::message_t m;
                    os << P::node::as_final().send(m_send, m);
                    fcpp::details::self(m_nbr_msg_size, P::node::uid) = os.size();
                    m_network.send(std::move(os));
                    P::node::as_final().receive(m_send, P::node::uid, m);
                    m_send = TIME_MAX;
                } else P::node::update();
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                m_send = t + m_delay(get_generator(has_randomizer<P>{}, *this), common::tagged_tuple_t<>{});
                if (not message_push) {
                    std::vector<message_type> mv = m_network.receive();
                    common::unlock_guard<parallel> l(P::node::mutex);
                    for (message_type& m : mv) receive(m);
                }
                P::node::round_start(t);
                maybe_align_inplace(m_nbr_dist, has_calculus<P>{});
                maybe_align_inplace(m_nbr_msg_size, has_calculus<P>{});
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            using P::node::receive;

            //! @brief Receives an incoming raw message.
            void receive(message_type& m) {
                PROFILE_COUNT("connector");
                common::lock_guard<parallel> l(P::node::mutex);
                fcpp::details::self(m_nbr_dist, m.device) = m.power;
                fcpp::details::self(m_nbr_msg_size, m.device) = m.content.size();
                common::isstream is(std::move(m.content));
                typename F::node::message_t mt;
#ifndef FCPP_DISABLE_EXCEPTIONS
                try {
#endif
                    is >> mt;
                    if (is.size() == 0)
                        P::node::as_final().receive(m.time, m.device, mt);
#ifndef FCPP_DISABLE_EXCEPTIONS
                } catch (common::format_error&) {}
#endif
            }

            //! @brief Perceived distances from neighbours.
            field<real_t> const& nbr_dist() const {
                return m_nbr_dist;
            }

            //! @brief Size of last message sent.
            size_t msg_size() const {
                return fcpp::details::self(m_nbr_msg_size, P::node::uid);
            }

            //! @brief Sizes of messages received from neighbours.
            field<size_t> const& nbr_msg_size() const {
                return m_nbr_msg_size;
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

            //! @brief Changes the domain of a field-like structure to match the domain of the neightbours ids.
            template <typename A>
            void maybe_align_inplace(field<A>& x, std::true_type) {
                align_inplace(x, std::vector<device_t>(fcpp::details::get_ids(P::node::nbr_uid())));
            }

            //! @brief Does not perform any alignment
            template <typename A>
            void maybe_align_inplace(field<A>&, std::false_type) {}

            //! @brief A generator for delays in sending messages.
            delay_type m_delay;

            //! @brief Time of the next send-message event.
            times_t m_send;

            //! @brief Perceived distances from neighbours.
            field<real_t> m_nbr_dist;

            //! @brief Sizes of messages received from neighbours.
            field<size_t> m_nbr_msg_size;

            //! @brief Backend regulating and performing the connection.
            connector_type m_network;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_DEPLOYMENT_HARDWARE_CONNECTOR_H_
