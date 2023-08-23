// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file hardware_logger.hpp
 * @brief Implementation of the `hardware_logger` component logging summarisations of nodes for deployed systems.
 */

#ifndef FCPP_DEPLOYMENT_HARDWARE_LOGGER_H_
#define FCPP_DEPLOYMENT_HARDWARE_LOGGER_H_

#include <iostream>

#include <cassert>
#include <cstddef>
#include <ctime>

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "lib/component/base.hpp"
#include "lib/component/logger.hpp"
#include "lib/deployment/os.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of tags and types for storing persistent data in nodes (defaults to the empty sequence).
    template <typename... Ts>
    struct node_store;
}


/**
 * @brief Component logging summarisations of nodes.
 *
 * Requires a \ref storage parent component.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::extra_info defines a sequence of net initialisation tags and types to be fed to plotters (defaults to the empty sequence).
 * - \ref tags::plot_type defines a plot type (defaults to \ref plot::none).
 * - \ref tags::ostream_type defines the output stream type to be used (defaults to `std::ostream`).
 * - \ref tags::node_store defines a sequence of tags and types for storing persistent data (defaults to the empty sequence).
 * - \ref tags::clock_type defines a clock type (defaults to `std::chrono::system_clock`)
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::name associates to the main name of a component composition instance (defaults to the empty string).
 * - \ref tags::output associates to an output stream for logging (defaults to `std::cout`).
 * - \ref tags::plotter associates to a pointer to a plotter object (defaults to `nullptr`).
 *
 * Admissible values for \ref tags::output are:
 * - a pointer to a stream (as `std::ostream*`);
 * - a file name (as `std::string` or `char const*`);
 * - a directory name ending in `/` or `\`, to which a generated file name will be appended (starting with \ref tags::name followed by a representation of the whole initialisation parameters of the net instance).
 */
template <class... Ts>
struct hardware_logger {
    //! @brief Tagged tuple type for storing extra info.
    using extra_info_type = common::tagged_tuple_t<common::option_types<tags::extra_info, Ts...>>;

    //! @brief Type of the plotter object.
    using plot_type = common::option_type<tags::plot_type, plot::none, Ts...>;

    //! @brief Type of the output stream.
    using ostream_type = common::option_type<tags::ostream_type, std::ostream, Ts ...>;

    //! @brief Type of the clock object.
    using clock_t = common::option_type<tags::clock_type, std::chrono::system_clock, Ts ...>;

    //! @brief Sequence of tags and types for storing persistent data.
    using node_store_type = common::storage_list<common::option_types<tags::node_store, Ts...>>;

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
        DECLARE_COMPONENT(logger);
        REQUIRE_COMPONENT(logger,storage);
        //! @endcond

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            //! @brief Tuple type of the contents.
            using node_tuple_type = common::tagged_tuple_t<node_store_type>;

            //! @brief Type for the result of an aggregation.
            using row_type = common::tagged_tuple_cat<common::tagged_tuple_t<plot::time, times_t>, node_tuple_type, extra_info_type>;

            //! @brief Sequence of tags to be printed.
            using tag_type = typename node_tuple_type::tags;

            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_stream(details::make_stream<ostream_type>(common::get_or<tags::output>(t, &std::cout), t)), m_plotter(common::get_or<tags::plotter>(t, nullptr), [] (void*) {}), m_extra_info(t) {
                if (m_stream != nullptr) {
                    std::time_t time = clock_t::to_time_t(clock_t::now());
                    std::string tstr = std::string(ctime(&time));
                    tstr.pop_back();
                    *m_stream << "########################################################\n";
                    *m_stream << "# FCPP execution started at:  " << tstr << " #\n";
                    *m_stream << "########################################################\n# ";
                    t.print(*m_stream, common::assignment_tuple, common::skip_tags<tags::name,tags::output,tags::plotter>);
                    *m_stream << "\n#\n";
                    *m_stream << "# The columns have the following meaning:\n# time ";
                    print_headers(tag_type{});
                    *m_stream << "0 ";
                    print_output(tag_type{});
                }
            }

            //! @brief Destructor printing an export end section.
            ~node() {
                if (m_stream != nullptr) {
                    std::time_t time = clock_t::to_time_t(clock_t::now());
                    std::string tstr = std::string(ctime(&time));
                    tstr.pop_back();
                    *m_stream << "########################################################\n";
                    *m_stream << "# FCPP execution finished at: " << tstr << " #\n";
                    *m_stream << "########################################################" << std::endl;
                }
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                if (m_stream != nullptr) {
                    *m_stream << t << " " << std::flush;
                }
                P::node::round_start(t);
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                if (m_stream != nullptr) {
                    print_output(tag_type{});
                }
                if (m_plotter != nullptr)
                    data_plotter(std::is_same<plot_type, plot::none>{}, t);
            }

          private: // implementation details
            //! @brief Prints the storage headers.
            void print_headers(common::type_sequence<>) const {
                *m_stream << std::endl;
            }
            template <typename U, typename... Us>
            void print_headers(common::type_sequence<U,Us...>) const {
                *m_stream << common::strip_namespaces(common::type_name<U>()) << " ";
                print_headers(common::type_sequence<Us...>{});
            }

            //! @brief Prints the storage values.
            void print_output(common::type_sequence<>) const {
                *m_stream << std::endl;
            }
            template <typename U, typename... Us>
            void print_output(common::type_sequence<U,Us...>) const {
                *m_stream << common::escape(P::node::storage(U{})) << " ";
                print_output(common::type_sequence<Us...>{});
            }

            //! @brief Plots data if a plotter is given.
            inline void data_plotter(std::false_type, times_t t) const {
                row_type r = m_extra_info;
                common::get<plot::time>(r) = t;
                r = P::node::storage_tuple();
                *m_plotter << r;
            }
            inline void data_plotter(std::true_type, times_t) const {}

            //! @brief The stream where data is exported.
            std::shared_ptr<ostream_type> m_stream;

            //! @brief A reference to a plotter object.
            std::shared_ptr<plot_type> m_plotter;

            //! @brief Tuple storing extra information.
            extra_info_type m_extra_info;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_DEPLOYMENT_HARDWARE_LOGGER_H_
