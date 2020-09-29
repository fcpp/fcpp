// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

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
#include <thread>
#include <type_traits>
#include <vector>

#include "lib/component/base.hpp"
#include "lib/component/logger.hpp"
#include "lib/deployment/os.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of tags and types for storing persistent data.
    template <typename... Ts>
    struct tuple_store;
}


/**
 * @brief Component logging summarisations of nodes.
 *
 * Requires a \ref storage parent component.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::tuple_store defines a sequence of tags and types for storing persistent data (defaults to the empty sequence).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::name associates to the main name of a component composition instance (defaults to the empty string).
 * - \ref tags::output associates to an output stream for logging (defaults to `std::cout`).
 *
 * Admissible values for \ref tags::output are:
 * - a pointer to a stream (as `std::ostream*`);
 * - a file name (as `std::string` or `const char*`);
 * - a directory name ending in `/` or `\`, to which a generated file name will be appended (starting with \ref tags::name followed by a representation of the whole initialisation parameters of the net instance).
 */
template <class... Ts>
struct hardware_logger {
    //! @brief Sequence of tags and types for storing persistent data.
    using tuple_store_type = common::option_types<tags::tuple_store, Ts...>;

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
        DECLARE_COMPONENT(logger);
        REQUIRE_COMPONENT(logger,storage);

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            //! @brief Tuple type of the contents.
            using tuple_type = common::tagged_tuple_t<tuple_store_type>;
            
            //! @brief Sequence of tags to be printed.
            using tag_type = typename tuple_type::tags;

            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_stream(details::make_stream(common::get_or<tags::output>(t, &std::cout), t)) {
                std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::string tstr = std::string(ctime(&time));
                tstr.pop_back();
                *m_stream << "########################################################\n";
                *m_stream << "# FCPP execution started at:  " << tstr << " #\n";
                *m_stream << "########################################################\n# ";
                t.print(*m_stream, common::assignment_tuple, common::skip_tags<tags::name,tags::output>);
                *m_stream << "\n#\n";
                *m_stream << "# The columns have the following meaning:\n# time ";
                print_headers(tag_type{});
                *m_stream << "0 ";
                print_output(tag_type{});
            }

            //! @brief Destructor printing an export end section.
            ~node() {
                std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::string tstr = std::string(ctime(&time));
                tstr.pop_back();
                *m_stream << "########################################################\n";
                *m_stream << "# FCPP execution finished at: " << tstr << " #\n";
                *m_stream << "########################################################" << std::endl;
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                *m_stream << t << " " << std::flush;
                P::node::round_start(t);
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                print_output(tag_type{});
            }
            
          private: // implementation details
            //! @brief Prints the aggregator headers.
            void print_headers(common::type_sequence<>) const {
                *m_stream << std::endl;
            }
            template <typename U, typename... Us>
            void print_headers(common::type_sequence<U,Us...>) const {
                *m_stream << common::details::strip_namespaces(common::type_name<U>()) << " ";
                print_headers(common::type_sequence<Us...>{});
            }

            //! @brief Prints the aggregator headers.
            void print_output(common::type_sequence<>) const {
                *m_stream << std::endl;
            }
            template <typename U, typename... Us>
            void print_output(common::type_sequence<U,Us...>) const {
                *m_stream << P::node::storage(U{}) << " ";
                print_output(common::type_sequence<Us...>{});
            }

            //! @brief The stream where data is exported.
            std::shared_ptr<std::ostream> m_stream;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_DEPLOYMENT_HARDWARE_LOGGER_H_