// Copyright © 2026 Giorgio Audrito. All Rights Reserved.

/**
 * @file graph_connector.hpp
 * @brief Implementation of the `graph_connector` component handling message exchanges between nodes of a graph-based network.
 */

#ifndef FCPP_CLOUD_GRAPH_CONNECTOR_H_
#define FCPP_CLOUD_GRAPH_CONNECTOR_H_

#include <cmath>

#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include "lib/common/algorithm.hpp"
#include "lib/common/mpi.hpp"
#include "lib/common/option.hpp"
#include "lib/common/serialize.hpp"
#include "lib/common/utilities.hpp"
#include "lib/component/base.hpp"
#include "lib/data/field.hpp"
#include "lib/internal/twin.hpp"
#include "lib/option/distribution.hpp"
#include "lib/option/functor.hpp"
#include "lib/option/sequence.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a delay generator for sending messages through MPI (defaults to `sequence::never`).
    template <typename T>
    struct mpi_send_schedule {};

    //! @brief Declaration tag associating to a delay generator for receiving messages through MPI (defaults to `sequence::never`, should be more than twice as frequent than \ref tags::mpi_send_schedule to prevent message accumulation).
    template <typename T>
    struct mpi_recv_schedule {};

    //! @brief Declaration tag associating to a node splitting functor (defaults to `functor::mod`, is not randomized).
    template <typename A>
    struct node_splitting;

    //! @brief Declaration tag associating to a delay generator for sending messages after rounds (defaults to zero delay through \ref distribution::constant_n "distribution::constant_n<times_t, 0>").
    template <typename T>
    struct send_delay;

    //! @brief Declaration flag associating to whether message sizes should be emulated (defaults to false).
    template <bool b>
    struct message_size;

    //! @brief Declaration flag associating to whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
    template <bool b>
    struct parallel;

    //! @brief Declaration flag associating to whether the neighbour relation is symmetric (defaults to true).
    template <bool b>
    struct symmetric;

    //! @brief Net initialisation tag associating to the number of threads that can be created.
    struct threads;

    //! @brief Net initialisation tag associated to the total number of existing MPI processes (defaults to 1).
    struct mpi_procs {};

    //! @brief Node initialisation tag associating to a `device_t` unique identifier.
    struct uid;
}


//! @brief Enumeration type for kinds of connection and disconnection requests.
enum class request_kind : int8_t {
    CONNECT = -1,
    NONE = 0,
    DISCONNECT = 1,
    BIDISCONNECT = 2
};

//! @brief Converts a request_kind to its string representation.
std::string to_string(request_kind r);


/**
 * @brief Component handling message exchanges between nodes of a graph-based network.
 *
 * Requires a \ref identifier parent component.
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::mpi_send_schedule defines the delay generator for sending messages through MPI (defaults to `sequence::never`).
 * - \ref tags::mpi_recv_schedule defines the delay generator for receiving messages through MPI (defaults to `sequence::never`, should be more than twice as frequent than \ref tags::mpi_send_schedule to prevent message accumulation).
 * - \ref tags::node_splitting defines the node splitting functor (defaults to `functor::mod`, is not randomized).
 * - \ref tags::send_delay defines the delay generator for sending messages after rounds (defaults to zero delay through \ref distribution::constant_n "distribution::constant_n<times_t, 0>").
 *
 * <b>Declaration flags:</b>
 * - \ref tags::message_size defines whether message sizes should be emulated (defaults to false).
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 * - \ref tags::symmetric defines whether the neighbour relation is symmetric (defaults to true).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::threads defines the number of threads that can be created (defaults to \ref FCPP_THREADS).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::uid associates to a `device_t` unique identifier (required).
 */
template <class... Ts>
struct graph_connector {
    //! @brief Whether message sizes should be emulated.
    constexpr static bool message_size = common::option_flag<tags::message_size, false, Ts...>;

    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief Whether the neighbour relation is symmetric (defaults to true).
    constexpr static bool symmetric = common::option_flag<tags::symmetric, true, Ts...>;

    //! @brief Delay generator for sending messages after rounds.
    using delay_type = common::option_type<tags::send_delay, distribution::constant_n<times_t, 0>, Ts...>;

    //! @brief The type of settings data regulating connection.
    using connection_data_type = common::tagged_tuple_t<>;

    //! @brief The node splitting functor (defaults to `functor::mod`, is not randomized).
    using node_splitting_type = common::option_type<tags::node_splitting, functor::mod<tags::uid, tags::mpi_procs, int>, Ts...>;

    //! @brief Delay generator for sending messages through MPI (defaults to `sequence::never`).
    using mpi_send_schedule_type = common::option_type<tags::mpi_send_schedule, sequence::never, Ts...>;

    //! @brief Delay generator for receiving messages through MPI (defaults to `sequence::never`, should be more than twice as frequent than \ref tags::mpi_send_schedule to prevent message accumulation).
    using mpi_recv_schedule_type = common::option_type<tags::mpi_recv_schedule, sequence::never, Ts...>;

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
        REQUIRE_COMPONENT(connector,identifier);
        CHECK_COMPONENT(randomizer);
        //! @endcond

#ifdef FCPP_MPI
        //! @brief Reference handling both local and remote nodes.
        class node_accessor {
            //! @brief Type of a pointer to a node.
            using node_pointer = typename F::node*;

            //! @brief Type of a node numerical address.
            using node_address = std::pair<device_t, int>;

          public:
            //! @brief Constructor.
            node_accessor(typename F::net& net, device_t i) {
                int rank = net.mpi_rank(i);
                if (rank == net.mpi_rank()) {
                    // local node reference
                    m_ref = const_cast<node_pointer>(&net.node_at(i));
                } else {
                    // remote node reference
                    m_ref = node_address(i, rank);
                }
            }

            //! @brief Message receipt method that handles the distinction between local and remote neighbour
            template <typename S, typename T>
            inline void receive(typename F::net& net, times_t t, device_t d, common::tagged_tuple<S,T> const& m) const {
                if (auto* n = std::get_if<node_pointer>(&m_ref)) {
                    // local node reference
                    common::lock_guard<parallel> l((*n)->mutex);
                    (*n)->receive(t, d, m);
                }
                if (auto* n = std::get_if<node_address>(&m_ref)) {
                    // remote node reference
                    net.mpi_receive(n->second, n->first, t, d, m);
                }
            }

            //! @brief Back-connection request to a given node.
            inline void connect_from(typename F::net& net, device_t i) const {
                if (auto* n = std::get_if<node_pointer>(&m_ref)) {
                    // local node reference
                    (*n)->connect_from(i);
                }
                if (auto* n = std::get_if<node_address>(&m_ref)) {
                    // remote node reference
                    net.mpi_conn_request(n->second, n->first, i, request_kind::CONNECT);
                }
            }

            //! @brief Back-disconnection from a given node.
            inline void disconnect_from(typename F::net& net, device_t i) const {
                if (auto* n = std::get_if<node_pointer>(&m_ref)) {
                    // local node reference
                    (*n)->disconnect_from(i);
                }
                if (auto* n = std::get_if<node_address>(&m_ref)) {
                    // remote node reference
                    net.mpi_conn_request(n->second, n->first, i, request_kind::DISCONNECT);
                }
            }

            //! @brief Total disconnection from a given node.
            inline void bidisconnect_from(typename F::net& net, device_t i) const {
                if (auto* n = std::get_if<node_pointer>(&m_ref)) {
                    // local node reference
                    (*n)->bidisconnect_from(i);
                }
                if (auto* n = std::get_if<node_address>(&m_ref)) {
                    // remote node reference
                    net.mpi_conn_request(n->second, n->first, i, request_kind::BIDISCONNECT);
                }
            }

          private:
            //! @brief The reference to the node.
            std::variant<node_address, node_pointer> m_ref;
        };
#else
        //! @brief Reference handling local nodes.
        class node_accessor {
            //! @brief Type of a pointer to a node.
            using node_pointer = typename F::node*;

          public:
            //! @brief Constructor.
            node_accessor(typename F::net& net, device_t i) : m_ref(const_cast<typename F::node*>(&net.node_at(i))) {}

            //! @brief Message receipt method that handles the distinction between local and remote neighbour
            template <typename S, typename T>
            void receive(typename F::net&, times_t t, device_t d, common::tagged_tuple<S,T> const& m) const {
                common::lock_guard<parallel> l(m_ref->mutex);
                m_ref->receive(t, d, m);
            }

            //! @brief Local back-connection to a given node.
            void connect_from(typename F::net&, device_t i) const {
                m_ref->connect_from(i);
            }

            //! @brief Back-disconnection from a given node.
            void disconnect_from(typename F::net&, device_t i) const {
                m_ref->disconnect_from(i);
            }

            //! @brief Inverse back-disconnection from a given node.
            void bidisconnect_from(typename F::net&, device_t i) const {
                m_ref->bidisconnect_from(i);
            }

          private:
            //! @brief The reference to the node.
            node_pointer m_ref;
        };
#endif

        //! @brief The local part of the component.
        class node : public P::node {
            //! @brief Stores the list of neighbours in the graph.
            using neighbour_list = std::unordered_map<device_t, node_accessor>;

          public: // visible by net objects and the main program
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_delay(get_generator(has_randomizer<P>{}, *this),t), m_send(TIME_MAX), m_nbr_msg_size(0) {}

            //! @brief Destructor ensuring deadlock-free mutual disconnection.
            ~node() {
                neighbour_list nlist;
                {
                    common::lock_guard<parallel> l(P::node::mutex);
                    std::swap(nlist, m_neighbours.first());
                    if (not symmetric) {
                        nlist.insert(m_neighbours.second().begin(), m_neighbours.second().end());
                        m_neighbours.second().clear();
                    }
                }
                for (auto const& n : nlist) {
                    n.second.bidisconnect_from(P::node::net, P::node::uid);
                }
            }

            //! @brief Adds given device to neighbours (returns true on succeed).
            bool connect(device_t i) {
                if (P::node::uid == i or m_neighbours.first().count(i) > 0) return false;
                node_accessor n{P::node::net, i};
                m_neighbours.first().emplace(i, n);
                common::unlock_guard<parallel> u(P::node::mutex);
                n.connect_from(P::node::net, P::node::uid);
                return true;
            }

            //! @brief Removes given device from neighbours (returns true on succeed).
            bool disconnect(device_t i) {
                if (P::node::uid == i or m_neighbours.first().count(i) == 0) return false;
                node_accessor n = m_neighbours.first().at(i);
                m_neighbours.first().erase(i);
                common::unlock_guard<parallel> u(P::node::mutex);
                n.disconnect_from(P::node::net, P::node::uid);
                return true;
            }

            //! @brief Removes given device from neighbours bidirectionally (returns true on succeed).
            bool bidisconnect(device_t i) {
                if (P::node::uid == i or m_neighbours.first().count(i) + m_neighbours.second().count(i) == 0) return false;
                node_accessor n = m_neighbours.first().count(i) ? m_neighbours.first().at(i) : m_neighbours.second().at(i);
                m_neighbours.first().erase(i);
                m_neighbours.second().erase(i);
                common::unlock_guard<parallel> u(P::node::mutex);
                n.bidisconnect_from(P::node::net, P::node::uid);
                return true;
            }

            //! @brief Registers a connection from another node (for internal use).
            void connect_from(device_t i) {
                if (P::node::uid == i or m_neighbours.second().count(i) > 0) return;
                common::lock_guard<parallel> l(P::node::mutex);
                m_neighbours.second().emplace(i, node_accessor{P::node::net, i});
            }

            //! @brief Registers a disconnection from another node (for internal use).
            void disconnect_from(device_t i) {
                if (P::node::uid == i or m_neighbours.second().count(i) == 0) return;
                common::lock_guard<parallel> l(P::node::mutex);
                m_neighbours.second().erase(i);
            }

            //! @brief Registers a bidisconnection from another node (for internal use).
            void bidisconnect_from(device_t i) {
                if (P::node::uid == i or m_neighbours.first().count(i) + m_neighbours.second().count(i) == 0) return;
                common::lock_guard<parallel> l(P::node::mutex);
                m_neighbours.first().erase(i);
                m_neighbours.second().erase(i);
            }

            //! @brief Disconnects from every neighbour (should only be used on all neighbours at once).
            void global_disconnect() {
                common::lock_guard<parallel> l(P::node::mutex);
                m_neighbours.first().clear();
                if (not symmetric) m_neighbours.second().clear();
            }

            //! @brief Checks whether a given device identifier is within neighbours.
            bool connected(device_t i) const {
                return m_neighbours.first().count(i);
            }

            //! @brief Connector data.
            connection_data_type& connector_data() {
                return m_data;
            }

            //! @brief Connector data (const access).
            connection_data_type const& connector_data() const {
                return m_data;
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
                times_t t = m_send;
                times_t pt = P::node::next();
                if (t < pt) {
                    PROFILE_COUNT("connector");
                    {
                        PROFILE_COUNT("connector/send");
                        m_send = TIME_MAX;
                        typename F::node::message_t m;
                        P::node::as_final().send(t, m);
                        P::node::as_final().receive(t, P::node::uid, m);
                        auto copy = m_neighbours.first();
                        common::unlock_guard<parallel> u(P::node::mutex);
                        for (auto const& p : copy) {
                            p.second.receive(P::node::net, t, P::node::uid, m);
                        }
                    }
                } else P::node::update();
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                m_send = t + m_delay(get_generator(has_randomizer<P>{}, *this), common::tagged_tuple_t<>{});
                P::node::round_start(t);
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            inline void receive(times_t t, device_t d, common::tagged_tuple<S,T> const& m) {
                P::node::receive(t, d, m);
                receive_size(common::number_sequence<message_size>{}, d, m);
            }

          private: // implementation details
            //! @brief Stores size of received message (disabled).
            template <typename S, typename T>
            void receive_size(common::number_sequence<false>, device_t, common::tagged_tuple<S,T> const&) {}
            //! @brief Stores size of received message.
            template <typename S, typename T>
            void receive_size(common::number_sequence<true>, device_t d, common::tagged_tuple<S,T> const& m) {
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

            //! @brief A list of neighbours.
            internal::twin<neighbour_list, symmetric> m_neighbours;

            //! @brief A generator for delays in sending messages.
            delay_type m_delay;

            //! @brief Time of the next send-message event.
            times_t m_send;

            //! @brief Sizes of messages received from neighbours.
            common::option<field<size_t>, message_size> m_nbr_msg_size;

            //! @brief Data regulating the connection.
            connection_data_type m_data;
        };

#ifdef FCPP_MPI
        //! @brief Structure representing messages for a single node.
        struct node_message_type {
            //! @brief Map associating a device UID with a connection (+1) or disconnection (-1) request from it.
            std::unordered_map<device_t, request_kind> conn_requests;
            //! @brief Map associating a sender UID to the most recent timestamped message received from it.
            std::unordered_map<device_t, std::pair<times_t, typename F::node::message_t>> messages;

            //! @brief Serialises the content from/to a given input/output stream.
            template <typename S>
            S& serialize(S& s) {
                return s & conn_requests & messages;
            }

            //! @brief Serialises the content from/to a given input/output stream (const overload).
            template <typename S>
            S& serialize(S& s) const {
                return s << conn_requests << messages;
            }
        };

        //! @brief Map associating a receiver UID to the structure of messages for it.
        using mpi_message_type = std::unordered_map<device_t, node_message_type>;

        //! @brief The global part of the component.
        class net : public P::net {
            friend class node_accessor;
#else
        //! @brief The global part of the component.
        class net : public P::net {
            friend class node_accessor;
#endif

          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) : 
                P::net(t), m_threads(common::get_or<tags::threads>(t, FCPP_THREADS)), m_mpi(1, false)
#ifdef FCPP_MPI
                , m_send_schedule(get_generator(has_randomizer<P>{}, *this), t)
                , m_recv_schedule(get_generator(has_randomizer<P>{}, *this), t)
                , m_node_splitter(norand{}, t)
#endif
                {}

            //! @brief Destructor ensuring that edges are deleted first.
            ~net() {
                auto n_beg = P::net::node_begin();
                auto n_end = P::net::node_end();
                common::parallel_for(common::tags::general_execution<parallel>(m_threads), n_end-n_beg, [&] (size_t i, size_t) {
                    n_beg[i].second.global_disconnect();
                });
#if defined(FCPP_MPI) && !defined(NDEBUG)
                if (m_mpi_comm_map.size()) {
                    std::cerr << (std::to_string(m_mpi_comm_map.size()) + " MPI messages waiting to be sent:\n") << std::flush;
                    for (auto const& process_messages : m_mpi_comm_map) {
                        for (auto const& node_messages : process_messages.second) {
                            for (auto const& req : node_messages.second.conn_requests) {
                                std::cerr << ("\t" + to_string(req)
                                    + " request sent to node" + std::to_string(node_messages.first)
                                    + " of rank " + std::to_string(process_messages.first)
                                    + " from node " + std::to_string(req.first) + "\n") << std::flush;
                            }
                            for (auto const& msg : node_messages.second.messages) {
                                std::cerr << ("\tmessage sent to node " + std::to_string(node_messages.first)
                                    + " of rank " + std::to_string(process_messages.first)
                                    + " from node " + std::to_string(msg.first)
                                    + " at time " + std::to_string(msg.second.first) + "\n") << std::flush;
                            }
                        }
                    }
                }
#endif
            }

#ifdef FCPP_MPI
            /**
             * @brief Returns next event to schedule for the net component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(std::min(m_send_schedule.next(), m_recv_schedule.next()), P::net::next());
            }

            //! @brief Updates the internal status of net component.
            void update() {
                times_t t_send = m_send_schedule.next();
                times_t t_recv = m_recv_schedule.next();
                times_t pt = P::net::next();                    

                if (t_send < pt and t_send <= t_recv) {
                    // sending our comm_map to other MPI nodes
                    m_send_schedule.step(get_generator(has_randomizer<P>{}, *this), fcpp::common::make_tagged_tuple<>());
                    common::lock_guard<parallel> l(m_comm_map_mutex);
                    for (auto const& process_messages : m_mpi_comm_map) {
                        //std::cout << "Sending remote message to process: " << process_messages.first << std::endl;
                        common::osstream os;
                        os << process_messages.second;
                        m_mpi.isend(0, {process_messages.first, std::move(os.data())});
                    }
                    m_mpi_comm_map.clear();
                } else if (t_recv < pt) {
                    // processing received comm_maps from other MPI nodes
                    m_recv_schedule.step(get_generator(has_randomizer<P>{}, *this), fcpp::common::make_tagged_tuple<>());
                    for (int rank = 0; rank < m_mpi.n_procs; ++rank) if (rank != m_mpi.rank) {
                        common::option<common::mpi_message> m = m_mpi.irecv(0, rank);
                        if (m.empty()) continue;
                        common::isstream is(std::move(m.front().data));
                        mpi_message_type comm_map;
                        is >> comm_map;
                        for (auto const& node_messages : comm_map) {
                            //std::cout << "Processing messages SENT to node " << node_messages.first << std::endl;
                            if (not P::net::node_count(node_messages.first)) {
                                // send back a bidisconnection request to each node communicating with a non-existing node
                                for (auto const& msg : node_messages.second.messages) {
                                    mpi_conn_request(m.front().rank, msg.first, node_messages.first, request_kind::BIDISCONNECT);
                                }
                                for (auto const& msg : node_messages.second.conn_requests) {
                                    mpi_conn_request(m.front().rank, msg.first, node_messages.first, request_kind::BIDISCONNECT);
                                }
                                continue;
                            }
                            node_accessor n{P::net::as_final(), node_messages.first};
                            for (auto const& msg : node_messages.second.messages) {
                                //std::cout << "Message RECEIVED FROM " << msg.first << std::endl;
                                //std::cout << "Message TIMESTAMP " << msg.second.first << std::endl;
                                n.receive(P::net::as_final(), msg.second.first, msg.first, msg.second.second);
                            }
                            for (auto const& msg : node_messages.second.conn_requests) {
                                switch (msg.second) {
                                    case request_kind::CONNECT:
                                        n.connect_from(P::net::as_final(), msg.first);
                                        break;
                                    case request_kind::DISCONNECT:
                                        n.disconnect_from(P::net::as_final(), msg.first);
                                        break;
                                    case request_kind::BIDISCONNECT:
                                        n.bidisconnect_from(P::net::as_final(), msg.first);
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                } else P::net::update();
            }
#endif

            //! @brief The current MPI process rank.
            inline int mpi_rank() const {
                return m_mpi.rank;
            }

            //! @brief Computes the MPI process rank for a given node.
            inline int mpi_rank(device_t i) {
#ifdef FCPP_MPI
                return m_node_splitter(norand{}, common::make_tagged_tuple<tags::uid, tags::mpi_procs>(i, m_mpi.n_procs));
#else
                return m_mpi.rank;
#endif
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

#ifdef FCPP_MPI
            //! @brief Receives a remote message to be sent through MPI.
            inline void mpi_receive(int receiver_rank, device_t receiver_uid, times_t timestamp, device_t sender_uid, typename F::node::message_t const& msg) {
                common::lock_guard<parallel> l(m_comm_map_mutex);
                m_mpi_comm_map[receiver_rank][receiver_uid].messages[sender_uid] = std::make_pair(timestamp, msg);
            }

            //! @brief Receives a remote connection or disconnection request to be sent through MPI.
            inline void mpi_conn_request(int receiver_rank, device_t receiver_uid, device_t sender_uid, request_kind req) {
                common::lock_guard<parallel> l(m_comm_map_mutex);
                auto& conn_requests = m_mpi_comm_map[receiver_rank][receiver_uid].conn_requests;
                if (conn_requests.count(sender_uid)) {
                    if (static_cast<int8_t>(req) * static_cast<int8_t>(conn_requests[sender_uid]) == -1) {
                        // connect-disconnect or disconnect-connect, same as doing nothing
                        conn_requests.erase(sender_uid);
                        return;
                    }
                }
                // otherwise, the last operation wins
                // (since bidisconnect is only issued on destruction,
                // we can assume it can only be the last)
                conn_requests[sender_uid] = req;
            }
#endif

            //! @brief The number of threads to be used.
            size_t const m_threads;

            //! @brief The MPI manager.
            common::mpi_manager m_mpi;

#ifdef FCPP_MPI
            //! @brief Sequence of MPI sending events.
            mpi_send_schedule_type m_send_schedule;

            //! @brief Sequence of MPI receiving events.
            mpi_recv_schedule_type m_recv_schedule;

            //! @brief Functor to compute the MPI process associated to a node.
            node_splitting_type m_node_splitter;

            //! @brief Map associating the MPI process rank to the map of messages for its nodes.
            std::unordered_map<int, mpi_message_type> m_mpi_comm_map;

            //! @brief Mutex to manage parallel access to the communication map.
            common::mutex<parallel> m_comm_map_mutex;
#endif
        };
    };
};


} // namespace component


} // namespace fcpp

#endif // FCPP_CLOUD_GRAPH_CONNECTOR_H_
