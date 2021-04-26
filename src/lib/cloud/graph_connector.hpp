// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file graph_connector.hpp
 * @brief Implementation of the `graph_connector` component handling message exchanges between nodes of a graph net.
 */

#ifndef FCPP_CLOUD_GRAPH_CONNECTOR_H_
#define FCPP_CLOUD_GRAPH_CONNECTOR_H_

#include <cmath>

#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lib/common/option.hpp"
#include "lib/common/serialize.hpp"
#include "lib/component/base.hpp"
#include "lib/data/field.hpp"
#include "lib/data/vec.hpp"
#include "lib/option/connect.hpp"
#include "lib/option/distribution.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a connector class.
    template <typename T>
    struct connector {};

    //! @brief Declaration tag associating to a delay generator for sending messages after rounds.
    template <typename T>
    struct delay {};

    //! @brief Declaration tag associating to the dimensionality of the space.
    template <size_t n>
    struct dimension;

    //! @brief Declaration flag associating to whether message sizes should be emulated.
    template <bool b>
    struct message_size {};

    //! @brief Declaration flag associating to whether parallelism is enabled.
    template <bool b>
    struct parallel;

    //! @brief Declaration flag associating to whether the topology of the graph is static.
    template <bool b>
    struct static_topology;

    //! @brief Node initialisation tag associating to communication power.
    struct connection_data {};

    //! @brief Initialisation tag associating to the time sensitivity, allowing indeterminacy below it.
    struct epsilon;

    //! @brief Net initialisation tag associating to communication radius.
    struct radius {};
}

/**
 * @brief Component handling message exchanges between nodes of a graph net.
 *
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 * Any \ref simulated_connector component cannot be a parent of a \ref timer otherwise round planning may block message exchange.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::connector defines the connector class (defaults to \ref connect::clique "connect::clique<dimension>").
 * - \ref tags::delay defines the delay generator for sending messages after rounds (defaults to zero delay through \ref distribution::constant_n "distribution::constant_n<times_t, 0>").
 * - \ref tags::dimension defines the dimensionality of the space (defaults to 2).
 *
 * <b>Declaration flags:</b>
 * - \ref tags::message_size defines whether message sizes should be emulated (defaults to false).
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::connection_data associates to communication power (defaults to `connector_type::data_type{}`).
 * - \ref tags::epsilon associates to the time sensitivity, allowing indeterminacy below it (defaults to \ref FCPP_TIME_EPSILON).
 *
 * Net initialisation tags (such as \ref tags::radius) are forwarded to connector classes.
 * Connector classes should have the following members (see \ref connect for a list of available ones):
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * using data_type = // type for connection power data on nodes
 * using position_type = vec<n>;
 * template <typename G, typename S, typename T> connector_type(G&& gen, const common::tagged_tuple<S,T>& tup);
 * real_t maximum_radius() const;
 * bool operator()(const data_type& data1, const position_type& position1, const data_type& data2, const position_type& position2) const;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class... Ts>
struct graph_connector {
    //! @brief Whether message sizes should be emulated.
    constexpr static bool message_size = common::option_flag<tags::message_size, false, Ts...>;

    //    constexpr static bool static_topology = common::option_flag<tags::static_topology, false, Ts...>;

    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief The dimensionality of the space.
    constexpr static size_t dimension = common::option_num<tags::dimension, 2, Ts...>;

    //! @brief Type for representing a position.
    using position_type = vec<dimension>;

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
        DECLARE_COMPONENT(connector);
        CHECK_COMPONENT(randomizer);
        CHECK_COMPONENT(identifier);

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            //@{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_delay(get_generator(has_randomizer<P>{}, *this),t), m_nbr_msg_size(0) {
                m_send = TIME_MAX;
                m_epsilon = common::get_or<tags::epsilon>(t, FCPP_TIME_EPSILON);
            }

            //! @brief Destructor leaving the corresponding cell.
            ~node() {
            }

            void connect(device_t i, typename F::node *n) {
                m_neighbours.insert(std::make_pair(i,n));
            }

            void disconnect(device_t i) {
                m_neighbours.erase(i);
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

            //! @brief Size of last message sent.
            size_t msg_size() const {
                return fcpp::details::self(m_nbr_msg_size.front(), P::node::uid);
            }

            //! @brief Sizes of messages received from neighbours.
            field<size_t> const& nbr_msg_size() const {
                return m_nbr_msg_size.front();
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
                    times_t t = next();
                    PROFILE_COUNT("connector/send");
                    m_send = TIME_MAX;
                    for (std::pair<device_t, typename F::node*> p : m_neighbours) {
                        typename F::node *n = p.second;
                        typename F::node::message_t m;
                        if (n != this) {
                            common::unlock_guard<parallel> u(P::node::mutex);
                            common::lock_guard<parallel> l(n->mutex);
                            n->receive(t, P::node::uid, P::node::as_final().send(t, n->uid, m));
                        } else n->receive(t, P::node::uid, P::node::as_final().send(t, n->uid, m));
                    }
                } else P::node::update();
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                m_send = t + m_delay(get_generator(has_randomizer<P>{}, *this));
                P::node::round_start(t);
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            inline void receive(times_t t, device_t d, const common::tagged_tuple<S,T>& m) {
                P::node::receive(t, d, m);
                receive_size(common::bool_pack<message_size>{}, d, m);
            }

          private: // implementation details
            //! @brief
            using neighbour_list = std::unordered_map<device_t, typename F::node*>;

            //! @brief Stores size of received message (disabled).
            template <typename S, typename T>
            void receive_size(common::bool_pack<false>, device_t, const common::tagged_tuple<S,T>&) {}
            //! @brief Stores size of received message.
            template <typename S, typename T>
            void receive_size(common::bool_pack<true>, device_t d, const common::tagged_tuple<S,T>& m) {
                common::osstream os;
                os << m;
                fcpp::details::self(m_nbr_msg_size.front(), d) = os.size();
            }

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

            //! @brief A generator for delays in sending messages.
            neighbour_list m_neighbours;

            //! @brief A generator for delays in sending messages.
            delay_type m_delay;

            //! @brief Time of the next send-message event (and epsilon time).
            times_t m_send, m_epsilon;

            //! @brief Sizes of messages received from neighbours.
            common::option<field<size_t>, message_size> m_nbr_msg_size;
        };

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) : P::net(t) {}

            //! @brief Destructor ensuring that nodes are deleted first.
            ~net() {
                maybe_clear(has_identifier<P>{}, *this);
            }

            // //! @brief Checks whether connection is possible.
            // template <typename G>
            // inline bool connection_success(G&& gen, const connection_data_type& data1, const position_type& position1, const connection_data_type& data2, const position_type& position2) const {
            //     return m_connector(gen, data1, position1, data2, position2);
            // }

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

            //! @brief Deletes all nodes if parent identifier.
            template <typename N>
            inline void maybe_clear(std::true_type, N& n) {
                return n.node_clear();
            }

            //! @brief Does nothing otherwise.
            template <typename N>
            inline void maybe_clear(std::false_type, N&) {}

            //! @brief The mutex regulating access to maps.
            common::mutex<parallel> m_mutex;
        };
    };
};


}


}

#endif // FCPP_CLOUD_GRAPH_CONNECTOR_H_
