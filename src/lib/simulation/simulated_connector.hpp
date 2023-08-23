// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file simulated_connector.hpp
 * @brief Implementation of the `simulated_connector` component handling message exchanges between nodes.
 */

#ifndef FCPP_SIMULATION_SIMULATED_CONNECTOR_H_
#define FCPP_SIMULATION_SIMULATED_CONNECTOR_H_

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


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a connector class (defaults to \ref connect::clique "connect::clique<dimension>").
    template <typename T>
    struct connector {};

    //! @brief Declaration tag associating to a delay generator for sending messages after rounds (defaults to zero delay through \ref distribution::constant_n "distribution::constant_n<times_t, 0>").
    template <typename T>
    struct delay {};

    //! @brief Declaration tag associating to the dimensionality of the space (defaults to 2).
    template <intmax_t n>
    struct dimension;

    //! @brief Declaration flag associating to whether message sizes should be emulated (defaults to false).
    template <bool b>
    struct message_size {};

    //! @brief Declaration flag associating to whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
    template <bool b>
    struct parallel;

    //! @brief Node initialisation tag associating to communication power (defaults to `connector_type::data_type{}`).
    struct connection_data {};

    //! @brief Initialisation tag associating to the time sensitivity, allowing indeterminacy below it (defaults to \ref FCPP_TIME_EPSILON).
    struct epsilon;
}


//! @cond INTERNAL
namespace details {
    //! @brief A cell of space, containing nodes and linking to neighbour cells.
    template <bool parallel, typename N>
    class cell {
      public:
        //! @brief Default constructors.
        cell() = default;
        cell(cell const&) = delete;
        cell(cell&&) = delete;
        cell& operator=(cell const&) = delete;
        cell& operator=(cell&&) = delete;

        //! @brief Inserts a node in the cell.
        void insert(N& n) {
            common::exclusive_guard<parallel> l(m_mutex);
            m_contents.insert(&n);
        }

        //! @brief Removes a node from the cell.
        void erase(N& n) {
            common::exclusive_guard<parallel> l(m_mutex);
            m_contents.erase(&n);
        }

        //! @brief Links a new cell.
        void link(cell const& o) {
            common::exclusive_guard<parallel> l(m_mutex);
            m_linked.push_back(&o);
        }

        //! @brief Gives const access to linked cells.
        std::conditional_t<parallel, std::vector<cell const*>, std::vector<cell const*> const&>
        linked() const {
            common::shared_guard<parallel> l(m_mutex);
            return m_linked;
        }

        std::conditional_t<parallel, std::unordered_set<N*>, std::unordered_set<N*> const&>
        content() const {
            common::shared_guard<parallel> l(m_mutex);
            return m_contents;
        }

      private:
        //! @brief The content of the cell.
        std::unordered_set<N*> m_contents;

        //! @brief The linked cells.
        std::vector<cell const*> m_linked;

        //! @brief A mutex regulating access to this cell.
        mutable common::shared_mutex<parallel> m_mutex;
    };
}
//! @endcond


/**
 * @brief Component handling message exchanges between nodes.
 *
 * Requires a \ref simulated_positioner parent component.
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
 * template <typename G, typename S, typename T> connector_type(G&& gen, common::tagged_tuple<S,T> const& tup);
 * real_t maximum_radius() const;
 * bool operator()(data_type const& data1, position_type const& position1, data_type const& data2, position_type const& position2) const;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class... Ts>
struct simulated_connector {
    //! @brief Whether message sizes should be emulated.
    constexpr static bool message_size = common::option_flag<tags::message_size, false, Ts...>;

    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief The dimensionality of the space.
    constexpr static intmax_t dimension = common::option_num<tags::dimension, 2, Ts...>;

    //! @brief Type for representing a position.
    using position_type = vec<dimension>;

    //! @brief Type for representing a cell identifier.
    using cell_id_type = std::array<int, dimension>;

    //! @brief Connector class.
    using connector_type = common::option_type<tags::connector, connect::clique<dimension>, Ts...>;

    //! @brief The type of settings data regulating connection.
    using connection_data_type = typename connector_type::data_type;

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
        REQUIRE_COMPONENT(connector,positioner);
        CHECK_COMPONENT(identifier);
        CHECK_COMPONENT(randomizer);
        CHECK_COMPONENT(scheduler);
        CHECK_COMPONENT(calculus);
        //! @endcond

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            //! @brief The type of settings data regulating connection.
            using connection_data_type = simulated_connector<Ts...>::connection_data_type;

            //! @{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_delay(get_generator(has_randomizer<P>{}, *this),t), m_data(common::get_or<tags::connection_data>(t, connection_data_type{})), m_nbr_msg_size(0) {
                m_send = m_leave = TIME_MAX;
                m_epsilon = common::get_or<tags::epsilon>(t, FCPP_TIME_EPSILON);
                P::node::net.cell_enter(P::node::as_final());
            }

            //! @brief Destructor leaving the corresponding cell.
            ~node() {
                P::node::net.cell_leave(P::node::as_final());
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

            //! @brief Size of last message sent (zero if `message_size` is false).
            size_t msg_size() const {
                if (message_size) return fcpp::details::self(m_nbr_msg_size.front(), P::node::uid);
                else return 0;
            }

            /**
             * @brief Sizes of messages received from neighbours.
             *
             * Returns a `field<size_t> const&` if `message_size` is true, otherwise it returns a `size_t` equal to zero.
             */
            auto nbr_msg_size() const {
                return get_nbr_msg_size(common::number_sequence<message_size>{});
            }

            /**
             * @brief Returns next event to schedule for the node component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(std::min(m_send, m_leave), P::node::next());
            }

            //! @brief Updates the internal status of node component.
            void update() {
                times_t t = std::min(m_send, m_leave);
                times_t pt = P::node::next();
                if (t < pt) {
                    PROFILE_COUNT("connector");
                    if (t == m_leave) {
                        PROFILE_COUNT("connector/cell");
                        m_leave = TIME_MAX;
                        if (pt < TIME_MAX) {
                            P::node::net.cell_move(P::node::as_final(), t);
                            set_leave_time(t);
                        }
                    }
                    if (t == m_send) {
                        PROFILE_COUNT("connector/send");
                        m_send = TIME_MAX;
                        typename F::node::message_t m;
                        P::node::as_final().send(t, m);
                        P::node::as_final().receive(t, P::node::uid, m);
                        common::unlock_guard<parallel> u(P::node::mutex);
                        for (auto c : P::node::net.cell_of(P::node::as_final()).linked())
                            for (typename F::node* n : c->content()) {
                                common::lock_guard<parallel> l(n->mutex);
                                if (n != this and P::node::net.connection_success(get_generator(has_randomizer<P>{}, *this), m_data, P::node::position(t), n->m_data, n->position(t))) {
                                    n->receive(t, P::node::uid, m);
                                }
                            }
                    }
                } else P::node::update();
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                m_send = t + m_delay(get_generator(has_randomizer<P>{}, *this), common::tagged_tuple_t<>{});
                P::node::round_start(t);
                maybe_align_inplace_m_nbr_msg_size(common::number_sequence<has_calculus<P>::value and message_size>{});
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                P::node::net.cell_move(P::node::as_final(), t);
                if (has_scheduler<P>::value and P::node::next() == TIME_MAX) m_leave = TIME_MAX;
                else set_leave_time(t);
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            inline void receive(times_t t, device_t d, common::tagged_tuple<S,T> const& m) {
                P::node::receive(t, d, m);
                receive_size(common::number_sequence<message_size>{}, d, m);
            }

          private: // implementation details
            //! @brief Sizes of messages received from neighbours (disabled).
            constexpr static size_t get_nbr_msg_size(common::number_sequence<false>) {
                return 0;
            }
            //! @brief Sizes of messages received from neighbours (enabled).
            field<size_t> const& get_nbr_msg_size(common::number_sequence<true>) const {
                return m_nbr_msg_size.front();
            }

            //! @brief Changes the domain of m_nbr_msg_size to match the domain of the neightbours ids (disabled).
            void maybe_align_inplace_m_nbr_msg_size(common::number_sequence<false>) {}
            //! @brief Changes the domain of m_nbr_msg_size to match the domain of the neightbours ids (enabled).
            void maybe_align_inplace_m_nbr_msg_size(common::number_sequence<true>) {
                align_inplace(m_nbr_msg_size.front(), std::vector<device_t>(fcpp::details::get_ids(P::node::nbr_uid())));
            }

            //! @brief Stores size of received message (disabled).
            template <typename S, typename T>
            void receive_size(common::number_sequence<false>, device_t, common::tagged_tuple<S,T> const&) {}
            //! @brief Stores size of received message (enabled).
            template <typename S, typename T>
            void receive_size(common::number_sequence<true>, device_t d, common::tagged_tuple<S,T> const& m) {
                common::osstream os;
                os << m;
                fcpp::details::self(m_nbr_msg_size.front(), d) = os.size();
            }

            //! @brief Checks when the node will leave the current cell.
            void set_leave_time(times_t t) {
                m_leave = TIME_MAX;
                position_type x = P::node::position(t);
                real_t R = P::node::net.connection_radius();
                for (size_t i=0; i<dimension; ++i) {
                    int c = (int)floor(x[i]/R);
                    m_leave = std::min(m_leave, P::node::reach_time(i,  c   *R, t));
                    m_leave = std::min(m_leave, P::node::reach_time(i, (c+1)*R, t));
                }
                m_leave = std::max(m_leave, t);
                if (m_leave < TIME_MAX) m_leave += m_epsilon;
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
            delay_type m_delay;

            //! @brief Time of the next send-message and cell-leave events (and epsilon time).
            times_t m_send, m_leave, m_epsilon;

            //! @brief Data regulating the connection.
            connection_data_type m_data;

            //! @brief Sizes of messages received from neighbours.
            common::option<field<size_t>, message_size> m_nbr_msg_size;
        };

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief The type of cells grouping nearby nodes.
            using cell_type = details::cell<parallel, typename F::node>;

            //! @brief Type for representing a position.
            using position_type = simulated_connector<Ts...>::position_type;

            //! @brief The type of settings data regulating connection.
            using connection_data_type = simulated_connector<Ts...>::connection_data_type;

            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) : P::net(t), m_connector(get_generator(has_randomizer<P>{}, *this),t) {}

            //! @brief Destructor ensuring that nodes are deleted first.
            ~net() {
                maybe_clear(has_identifier<P>{}, *this);
            }

            //! @brief Inserts a new node into its cell.
            void cell_enter(typename F::node& n) {
                cell_enter_impl<false>(n, n.position());
            }

            //! @brief Removes a node from all cells.
            void cell_leave(typename F::node& n) {
                if (m_nodes.size() == 0) return;
                common::exclusive_guard<parallel> l(m_node_mutex);
                m_nodes.at(n.uid)->second.erase(n);
                m_nodes.erase(n.uid);
            }

            //! @brief Moves a node across cells.
            void cell_move(typename F::node& n, times_t t) {
                cell_enter_impl<true>(n, n.position(t));
            }

            //! @brief Returns the cells in proximity of node `n`.
            cell_type const& cell_of(typename F::node const& n) const {
                common::shared_guard<parallel> l(m_node_mutex);
                return m_nodes.at(n.uid)->second;
            }

            //! @brief The maximum connection radius.
            inline real_t connection_radius() const {
                return m_connector.maximum_radius();
            }

            //! @brief Checks whether connection is possible.
            template <typename G>
            inline bool connection_success(G&& gen, connection_data_type const& data1, position_type const& position1, connection_data_type const& data2, position_type const& position2) const {
                return m_connector(gen, data1, position1, data2, position2);
            }

          private: // implementation details
            //! @brief A custom hash for cell identifiers.
            struct cell_hasher {
                size_t operator()(cell_id_type const& c) const {
                    size_t h = dimension;
                    for (auto& i : c) h ^= i + 0x9e3779b9 + (h << 6) + (h >> 2);
                    return h;
                }
            };

            //! @brief The map type used internally for storing cells.
            using cell_map_type = std::unordered_map<cell_id_type, cell_type, cell_hasher>;

            //! @brief Converts a position into a cell identifier.
            cell_id_type to_cell(position_type const& v) {
                cell_id_type c;
                for (size_t i=0; i<dimension; ++i) c[i] = (int)floor(v[i]/connection_radius());
                return c;
            }

            //! @brief Interts a node in the cell correspoding to a given position.
            template <bool move>
            inline void cell_enter_impl(typename F::node& n, position_type const& p) {
                cell_id_type c = to_cell(p);
                typename cell_map_type::iterator nit;
                bool create;
                {
                    common::shared_guard<parallel> l(m_cell_mutex);
                    nit = m_cells.find(c);
                    create = nit == m_cells.end();
                }
                if (create) {
                    common::exclusive_guard<parallel> l(m_cell_mutex);
                    nit = m_cells.emplace(std::piecewise_construct, std::make_tuple(c), std::make_tuple()).first;
                    nit->second.link(nit->second);
                    cell_id_type d;
                    for (size_t i=0; i<dimension; ++i) d[i] = c[i]-1;
                    while (true) {
                        if (c != d) {
                            auto lit = m_cells.find(d);
                            if (lit != m_cells.end()) {
                                nit->second.link(lit->second);
                                lit->second.link(nit->second);
                            }
                        }
                        size_t i;
                        for (i = 0; i < dimension and d[i] == c[i]+1; ++i) d[i] = c[i]-1;
                        if (i == dimension) break;
                        ++d[i];
                    }
                }
                typename cell_map_type::iterator *it;
                if (move) {
                    {
                        common::shared_guard<parallel> l(m_node_mutex);
                        it = &m_nodes.at(n.uid);
                    }
                    if (c == (*it)->first) return;
                    else (*it)->second.erase(n);
                } else {
                    common::exclusive_guard<parallel> l(m_node_mutex);
                    it = &m_nodes[n.uid];
                }
                *it = nit;
                (*it)->second.insert(n);
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

            //! @brief Deletes all nodes if parent identifier.
            template <typename N>
            inline void maybe_clear(std::true_type, N& n) {
                return n.node_clear();
            }

            //! @brief Does nothing otherwise.
            template <typename N>
            inline void maybe_clear(std::false_type, N&) {}

            //! @brief The map from cell identifiers to cells.
            cell_map_type m_cells;

            //! @brief The map associating devices identifiers to their cell.
            std::unordered_map<device_t, typename cell_map_type::iterator> m_nodes;

            //! @brief The connector predicate.
            connector_type m_connector;

            //! @brief The mutexes regulating access to maps.
            mutable common::shared_mutex<parallel> m_node_mutex, m_cell_mutex;
        };
    };
};


}


}

#endif // FCPP_SIMULATION_SIMULATED_CONNECTOR_H_
