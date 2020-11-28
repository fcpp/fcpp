// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

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

#include "lib/component/base.hpp"
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

    //! @brief Declaration flag associating to whether parallelism is enabled.
    template <bool b>
    struct parallel;

    //! @brief Node initialisation tag associating to communication power.
    struct connection_data {};

    //! @brief Initialisation tag associating to the time sensitivity, allowing indeterminacy below it.
    struct epsilon;

    //! @brief Net initialisation tag associating to communication radius.
    struct radius {};
}


//! @cond INTERNAL
namespace details {
    //! @brief A cell of space, containing nodes and linking to neighbour cells.
    template <bool parallel, typename N>
    class cell {
      public:
        //! @brief Default constructors.
        cell() = default;
        cell(const cell&) = default;
        cell(cell&&) = default;
        cell& operator=(const cell&) = default;
        cell& operator=(cell&&) = default;

        //! @brief Inserts a node in the cell.
        void insert(N& n) {
            common::lock_guard<parallel> l(m_mutex);
            m_contents.insert(&n);
        }

        //! @brief Removes a node from the cell.
        void erase(N& n) {
            common::lock_guard<parallel> l(m_mutex);
            m_contents.erase(&n);
        }

        //! @brief Links a new cell.
        void link(const cell& o) {
            common::lock_guard<parallel> l(m_mutex);
            m_linked.push_back(&o);
        }

        //! @brief Gives const access to linked cells.
        std::conditional_t<parallel, std::vector<const cell*>, std::vector<const cell*> const&>
        linked() const {
            common::lock_guard<parallel> l(m_mutex);
            return m_linked;
        }

        std::conditional_t<parallel, std::unordered_set<N*>, std::unordered_set<N*> const&>
        content() const {
            common::lock_guard<parallel> l(m_mutex);
            return m_contents;
        }

      private:
        //! @brief The content of the cell.
        std::unordered_set<N*> m_contents;

        //! @brief The linked cells.
        std::vector<const cell*> m_linked;

        //! @brief A mutex regulating access to this cell.
        mutable common::mutex<parallel> m_mutex;
    };
}
//! @endcond


/**
 * @brief Component handling message exchanges between nodes.
 *
 * Requires a \ref physical_position parent component.
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 * Any \ref simulated_connector component cannot be a parent of a \ref timer otherwise round planning may block message exchange.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::connector defines the connector class (defaults to \ref connect::clique "connect::clique<dimension>").
 * - \ref tags::delay defines the delay generator for sending messages after rounds (defaults to zero delay through \ref distribution::constant_n "distribution::constant_n<times_t, 0>").
 * - \ref tags::dimension defines the dimensionality of the space (defaults to 2).
 *
 * <b>Declaration flags:</b>
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
struct simulated_connector {
    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief The dimensionality of the space.
    constexpr static size_t dimension = common::option_num<tags::dimension, 2, Ts...>;

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
        DECLARE_COMPONENT(connector);
        REQUIRE_COMPONENT(connector,positioner);
        CHECK_COMPONENT(randomizer);

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
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_delay(get_generator(has_randomizer<P>{}, *this),t), m_data(common::get_or<tags::connection_data>(t, connection_data_type{})) {
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
            const connection_data_type& connector_data() const {
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
                if (std::min(m_send, m_leave) < P::node::next()) {
                    PROFILE_COUNT("connector");
                    times_t t = next();
                    if (t == m_leave) {
                        PROFILE_COUNT("connector/cell");
                        m_leave = TIME_MAX;
                        if (P::node::next() < TIME_MAX) {
                            P::node::net.cell_move(P::node::as_final(), t);
                            set_leave_time(t);
                        }
                    }
                    if (t == m_send) {
                        PROFILE_COUNT("connector/send");
                        m_send = TIME_MAX;
                        for (auto c : P::node::net.cell_of(P::node::as_final()).linked())
                            for (typename F::node* n : c->content())
                                if (P::node::net.connection_success(m_data, P::node::position(t), n->m_data, n->position(t))) {
                                    typename F::node::message_t m;
                                    if (n != this) {
                                        P::node::mutex.unlock();
                                        common::lock(P::node::mutex, n->mutex);
                                        common::lock_guard<parallel> l(n->mutex, std::adopt_lock);
                                        n->receive(t, P::node::uid, P::node::as_final().send(t, n->uid, m));
                                    } else n->receive(t, P::node::uid, P::node::as_final().send(t, n->uid, m));
                                }
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
                set_leave_time(t);
            }

          private: // implementation details
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
        };

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief The type of cells grouping nearby nodes.
            using cell_type = details::cell<parallel, typename F::node>;

            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) : P::net(t), m_connector(get_generator(has_randomizer<P>{}, *this),t) {}

            //! @brief Inserts a new node into its cell.
            void cell_enter(typename F::node& n) {
                cell_enter_impl(n, n.position());
            }

            //! @brief Removes a node from all cells.
            void cell_leave(typename F::node& n) {
                if (m_nodes.size() == 0) return;
                m_nodes.at(n.uid)->second.erase(n);
                common::lock_guard<parallel> l(m_mutex);
                m_nodes.erase(n.uid);
            }

            //! @brief Moves a node across cells.
            void cell_move(typename F::node& n, times_t t) {
                m_nodes.at(n.uid)->second.erase(n);
                cell_enter_impl(n, n.position(t));
            }

            //! @brief Returns the cells in proximity of node `n`.
            cell_type const& cell_of(const typename F::node& n) const {
                return m_nodes.at(n.uid)->second;
            }

            //! @brief The maximum connection radius.
            inline real_t connection_radius() const {
                return m_connector.maximum_radius();
            }

            //! @brief Checks whether connection is possible.
            inline bool connection_success(const connection_data_type& data1, const position_type& position1, const connection_data_type& data2, const position_type& position2) const {
                return m_connector(data1, position1, data2, position2);
            }

          private: // implementation details
            //! @brief A custom hash for cell identifiers.
            struct cell_hasher {
                size_t operator()(const cell_id_type& c) const {
                    size_t h = dimension;
                    for (auto& i : c) h ^= i + 0x9e3779b9 + (h << 6) + (h >> 2);
                    return h;
                }
            };

            //! @brief The map type used internally for storing cells.
            using cell_map_type = std::unordered_map<cell_id_type, cell_type, cell_hasher>;

            //! @brief Converts a position into a cell identifier.
            cell_id_type to_cell(const position_type& v) {
                cell_id_type c;
                for (size_t i=0; i<dimension; ++i) c[i] = (int)floor(v[i]/connection_radius());
                return c;
            }

            //! @brief Interts a node in the cell correspoding to a given position.
            void cell_enter_impl(typename F::node& n, const position_type& p) {
                cell_id_type c = to_cell(p);
                if (m_cells.count(c) == 0) {
                    common::lock_guard<parallel> l(m_mutex);
                    if (m_cells.count(c) == 0) {
                        m_cells[c].link(m_cells[c]); // creates cell
                        cell_id_type d;
                        for (size_t i=0; i<dimension; ++i) d[i] = c[i]-1;
                        while (true) {
                            if (c != d and m_cells.count(d) > 0) {
                                m_cells[c].link(m_cells[d]);
                                m_cells[d].link(m_cells[c]);
                            }
                            size_t i;
                            for (i = 0; i < dimension and d[i] == c[i]+1; ++i) d[i] = c[i]-1;
                            if (i == dimension) break;
                            ++d[i];
                        }
                    }
                }
                m_cells[c].insert(n);
                if (m_nodes.count(n.uid) == 0) {
                    common::lock_guard<parallel> l(m_mutex);
                    m_nodes[n.uid] = m_cells.find(c);
                } else m_nodes[n.uid] = m_cells.find(c);
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

            //! @brief The map from cell identifiers to cells.
            cell_map_type m_cells;

            //! @brief The map associating devices identifiers to their cell.
            std::unordered_map<device_t, typename cell_map_type::iterator> m_nodes;

            //! @brief The connector predicate.
            connector_type m_connector;

            //! @brief The mutex regulating access to maps.
            common::mutex<parallel> m_mutex;
        };
    };
};


}


}

#endif // FCPP_SIMULATION_SIMULATED_CONNECTOR_H_
