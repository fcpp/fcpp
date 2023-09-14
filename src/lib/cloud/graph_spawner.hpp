// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file graph_spawner.hpp
 * @brief Implementation of the `graph_spawner` component handling generation of nodes from a graph.
 */

#ifndef FCPP_CLOUD_GRAPH_SPAWNER_H_
#define FCPP_CLOUD_GRAPH_SPAWNER_H_

#include <cassert>
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


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to the sequence of attributes tags and types describing nodes (defaults to the empty sequence).
    template <typename... Ts>
    struct node_attributes {};

    //! @brief Declaration tag associating to a sequence of node initialisation tags and generating distributions (defaults to the empty sequence).
    template <typename... Ts>
    struct init;

    //! @brief Net initialisation tag associating to the name of the file or input stream specifying graph nodes (default to "index").
    struct nodesinput {};

    //! @brief Net initialisation tag associating to the name of the file or input stream specifying graph arcs (default to "arcs").
    struct arcsinput {};

    //! @brief Node initialisation tag associating to a starting time of execution (defaults to \ref TIME_MAX).
    struct start;
}

//! @cond INTERNAL
namespace details {
    //! @brief Makes an istream reference from a `std::string` path.
    std::shared_ptr<std::istream> make_istream(std::string const& s);

    //! @brief Makes an istream reference from a `char const*` path.
    std::shared_ptr<std::istream> make_istream(char const* s);

    //! @brief Makes an istream reference from a stream pointer.
    std::shared_ptr<std::istream> make_istream(std::istream* i);
}
//! @endcond

/**
 * @brief Component handling generation of nodes from a graph.
 *
 * Requires a \ref identifier parent component.
 * The \ref timer component cannot be a parent of a \ref spawner otherwise to preserve spawn scheduling.
 * If a \ref randomizer parent component is not found, \ref crand is used as random generator.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::node_attributes defines the sequence of attributes tags and types describing nodes (defaults to the empty sequence).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::nodesinput defines the name of the file or input stream specifying graph nodes (default to "index").
 * - \ref tags::arcsinput defines the name of the file or input stream specifying graph arcs (default to "arcs").
 *
 * Nodes generated receive all tags produced by generating distributions, and \ref tags::start associated to the creation time.
 */
template <class... Ts>
struct graph_spawner {
    //! @brief Type sequence of node attributes parameters.
    using attributes_tag_type = common::option_types<tags::node_attributes, Ts...>;

    //! @brief Type sequence of node attributes parameters, defaulting to tuple store parameters without node attributes.
    using attributes_type = std::conditional_t<
        std::is_same<attributes_tag_type, common::type_sequence<>>::value,
        common::storage_list<common::option_types<tags::node_store, Ts...>>,
        attributes_tag_type>;
    //! @brief Tagged tuple of node attributes.
    using attributes_tuple_type = common::tagged_tuple_t<attributes_type>;

    //! @brief Type sequence of node initialisation tags and generating distributions.
    using init_tag_type = common::option_types<tags::init, Ts...>;

    //! @brief Tagged tuple of node initialisation tags and generating distributions.
    using init_tuple_type = common::tagged_tuple_t<init_tag_type>;

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
        DECLARE_COMPONENT(spawner);
        REQUIRE_COMPONENT(spawner,identifier);
        CHECK_COMPONENT(randomizer);
        //! @endcond

        //! @brief The local part of the component.
        using node = typename P::node;

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) : P::net(t) {
                read_nodes(
                    details::make_istream(common::get_or<tags::nodesinput>(t, "nodes")),
                    build_distributions(t, typename init_tuple_type::tags(), typename init_tuple_type::types()),
                    common::get_or<tags::start>(t, 0)
                );
                read_arcs(details::make_istream(common::get_or<tags::arcsinput>(t, "arcs")));
            }

          private: // implementation details
            //! @brief Reads node information from file and creates corresponding nodes.
            void read_nodes(std::shared_ptr<std::istream> is, init_tuple_type dist, times_t start) {
                attributes_tuple_type row;
                while (read_row(*is, row, typename attributes_tuple_type::tags{})) {
                    using res_type = std::result_of_t<init_tuple_type(crand, common::tagged_tuple_t<>)>;
                    using full_type = common::tagged_tuple_cat<attributes_tuple_type, res_type>;
                    full_type tt = row;
                    call_distribution(dist, get_generator(has_randomizer<P>{}, *this), tt, typename init_tuple_type::tags{});
                    auto ttt = push_time(start, tt, typename full_type::tags::template intersect<tags::start>{});
                    device_t n = P::net::node_emplace(ttt);
                }
            }

            //! @brief Reads elements from a row of nodes file (empty overload).
            inline bool read_row(std::istream&, attributes_tuple_type&, common::type_sequence<>) {
                return true;
            }

            //! @brief Reads elements from a row of nodes file (non-empty overload).
            template <typename S, typename... Ss>
            inline bool read_row(std::istream& is, attributes_tuple_type& row, common::type_sequence<S, Ss...>) {
                if (not (is >> common::get<S>(row))) {
                    assert(is.eof());
                    return false;
                }
                return read_row(is, row, common::type_sequence<Ss...>{});
            }

            //! @brief Reads arc information from file and creates corresponding connections.
            void read_arcs(std::shared_ptr<std::istream> is) {
                device_t d1, d2;
                while (true) {
                    *is >> d1;
                    if (!*is) {
                        assert(is->eof());
                        break;
                    }
                    *is >> d2;
                    assert(*is);
                    if (d1 != d2) {
                        typename net::lock_type l;
                        P::net::node_at(d1,l).connect(d2);
                    }
                }
            }

            //! @brief Returns a `crand` generator if no randomizer.
            template <typename N>
            inline crand get_generator(std::false_type, N&) {
                return {};
            }

            //! @brief Returns the `randomizer` generator if available.
            template <typename N>
            inline auto& get_generator(std::true_type, N& n) {
                return n.generator();
            }

            //! @brief Constructs the tuple of distributions, feeding the initialising tuple to all of them.
            template <typename S, typename T, typename... Ss, typename... Us>
            common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>
            build_distributions(common::tagged_tuple<S,T> const& t, common::type_sequence<Ss...>, common::type_sequence<Us...>) {
                return {Us{get_generator(has_randomizer<P>{}, *this),t}...};
            }

            //! @brief Calls a distribution, updating the tuple of results (empty case).
            template <typename D, typename G, typename T>
            inline void call_distribution(D&, G&&, T&, common::type_sequence<>) {}

            //! @brief Calls a distribution, updating the tuple of results (general case).
            template <typename D, typename G, typename T, typename S, typename... Ss>
            inline void call_distribution(D& d, G&& g, T& t, common::type_sequence<S, Ss...>) {
                common::get<S>(t) = common::get<S>(d)(g, t);
                call_distribution(d, g, t, common::type_sequence<Ss...>{});
            }

            //! @brief No need to add a `start` time to a node file tuple (if present in the file).
            template <typename T>
            inline T const& push_time(times_t, T const& t, common::type_sequence<tags::start>) {
                return t;
            }

            //! @brief Adds a `start` time to a node file tuple (if not present in the file).
            template <typename S, typename T>
            inline auto push_time(times_t start, common::tagged_tuple<S,T> const& tup, common::type_sequence<>) {
                using tt_type = typename common::tagged_tuple<S,T>::template push_back<tags::start, times_t>;
                tt_type tt(tup);
                common::get<tags::start>(tt) = start;
                return tt;
            }
        };
    };
};


}


}

#endif // FCPP_CLOUD_GRAPH_SPAWNER_H_
