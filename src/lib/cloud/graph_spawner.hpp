// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file graph_spawner.hpp
 * @brief Implementation of the `graph_spawner` component handling generation of nodes from a graph.
 */

#ifndef FCPP_CLOUD_GRAPH_SPAWNER_H_
#define FCPP_CLOUD_GRAPH_SPAWNER_H_

#include <type_traits>
#include <utility>
#include <iostream>
#include <fstream>

#include "lib/component/base.hpp"
#include "lib/component/storage.hpp"
#include "lib/option/sequence.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of tags and types of attributes describing a node in the input.
    template <typename... Ts>
    struct node_attributes {};

    //! @brief Net initialisation tag associating to the name of the file specifying graph nodes.
    struct nodesfile;

    //! @brief Net initialisation tag associating to the name of the file specifying graph arcs.
    struct arcsfile;

    //! @brief Net initialisation tag setting a default start for nodes.
    struct start;
}


/**
 * @brief Component handling generation of nodes from a graph.
 *
 * Requires a \ref identifier parent component.
 * The \ref timer component cannot be a parent of a \ref spawner otherwise to preserve spawn scheduling.
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::node_attributes defines a sequence of attributes tags and and types.
 *
 * Nodes generated receive all tags produced by generating distributions, and \ref tags::start associated to the creation time.
 */
template <class... Ts>
struct graph_spawner {
    //! @brief Attributes type.
    using attributes_tag_type = common::option_types<tags::node_attributes, Ts...>;
    using attributes_type = std::conditional_t<std::is_same<attributes_tag_type, common::type_sequence<>>::value, common::option_types<tags::tuple_store, Ts...>, attributes_tag_type>;
    using attributes_tuple_type = common::tagged_tuple_t<attributes_type>;


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
        DECLARE_COMPONENT(graph_spawner);
        REQUIRE_COMPONENT(graph_spawner,identifier);
        AVOID_COMPONENT(graph_spawner,timer);
        CHECK_COMPONENT(randomizer);

        //! @brief The local part of the component.
        using node = typename P::node;

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) :
                P::net(t),
                m_start(common::get_or<tags::start>(t, 0)),
                m_nodesfile( common::get_or<tags::nodesfile>(t, "index") ),
                m_arcsfile( common::get_or<tags::arcsfile>(t, "arcs") ) {
                read_nodes();
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

            //! @brief Adds a `start` time to a given tagged tuple.
            template <typename S, typename T>
            auto push_time(const common::tagged_tuple<S,T>& tup, times_t t) {
                using tt_type = typename common::tagged_tuple<S,T>::template push_back<tags::start, times_t>;
                tt_type tt(tup);
                common::get<tags::start>(tt) = t;
                return tt;
            }

            inline void read_nodes() {
                std::ifstream nodesf(m_nodesfile);
                attributes_tuple_type row;

                while (nodesf) {
                    read_row(nodesf, row, typename attributes_tuple_type::tags{});
                    // chiamare emplace con row
                }

                nodesf.close();
            }


            inline void read_row(std::istream& is, attributes_tuple_type& row, common::type_sequence<>) {
            }

            template <typename S, typename... Ss>
            inline void read_row(std::istream& is, attributes_tuple_type& row, common::type_sequence<S, Ss...>) {
                is >> common::get<S>(row);
                read_row(is, row, common::type_sequence<Ss...>{});
            }

            // //! @brief Emplaces a node generated through the j-th distribution.
            // template <size_t i>
            // inline void call_distribution(size_t j, times_t t, std::index_sequence<i>) {
            //     assert(i == j);
            //     P::net::node_emplace(push_time(std::get<i>(m_distributions)(get_generator(has_randomizer<P>{}, *this)), t));
            // }

            //! @brief The default start of nodes.
            size_t m_start;

            //! @brief The file describing graph nodes.
            std::string m_nodesfile;

            //! @brief The file describing graph arcs.
             std::string m_arcsfile;
        };
    };
};


}


}

#endif // FCPP_SIMULATION_SPAWNER_H_
