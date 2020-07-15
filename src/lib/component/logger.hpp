// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file logger.hpp
 * @brief Implementation of the `logger` component logging summarisations of nodes.
 */

#ifndef FCPP_COMPONENT_LOGGER_H_
#define FCPP_COMPONENT_LOGGER_H_

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

#include "lib/settings.hpp"
#include "lib/common/mutex.hpp"
#include "lib/common/profiler.hpp"
#include "lib/option/aggregator.hpp"
#include "lib/option/sequence.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of storage tags and corresponding aggregator types.
    template <typename... Ts>
    struct aggregators {};

    //! @brief Declaration tag associating to a sequence generator type scheduling writing of data.
    template <typename T>
    struct log_schedule {};

    //! @brief Declaration flag associating to whether parallelism is enabled.
    template <bool b>
    struct parallel;

    //! @brief Declaration flag associating to whether new values are pushed to aggregators or pulled when needed.
    template <bool b>
    struct value_push {};

    //! @brief Net initialisation tag associating to the main name of a component composition instance.
    struct name {};

    //! @brief Net initialisation tag associating to an output stream for logging.
    struct output {};

    //! @brief Net initialisation tag associating to the number of threads that can be created.
    struct threads;
}


//! @cond INTERNAL
namespace details {
    //! @brief Makes a stream reference from a `std::string` path.
    template <typename S, typename T>
    std::shared_ptr<std::ostream> make_stream(const std::string& s, const common::tagged_tuple<S,T>& t) {
        if (s.back() == '/' or s.back() == '\\') {
            std::stringstream ss;
            ss << s;
            std::string name{common::get_or<tags::name>(t, "")};
            if (name.size() > 0)
                ss << name << "_";
            t.print(ss, common::underscore_tuple, common::skip_tags<tags::name,tags::output>);
            ss << ".txt";
            return std::shared_ptr<std::ostream>(new std::ofstream(ss.str()));
        } else return std::shared_ptr<std::ostream>(new std::ofstream(s));
    }
    //! @brief Makes a stream reference from a `const char*` path.
    template <typename S, typename T>
    std::shared_ptr<std::ostream> make_stream(const char* s, const common::tagged_tuple<S,T>& t) {
        return make_stream(std::string(s), t);
    }
    //! @brief Makes a stream reference from a stream pointer.
    template <typename S, typename T>
    std::shared_ptr<std::ostream> make_stream(std::ostream* o, const common::tagged_tuple<S,T>&) {
        return std::shared_ptr<std::ostream>(o, [] (void*) {});
    }
}
//! @endcond


/**
 * @brief Component logging summarisations of nodes.
 *
 * Must be unique in a composition of components.
 * Requires a \ref storage parent component, and also an \ref identifier parent component if \ref tags::value_push is false.
 * If a \ref randomizer parent component is not found, \ref crand is passed to the \ref tags::log_schedule object.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::aggregators defines a sequence of storage tags and corresponding aggregator types (defaults to the empty sequence).
 * - \ref tags::log_schedule defines a sequence generator type scheduling writing of data (defaults to \ref sequence::never).
 *
 * <b>Declaration flags:</b>
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 * - \ref tags::value_push defines whether new values are pushed to aggregators or pulled when needed (defaults to \ref FCPP_VALUE_PUSH).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::name associates to the main name of a component composition instance (defaults to the empty string).
 * - \ref tags::output associates to an output stream for logging (defaults to `std::cout`).
 * - \ref tags::threads associates to the number of threads that can be created (defaults to \ref FCPP_THREADS).
 *
 * If \ref tags::value_push is true, all aggregators need to support erasing and \ref tags::threads is ignored; otherwise, it requires an \ref identifier parent component.
 *
 * Overall, \ref tags::threads is ignored whenever \ref tags::value_push is true or \ref tags::parallel is false.
 *
 * Admissible values for \ref tags::output are:
 * - a pointer to a stream (as `std::ostream*`);
 * - a file name (as `std::string` or `const char*`);
 * - a directory name ending in `/` or `\`, to which a generated file name will be appended (starting with \ref tags::name followed by a representation of the whole initialisation parameters of the net instance).
 */
template <class... Ts>
struct logger {
    //! @brief Sequence of storage tags and corresponding aggregator types.
    using aggregators_type = common::option_types<tags::aggregators, Ts...>;

    //! @brief Sequence generator type scheduling writing of data.
    using schedule_type = common::option_type<tags::log_schedule, sequence::never, Ts...>;

    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief Whether new values are pushed to aggregators or pulled when needed.
    constexpr static bool value_push = common::option_flag<tags::value_push, FCPP_VALUE_PUSH, Ts...>;

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
        //! @brief Marks that a logger component is present.
        struct logger_tag {};

        //! @brief Checks if T has a `logger_tag`.
        template <typename T, typename = int>
        struct has_etag : std::false_type {};
        template <typename T>
        struct has_etag<T, std::conditional_t<true,int,typename T::logger_tag>> : std::true_type {};

        //! @brief Asserts that P has no `logger_tag`.
        static_assert(not has_etag<P>::value, "cannot combine multiple logger components");

        //! @brief Checks if T has a `storage_tag`.
        template <typename T, typename = int>
        struct has_stag : std::false_type {};
        template <typename T>
        struct has_stag<T, std::conditional_t<true,int,typename T::storage_tag>> : std::true_type {};

        //! @brief Asserts that P has a `storage_tag`.
        static_assert(has_stag<P>::value, "missing storage parent for logger component");

        //! @brief Checks if T has a `randomizer_tag`.
        template <typename T, typename = int>
        struct has_rtag : std::false_type {};
        template <typename T>
        struct has_rtag<T, std::conditional_t<true,int,typename T::randomizer_tag>> : std::true_type {};

        //! @brief Checks if T has a `identifier_tag`.
        template <typename T, typename = int>
        struct has_itag : std::false_type {};
        template <typename T>
        struct has_itag<T, std::conditional_t<true,int,typename T::identifier_tag>> : std::true_type {};

        //! @brief Asserts that P has a `identifier_tag`.
        static_assert(value_push or has_itag<P>::value, "missing identifier parent for logger component");

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t) {
                if (value_push) P::node::net.aggregator_insert(P::node::storage_tuple());
            }

            //! @brief Destructor erasing values from aggregators.
            ~node() {
                if (value_push) P::node::net.aggregator_erase(P::node::storage_tuple());
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                if (value_push) P::node::net.aggregator_erase(P::node::storage_tuple());
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                if (value_push) P::node::net.aggregator_insert(P::node::storage_tuple());
            }
        };

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Tuple type of the contents.
            using tuple_type = common::tagged_tuple_t<aggregators_type>;

            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) : P::net(t), m_stream(details::make_stream(common::get_or<tags::output>(t, &std::cout), t)), m_schedule(get_generator(common::bool_pack<has_rtag<P>::value>(), *this),t), m_threads(common::get_or<tags::threads>(t, FCPP_THREADS)) {
                std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::string tstr = std::string(ctime(&time));
                tstr.pop_back();
                *m_stream << "##########################################################\n";
                *m_stream << "# FCPP data export started at:  " << tstr << " #\n";
                *m_stream << "##########################################################\n# ";
                t.print(*m_stream, common::assignment_tuple, common::skip_tags<tags::name,tags::output>);
                *m_stream << "\n#\n";
                *m_stream << "# The columns have the following meaning:\n# time ";
                print_headers(t_tags());
                *m_stream << std::endl;
            }

            //! @brief Destructor printing an export end section.
            ~net() {
                std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::string tstr = std::string(ctime(&time));
                tstr.pop_back();
                *m_stream << "##########################################################\n";
                *m_stream << "# FCPP data export finished at: " << tstr << " #\n";
                *m_stream << "##########################################################" << std::endl;
            }

            /**
             * @brief Returns next event to schedule for the net component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_schedule.next(), P::net::next());
            }

            //! @brief Updates the internal status of net component.
            void update() {
                if (m_schedule.next() < P::net::next()) {
                    PROFILE_COUNT("logger");
                    *m_stream << m_schedule.next() << " ";
                    m_schedule.step(get_generator(common::bool_pack<has_rtag<P>::value>(), *this));
                    data_puller(common::bool_pack<not value_push>(), *this);
                    print_output(t_tags());
                    *m_stream << std::endl;
                    if (not value_push) m_aggregators = tuple_type{};
                } else P::net::update();
            }

            //! @brief Erases data from the aggregators.
            template <typename S, typename T>
            void aggregator_erase(const common::tagged_tuple<S,T>& t) {
                assert(value_push); // disabled for pull-based loggers
                common::lock_guard<value_push and parallel> lock(m_aggregators_mutex);
                aggregator_erase_impl(m_aggregators, t, t_tags());
            }

            //! @brief Inserts data into the aggregators.
            template <typename S, typename T>
            void aggregator_insert(const common::tagged_tuple<S,T>& t) {
                assert(value_push); // disabled for pull-based loggers
                common::lock_guard<value_push and parallel> lock(m_aggregators_mutex);
                aggregator_insert_impl(m_aggregators, t, t_tags());
            }

          private: // implementation details
            //! @brief The tagged tuple tags.
            using t_tags = typename tuple_type::tags;

            //! @brief Prints the aggregator headers.
            void print_headers(common::type_sequence<>) const {}
            template <typename U, typename... Us>
            void print_headers(common::type_sequence<U,Us...>) const {
                common::get<U>(m_aggregators).header(*m_stream, common::details::strip_namespaces(common::type_name<U>()));
                print_headers(common::type_sequence<Us...>());
            }

            //! @brief Prints the aggregator headers.
            void print_output(common::type_sequence<>) const {}
            template <typename U, typename... Us>
            void print_output(common::type_sequence<U,Us...>) const {
                common::get<U>(m_aggregators).output(*m_stream);
                print_output(common::type_sequence<Us...>());
            }

            //! @brief Erases data from the aggregators.
            template <typename S, typename T, typename... Us>
            void aggregator_erase_impl(S& a, const T& t, common::type_sequence<Us...>) {
                common::details::ignore((common::get<Us>(a).erase(common::get<Us>(t)),0)...);
            }

            //! @brief Inserts data into the aggregators.
            template <typename S, typename T, typename... Us>
            void aggregator_insert_impl(S& a,  const T& t, common::type_sequence<Us...>) {
                common::details::ignore((common::get<Us>(a).insert(common::get<Us>(t)),0)...);
            }

            //! @brief Inserts an aggregator data into the aggregators.
            template <typename S, typename T, typename... Us>
            void aggregator_add_impl(S& a,  const T& t, common::type_sequence<Us...>) {
                common::details::ignore((common::get<Us>(a) += common::get<Us>(t))...);
            }

            //! @brief Returns the `randomizer` generator if available.
            template <typename N>
            inline auto& get_generator(common::bool_pack<true>, N& n) {
                return n.generator();
            }

            //! @brief Returns a `crand` generator otherwise.
            template <typename N>
            inline crand get_generator(common::bool_pack<false>, N&) {
                return {};
            }

            //! @brief Collects data actively from nodes if `identifier` is available.
            template <typename N>
            inline void data_puller(common::bool_pack<true>, N& n) {
                if (m_threads == 1) {
                    for (auto it = n.node_begin(); it != n.node_end(); ++it)
                        aggregator_insert_impl(m_aggregators, it->second.storage_tuple(), t_tags());
                    return;
                }
                std::vector<tuple_type> thread_aggregators(m_threads);
                auto a = n.node_begin();
                auto b = n.node_end();
                common::parallel_for(common::tags::general_execution<parallel>(m_threads), b-a, [&thread_aggregators,&a,this] (size_t i, size_t t) {
                    aggregator_insert_impl(thread_aggregators[t], a[i].second.storage_tuple(), t_tags());
                });
                for (size_t i=0; i<m_threads; ++i)
                    aggregator_add_impl(m_aggregators, thread_aggregators[i], t_tags());
            }

            //! @brief Does nothing otherwise.
            template <typename N>
            inline void data_puller(common::bool_pack<false>, N&) {}

            //! @brief The stream where data is exported.
            std::shared_ptr<std::ostream> m_stream;

            //! @brief The scheduling of exporting events.
            schedule_type m_schedule;

            //! @brief The aggregator tuple.
            tuple_type m_aggregators;

            //! @brief A mutex for accessing aggregation.
            common::mutex<value_push and parallel> m_aggregators_mutex;

            //! @brief The number of threads to be used.
            const size_t m_threads;
        };
    };
};


}


}

#endif // FCPP_COMPONENT_LOGGER_H_
