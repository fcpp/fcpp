// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of tags and types of attributes describing a node in the input.
    template <typename... Ts>
    struct node_attributes {};

    //! @brief Declaration tag associating to a sequence of node initialisation tags and generating distributions.
    template <typename... Ts>
    struct init;

    //! @brief Net initialisation tag associating to the name of the file or input stream specifying graph nodes.
    struct nodesinput {};

    //! @brief Net initialisation tag associating to the name of the file or input stream specifying graph arcs.
    struct arcsinput {};

    //! @brief Net initialisation tag setting a default start for nodes.
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
 * - \ref tags::node_attributes defines a sequence of attributes tags and and types.
 *
 * Nodes generated receive all tags produced by generating distributions, and \ref tags::start associated to the creation time.
 */
template <class... Ts>
struct graph_spawner {
    //! @brief Type sequence of node attributes parameters.
    using attributes_tag_type = common::option_types<tags::node_attributes, Ts...>;
    //! @brief Type sequence of node attributes parameters, defaulting to tuple store parameters without node attributes.
    using attributes_type = std::conditional_t<std::is_same<attributes_tag_type, common::type_sequence<>>::value, common::option_types<tags::tuple_store, Ts...>, attributes_tag_type>;
    //! @brief Tagged tuple of node attributes.
    using attributes_tuple_type = common::tagged_tuple_t<attributes_type>;

    //! @brief Node initialisation tags and generating distributions as tagged tuples.
    using init_tag_type = common::option_types<tags::init, Ts...>;
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
            net(common::tagged_tuple<S,T> const& t) :
                P::net(t),
                m_start(common::get_or<tags::start>(t, 0)),
                m_arcsstream(details::make_istream(common::get_or<tags::arcsinput>(t, "arcs"))),
                m_nodesstream(details::make_istream(common::get_or<tags::nodesinput>(t, "index"))),
                m_distributions(build_distributions(t, typename init_tuple_type::tags(), typename init_tuple_type::types()))
            {
                read_nodes();
                read_arcs();
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

            //! @brief Constructs the tuple of distributions, feeding the initialising tuple to all of them.
            template <typename S, typename T, typename... Ss, typename... Us>
            common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>
            build_distributions(common::tagged_tuple<S,T> const& t, common::type_sequence<Ss...>, common::type_sequence<Us...>) {
                return {Us{get_generator(has_randomizer<P>{}, *this),t}...};
            }

            //! @brief Adds a `start` time to a node file tuple (if not present in the file).
            template <typename S, typename T>
            auto push_time(common::tagged_tuple<S,T> const& tup, common::type_sequence<>) {
                using tt_type = typename common::tagged_tuple<S,T>::template push_back<tags::start, times_t>;
                tt_type tt(tup);
                common::get<tags::start>(tt) = this->m_start;
                return tt;
            }

            //! @brief No need to add a `start` time to a node file tuple (if present in the file).
            template <typename T>
            inline T const& push_time(T const& t, common::type_sequence<tags::start>) {
                return t;
            }

            inline void read_nodes() {
                attributes_tuple_type row;

                while (read_row(*m_nodesstream, row, typename attributes_tuple_type::tags{})) {
                    //                    auto trow = push_time(row, typename attributes_tuple_type::tags::template intersect<tags::start>());
                    using tag_type = typename init_tuple_type::tags;
                    using dist_type = std::decay_t<decltype(m_distributions)>;
                    using res_type = std::decay_t<decltype(std::declval<dist_type>()(crand{}, common::tagged_tuple_t<>{}))>;
                    using full_type = common::tagged_tuple_cat<attributes_tuple_type, res_type>;
                    full_type tt;
                    tt = row;
                    call_distribution(m_distributions, get_generator(has_randomizer<P>{}, *this), tt, tag_type{});
                    auto ttt = push_time(tt, typename attributes_tuple_type::tags::template intersect<tags::start>());
                    device_t n = P::net::node_emplace(ttt);
                    assert(P::net::node_at(n).next() == 0);
                }
            }

            inline bool read_row(std::istream& is, attributes_tuple_type& row, common::type_sequence<>) {
                return true;
            }

            template <typename S, typename... Ss>
            inline bool read_row(std::istream& is, attributes_tuple_type& row, common::type_sequence<S, Ss...>) {
                if (!(is >> common::get<S>(row))) {
                    assert(is.eof());
                    return false;
                }
                return read_row(is, row, common::type_sequence<Ss...>{});
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

            inline void read_arcs() {
                std::pair<size_t,size_t> row;

                while (read_arc(*m_arcsstream, row)) {
                    typename net::lock_type l1, l2;
                    P::net::node_at(row.first,l1).connect(&P::net::node_at(row.second,l2));
                }
            }

            inline bool read_arc(std::istream& is, std::pair<size_t,size_t> &row) {
                is >> row.first;
                is >> row.second;
                if (!is) {
                    assert(is.eof());
                    return false;
                }

                return true;
            }

            //! @brief The default start of nodes.
            size_t m_start;

            //! @brief The stream describing graph nodes.
            std::shared_ptr<std::istream> m_nodesstream;

            //! @brief The stream describing graph arcs.
            std::shared_ptr<std::istream> m_arcsstream;

            //! @brief The generator tuple.
            init_tuple_type m_distributions;
        };
    };
};


}


}

#endif // FCPP_CLOUD_GRAPH_SPAWNER_H_
