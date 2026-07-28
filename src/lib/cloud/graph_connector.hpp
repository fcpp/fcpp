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
#include "lib/common/option.hpp"
#include "lib/common/serialize.hpp"
#include "lib/component/base.hpp"
#include "lib/data/field.hpp"
#include "lib/internal/twin.hpp"
#include "lib/option/distribution.hpp"
#include "lib/option/functor.hpp"
#include "lib/option/sequence.hpp"
#include "lib/simulation/batch.hpp" // TODO: move only the MPI-related part to common/algorithm.hpp


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

    //! @brief Declaration tag associating to a delay generator for receiving messages through MPI (defaults to `sequence::never`).
    template <typename T>
    struct mpi_recv_schedule {};

    //! @brief Declaration tag associating to a node splitting functor (defaults to `functor::mod`, should not be randomized).
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

    //! @brief Declaration flag associating to whether the topology of the graph is static (for future use).
    template <bool b>
    struct static_topology;

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
 * - \ref tags::mpi_recv_schedule defines the delay generator for receiving messages through MPI (defaults to `sequence::never`).
 * - \ref tags::node_splitting defines the node splitting functor (defaults to `functor::mod`, should not be randomized).
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

    //! @brief The node splitting functor (defaults to `functor::mod`, should not be randomized).
    using node_splitting_type = common::option_type<tags::node_splitting, functor::mod<tags::uid, tags::mpi_procs, int>, Ts...>;

    //! @brief Delay generator for sending messages through MPI (defaults to `sequence::never`).
    using mpi_send_schedule_type = common::option_type<tags::mpi_send_schedule, sequence::never, Ts...>;

    //! @brief Delay generator for receiving messages through MPI (defaults to `sequence::never`).
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
            //! @brief Direct local-only constructor.
            node_accessor(node_pointer p) : m_ref(p) {}

            //! @brief General constructor.
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
                if (auto* n = std::get_if<node_address>(&m_ref)) {
                    // remote node reference
                    net.mpi_receive(n->second, n->first, t, d, m);
                }
                if (auto* n = std::get_if<node_pointer>(&m_ref)) {
                    // local node reference
                    common::lock_guard<parallel> l((*n)->mutex);
                    (*n)->receive(t, d, m);
                }
            }

            //! @brief Back-connection request to a given node.
            inline void connect(typename F::net& net, device_t i, node_pointer p) const {
                if (auto* n = std::get_if<node_pointer>(&m_ref)) {
                    // local node reference
                    common::lock_guard<parallel> l((*n)->mutex);
                    (*n)->m_neighbours.second().emplace(i, p);
                }
                if (auto* n = std::get_if<node_address>(&m_ref)) {
                    // remote node reference
                    net.mpi_conn_request(n->second, n->first, i, request_kind::CONNECT);
                }
            }

            //! @brief Back-disconnection from a given node.
            inline void disconnect(typename F::net& net, device_t i) const {
                if (auto* n = std::get_if<node_address>(&m_ref)) {
                    // remote node reference
                    net.mpi_conn_request(n->second, n->first, i, request_kind::DISCONNECT);
                }
                if (auto* n = std::get_if<node_pointer>(&m_ref)) {
                    // local node reference
                    common::lock_guard<parallel> l((*n)->mutex);
                    (*n)->m_neighbours.second().erase(i);
                }
            }

            //! @brief Total disconnection from a given node.
            inline void bidisconnect(typename F::net& net, device_t i) const {
                if (auto* n = std::get_if<node_address>(&m_ref)) {
                    // remote node reference
                    net.mpi_conn_request(n->second, n->first, i, request_kind::BIDISCONNECT);
                }
                if (auto* n = std::get_if<node_pointer>(&m_ref)) {
                    // local node reference
                    common::lock_guard<parallel> l((*n)->mutex);
                    (*n)->m_neighbours.first().erase(i);
                    (*n)->m_neighbours.second().erase(i);
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
            //! @brief Direct local-only constructor.
            node_accessor(node_pointer p) : m_ref(p) {}

            //! @brief General constructor.
            node_accessor(typename F::net& net, device_t i) : m_ref(const_cast<typename F::node*>(&net.node_at(i))) {}

            //! @brief Message receipt method that handles the distinction between local and remote neighbour
            template <typename S, typename T>
            void receive(typename F::net&, times_t t, device_t d, common::tagged_tuple<S,T> const& m) const {
                common::lock_guard<parallel> l(m_ref->mutex);
                m_ref->receive(t, d, m);
            }

            //! @brief Local back-connection to a given node.
            void connect(typename F::net&, device_t i, node_pointer p) const {
                common::lock_guard<parallel> l(m_ref->mutex);
                m_ref->m_neighbours.second().emplace(i, p);
            }

            //! @brief Back-disconnection from a given node.
            void disconnect(typename F::net&, device_t i) const {
                common::lock_guard<parallel> l(m_ref->mutex);
                m_ref->m_neighbours.second().erase(i);
            }

            //! @brief Inverse back-disconnection from a given node.
            void bidisconnect(typename F::net&, device_t i) const {
                common::lock_guard<parallel> l(m_ref->mutex);
                m_ref->m_neighbours.first().erase(i);
                m_ref->m_neighbours.second().erase(i);
            }

          private:
            //! @brief The reference to the node.
            node_pointer m_ref;
        };
#endif

        //! @brief The local part of the component.
        class node : public P::node {
            friend class node_accessor;

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
                    n.second.bidisconnect(P::node::net, P::node::uid);
                }
            }

            //! @brief Adds given device to neighbours (returns true on succeed).
            bool connect(device_t i) {
                if (P::node::uid == i or m_neighbours.first().count(i) > 0) return false;
                node_accessor n{P::node::net, i};
                m_neighbours.first().emplace(i, n);
                common::unlock_guard<parallel> u(P::node::mutex);
                n.connect(P::node::net, P::node::uid, &P::node::as_final());
                return true;
            }

            //! @brief Removes given device from neighbours (returns true on succeed).
            bool disconnect(device_t i) {
                if (P::node::uid == i or m_neighbours.first().count(i) == 0) return false;
                node_accessor n = m_neighbours.first().at(i);
                m_neighbours.first().erase(i);
                common::unlock_guard<parallel> u(P::node::mutex);
                n.disconnect(P::node::net, P::node::uid);
                return true;
            }

            //! @brief Disconnects from every neighbour (should only be used on all neighbours at once).
            void global_disconnect() {
                return; // TODO: handle this case
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

        //! @brief The global part of the component.
        class net : public P::net {
            friend class node_accessor;

#ifdef FCPP_MPI
            //! @brief Structure representing messages for a single node.
            struct node_message_type {
                //! @brief Map associating a device UID with a connection (+1) or disconnection (-1) request from it.
                std::unordered_map<device_t, request_kind> conn_requests;
                //! @brief Map associating a sender UID to the most recent timestamped message received from it.
                std::unordered_map<device_t, std::pair<times_t, typename F::node::message_t>> messages;
            };

            //! @brief Map associating a receiver UID to the structure of messages for it.
            using mpi_message_type = std::unordered_map<device_t, node_message_type>;
#endif

          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) : 
                P::net(t),
#ifdef FCPP_MPI
                m_send_schedule(get_generator(has_randomizer<P>{}, *this), t),
                m_recv_schedule(get_generator(has_randomizer<P>{}, *this), t),
                m_node_splitter(get_generator(has_randomizer<P>{}, *this), t),
#endif
                m_threads(common::get_or<tags::threads>(t, FCPP_THREADS)) {
                batch::mpi_init(m_mpi_rank, m_mpi_procs);
            }

            //! @brief Destructor ensuring that nodes are deleted first.
            ~net() {
                auto n_beg = P::net::node_begin();
                auto n_end = P::net::node_end();
                common::parallel_for(common::tags::general_execution<parallel>(m_threads), n_end-n_beg, [&] (size_t i, size_t) {
                    n_beg[i].second.global_disconnect();
                });
#ifdef FCPP_MPI
                // TODO: remove debug information
                if (m_mpi_comm_map.size()) {
                    std::cerr << "MPI messages waiting to be sent:" << std::endl;
                    for (auto const& process_messages : m_mpi_comm_map) {
                        for (auto const& node_messages : process_messages.second) {
                            for (auto const& req : node_messages.second.conn_requests) {
                                std::cerr << "\t" << to_string(req)
                                    << " request sent to node" << node_messages.first
                                    << " of rank " << process_messages.first
                                    << " from node " << req.first;
                            }
                            for (auto const& msg : node_messages.second.messages) {
                                std::cerr << "\tmessage sent to node " << node_messages.first
                                    << " of rank " << process_messages.first
                                    << " from node " << msg.first
                                    << " at time " << msg.second.first << std::endl;
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
                return min(min(m_send_schedule.next(), m_recv_schedule.next()), P::net::next());
            }

            //! @brief Updates the internal status of net component.
            void update() {
                times_t t_send = m_send_schedule.next();
                times_t t_recv = m_recv_schedule.next();
                times_t pt = P::net::next();                    

                if (t_send < pt and t_send <= t_recv) {
                    // TODO: provide FCPP abstraction on MPI basic routines MPI_Send/MPI_Recv/MPI_Iprobe/MPI_Get_count
                    m_send_schedule.step(get_generator(has_randomizer<P>{}, *this), fcpp::common::make_tagged_tuple<>());
                    common::osstream os;
                    int snd_buffer_size = 0;

                    for (std::pair<const int, mpi_message_type> process_messages : m_mpi_comm_map){
                        //std::cout << "Sending remote message to process: " << process_messages.first << std::endl;
                        os << process_messages.second;
                        snd_buffer_size = os.size();
                        std::vector<char> m_data = std::move(os.data());
                        // 0 for a size message, 1 for a data message
                        MPI_Send(&snd_buffer_size, 1, MPI_INT, process_messages.first, 0, MPI_COMM_WORLD);
                        MPI_Send(m_data.data(), snd_buffer_size, MPI_CHAR, process_messages.first, 1, MPI_COMM_WORLD);
                    }
                } else if (t_recv < pt) {
                    int rcv_buffer_size;
                    int messageExists = 0;
                    m_recv_schedule.step(get_generator(has_randomizer<P>{}, *this), fcpp::common::make_tagged_tuple<>());
                    for (int rank = 0; rank < m_mpi_procs; rank++) {
                        messageExists = 0;
                        // std::cout << "Checking remote message from process: " << rank << std::endl;
                        rcv_buffer_size = 0;
                        MPI_Iprobe(rank, 0, MPI_COMM_WORLD, &messageExists, MPI_STATUS_IGNORE);
                        if (messageExists){
                            //std::cout << "Receiving remote message from process: " << rank << std::endl;
                            MPI_Recv(&rcv_buffer_size, 1, MPI_INT, rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            // TODO: si può rendere più efficiente evitando di riallocare ogni volta?
                            std::vector<char>rcv_buffer(rcv_buffer_size);
                            // qui bisogna fare un loop per considerare ogni rango
                            // per ciascuno, assicurarsi che la computazione non si blocchi se non ci sono messaggi
                            MPI_Recv(&rcv_buffer[0], rcv_buffer_size, MPI_CHAR, rank, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                            common::isstream is(std::move(rcv_buffer));
                            mpi_message_type incoming_msg_map;
                            is >> incoming_msg_map;

                            // ora scansiono la mappa, smistando i messaggi ai destinatari
                            // id ricevitore
                            for (std::pair<const device_t, node_message_type> node_messages : incoming_msg_map){
                                //std::cout << "Processing messages SENT to node " << node_messages.first << std::endl;
                                // recupero il puntatore a nodo
                                // TODO: check whether the node still exists, if not send back a disconnection request
                                typename F::node* n = const_cast<typename F::node*>(&P::net::node_at(node_messages.first));
                                common::lock_guard<parallel> l(n->mutex);
                                // id mittente
                                for (std::pair<const device_t, std::pair<times_t, typename F::node::message_t>> msg : node_messages.second.messages){
                                    //std::cout << "Message RECEIVED FROM " << msg.first << std::endl;
                                    //std::cout << "Message TIMESTAMP " << msg.second.first << std::endl;
                                    n->receive(msg.second.first, msg.first, msg.second.second);
                                }
                                // TODO: process node_messages.second.conn_requests
                            }
                        }
                    }
                } else P::net::update();
            }
#endif

            //! @brief The current MPI process rank.
            inline int mpi_rank() const {
                return m_mpi_rank;
            }

            //! @brief Computes the MPI process rank for a given node.
            inline int mpi_rank(device_t i) {
#ifdef FCPP_MPI
                return m_node_splitter(nullptr, common::make_tagged_tuple<tags::uid, tags::mpi_procs>(i, m_mpi_procs));
#else
                return m_mpi_rank;
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

            //! @brief Number of MPI processes.
            int m_mpi_procs;

            //! @brief Rank of the current MPI process.
            int m_mpi_rank;

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
