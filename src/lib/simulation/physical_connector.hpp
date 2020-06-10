// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file physical_connector.hpp
 * @brief Implementation of the `physical_connector` component handling physical evolution of a position through time.
 */

#ifndef FCPP_SIMULATION_PHYSICAL_CONNECTOR_H_
#define FCPP_SIMULATION_PHYSICAL_CONNECTOR_H_

#include <cmath>

#include <array>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lib/settings.hpp"
#include "lib/common/array.hpp"
#include "lib/common/distribution.hpp"
#include "lib/common/mutex.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/component/base.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Tag associating to a communication power.
    struct connector {};

    //! @brief Tag associating to a communication radius.
    struct radius {};
}


//! @cond INTERNAL
namespace details {
    //! @brief A cell of space, containing nodes and linking to neighbour cells.
    template <typename N>
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
            common::lock_guard<FCPP_PARALLEL> l(m_mutex);
            m_contents.insert(&n);
        }
        
        //! @brief Removes a node from the cell.
        void erase(N& n) {
            common::lock_guard<FCPP_PARALLEL> l(m_mutex);
            m_contents.erase(&n);
        }
        
        //! @brief Links a new cell.
        void link(const cell& o) {
            common::lock_guard<FCPP_PARALLEL> l(m_mutex);
            m_linked.push_back(&o);
        }
        
        //! @brief Gives const access to linked cells.
        std::conditional_t<FCPP_PARALLEL, std::vector<const cell*>, std::vector<const cell*> const&>
        linked() const {
            common::lock_guard<FCPP_PARALLEL> l(m_mutex);
            return m_linked;
        }

        std::conditional_t<FCPP_PARALLEL, std::unordered_set<N*>, std::unordered_set<N*> const&>
        content() const {
            common::lock_guard<FCPP_PARALLEL> l(m_mutex);
            return m_contents;
        }

      private:
        //! @brief The content of the cell.
        std::unordered_set<N*> m_contents;
        
        //! @brief The linked cells.
        std::vector<const cell*> m_linked;
        
        //! @brief A mutex regulating access to this cell.
        mutable common::mutex<FCPP_PARALLEL> m_mutex;
    };
}
//! @endcond


/**
 * @brief Component handling physical evolution of a position through time.
 *
 * Initialises `node` with tag `connector` associating to a `C::type` data (defaults to `C::type{}`).
 * Must be unique in a composition of components.
 * Requires a `position` parent component.
 * If a `randomizer` parent component is not found, `crand` is used as random generator.
 * Any `connector` component cannot be a parent of a `timer`, otherwise round planning may block message exchange.
 * The connection predicate `C` should be a class with the following members:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * typedef type;
 * template <typename G, typename S, typename T> C(G&& gen, const common::tagged_tuple<S,T>& tup);
 * double maximum_radius() const;
 * bool operator()(const type& data1, const std::array<double, n>& position1, const type& data2, const std::array<double, n>& position2) const;
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @param C A connection predicate.
 * @param G A generator for delays in sending messages (defaults to zero).
 * @param n Dimensionality of the space (defaults to 2).
 */
template <typename C, typename G = random::constant_distribution<times_t, 0>, size_t n = 2>
struct physical_connector {
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
        //! @brief Marks that a connector component is present.
        struct connector_tag {};

        //! @brief The dimensionality of the space.
        const size_t dimension = n;
        
        //! @brief The type of settings data regulating connection.
        using connector_type = typename C::type;

        //! @brief Checks if T has a `randomizer_tag`.
        template <typename T, typename = int>
        struct has_rtag : std::false_type {};
        template <typename T>
        struct has_rtag<T, std::conditional_t<true,int,typename T::randomizer_tag>> : std::true_type {};
        
        //! @brief Checks if T has a `connector_tag`.
        template <typename T, typename = int>
        struct has_ctag : std::false_type {};
        template <typename T>
        struct has_ctag<T, std::conditional_t<true,int,typename T::connector_tag>> : std::true_type {};
        
        //! @brief Asserts that P has no `connector_tag`.
        static_assert(not has_ctag<P>::value, "cannot combine multiple connector components");

        //! @brief Checks if T has a `position_tag`.
        template <typename T, typename = int>
        struct has_ptag : std::false_type {};
        template <typename T>
        struct has_ptag<T, std::conditional_t<true,int,typename T::position_tag>> : std::true_type {};
        
        //! @brief Asserts that P has a `position_tag`.
        static_assert(has_ptag<P>::value, "missing position parent for connector component");

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
            node(typename F::net& nt, const common::tagged_tuple<S,T>& t) : P::node(nt,t), m_delay(get_generator(common::bool_pack<has_rtag<P>::value>(), *this),t), m_data(common::get_or<tags::connector>(t, connector_type{})) {
                m_send = m_leave = TIME_MAX;
                P::node::net.cell_enter(P::node::as_final());
            }

            //! @brief Destructor leaving the corresponding cell.
            ~node() {
                P::node::net.cell_leave(P::node::as_final());
            }
            
            //! @brief Connector data.
            connector_type& connector_data() {
                return m_data;
            }
            
            //! @brief Connector data (const access).
            const connector_type& connector_data() const {
                return m_data;
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
                        for (const details::cell<typename F::node>* c : P::node::net.cell_of(P::node::as_final()).linked())
                            for (typename F::node* nn : c->content())
                                if (P::node::net.connection_success(m_data, P::node::position(t), nn->m_data, nn->position(t))) {
                                    typename F::node::message_t m;
                                    if (nn != this) {
                                        P::node::mutex.unlock();
                                        common::lock(P::node::mutex, nn->mutex);
                                        common::lock_guard<FCPP_PARALLEL> l(nn->mutex, std::adopt_lock);
                                        nn->receive(t, P::node::uid, P::node::as_final().send(t, nn->uid, m));
                                    } else nn->receive(t, P::node::uid, P::node::as_final().send(t, nn->uid, m));
                                }
                    }
                } else P::node::update();
            }
            
            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                m_send = t + m_delay(get_generator(common::bool_pack<has_rtag<P>::value>(), *this));
                set_leave_time(t);
            }
            
          private: // implementation details
            //! @brief Checks when the node will leave the current cell.
            void set_leave_time(times_t t) {
                m_leave = TIME_MAX;
                std::array<double, n> x = P::node::position(t);
                double R = P::node::net.connection_radius();
                for (size_t i=0; i<n; ++i) {
                    int c = (int)floor(x[i]/R);
                    m_leave = std::min(m_leave, P::node::reach_time(i,  c   *R, t));
                    m_leave = std::min(m_leave, P::node::reach_time(i, (c+1)*R, t));
                }
                m_leave = std::max(m_leave, t);
                if (m_leave < TIME_MAX) m_leave += FCPP_TIME_EPSILON;
            }
            
            //! @brief Returns the `randomizer` generator if available.
            template <typename N>
            inline auto& get_generator(common::bool_pack<true>, N& nn) {
                return nn.generator();
            }

            //! @brief Returns a `crand` generator otherwise.
            template <typename N>
            inline random::crand get_generator(common::bool_pack<false>, N&) {
                return random::crand();
            }

            //! @brief A generator for delays in sending messages.
            G m_delay;
            
            //! @brief Time of the next send-message and cell-leave events.
            times_t m_send, m_leave;

            //! @brief Data regulating the connection.
            connector_type m_data;
        };

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) : P::net(t), m_connector(get_generator(common::bool_pack<has_rtag<P>::value>(), *this),t) {}
            
            //! @brief Inserts a new node into its cell.
            void cell_enter(typename F::node& nn) {
                cell_enter_impl(nn, nn.position());
            }
            
            //! @brief Removes a node from all cells.
            void cell_leave(typename F::node& nn) {
                if (m_nodes.size() == 0) return;
                m_nodes.at(nn.uid)->second.erase(nn);
                common::lock_guard<FCPP_PARALLEL> l(m_mutex);
                m_nodes.erase(nn.uid);
            }
            
            //! @brief Moves a node across cells.
            void cell_move(typename F::node& nn, times_t t) {
                m_nodes.at(nn.uid)->second.erase(nn);
                cell_enter_impl(nn, nn.position(t));
            }
            
            //! @brief Returns the cells in proximity of node `n`.
            details::cell<typename F::node> const& cell_of(const typename F::node& nn) const {
                return m_nodes.at(nn.uid)->second;
            }
            
            //! @brief The maximum connection radius.
            inline double connection_radius() const {
                return m_connector.maximum_radius();
            }
            
            //! @brief Checks whether connection is possible.
            inline bool connection_success(const connector_type& data1, const std::array<double, n>& position1, const connector_type& data2, const std::array<double, n>& position2) const {
                return m_connector(data1, position1, data2, position2);
            }

          private: // implementation details
            //! @brief A custom hash for cell identifiers.
            struct cell_hasher {
                size_t operator()(const std::array<int, n>& c) const {
                    size_t h = n;
                    for (auto& i : c) h ^= i + 0x9e3779b9 + (h << 6) + (h >> 2);
                    return h;
                }
            };
            
            //! @brief The map type used internally for storing cells.
            using map_type = std::unordered_map<std::array<int, n>, details::cell<typename F::node>, cell_hasher>;
            
            //! @brief Converts a position into a cell identifier.
            std::array<int, n> to_cell(const std::array<double, n>& v) {
                std::array<int, n> c;
                for (size_t i=0; i<n; ++i) c[i] = (int)floor(v[i]/connection_radius());
                return c;
            }
            
            //! @brief Interts a node in the cell correspoding to a given position.
            void cell_enter_impl(typename F::node& nn, const std::array<double, n>& p) {
                std::array<int, n> c = to_cell(p);
                if (m_cells.count(c) == 0) {
                    common::lock_guard<FCPP_PARALLEL> l(m_mutex);
                    if (m_cells.count(c) == 0) {
                        m_cells[c].link(m_cells[c]); // creates cell
                        std::array<int, n> d;
                        for (size_t i=0; i<n; ++i) d[i] = c[i]-1;
                        while (true) {
                            if (c != d and m_cells.count(d) > 0) {
                                m_cells[c].link(m_cells[d]);
                                m_cells[d].link(m_cells[c]);
                            }
                            size_t i;
                            for (i = 0; i < n and d[i] == c[i]+1; ++i) d[i] = c[i]-1;
                            if (i == n) break;
                            ++d[i];
                        }
                    }
                }
                m_cells[c].insert(nn);
                if (m_nodes.count(nn.uid) == 0) {
                    common::lock_guard<FCPP_PARALLEL> l(m_mutex);
                    m_nodes[nn.uid] = m_cells.find(c);
                } else m_nodes[nn.uid] = m_cells.find(c);
            }
            
            //! @brief Returns the `randomizer` generator if available.
            template <typename N>
            inline auto& get_generator(common::bool_pack<true>, N& nn) {
                return nn.generator();
            }

            //! @brief Returns a `crand` generator otherwise.
            template <typename N>
            inline random::crand get_generator(common::bool_pack<false>, N&) {
                return random::crand();
            }

            //! @brief The map from cell identifiers to cells.
            map_type m_cells;
            
            //! @brief The map associating devices identifiers to their cell.
            std::unordered_map<device_t, typename map_type::iterator> m_nodes;
            
            //! @brief The connector predicate.
            C m_connector;
            
            //! @brief The mutex regulating access to maps.
            common::mutex<FCPP_PARALLEL> m_mutex;
        };
    };
};


}


//! @brief Namespace for connection predicates.
namespace connector {
    /**
     * Connection predicate which is true within a fixed radius (can be set through tag `radius`).
     *
     * @param num The numerator of the default value for the radius.
     * @param den The denominator of the default value for the radius.
     * @param n   Dimensionality of the space (defaults to 2).
     */
    template <intmax_t num, intmax_t den = 1, size_t n = 2>
    class fixed {
      public:
        //! @brief The data type.
        struct type {};
        
        //! @brief Generator and tagged tuple constructor.
        template <typename G, typename S, typename T>
        fixed(G&&, const common::tagged_tuple<S,T>& t) {
            m_radius = common::get_or<component::tags::radius>(t, ((double)num)/den);
        }
        
        //! @brief The maximum radius of connection.
        double maximum_radius() const {
            return m_radius;
        }

        //! @brief Checks if connection is possible.
        bool operator()(const type&, const std::array<double, n>& position1, const type&, const std::array<double, n>& position2) const {
            return norm(position1 - position2) <= m_radius;
        }
        
      private:
        //! @brief The connection radius.
        double m_radius;
    };
}


}

#endif // FCPP_SIMULATION_PHYSICAL_CONNECTOR_H_
