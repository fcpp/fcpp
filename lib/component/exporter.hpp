// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file exporter.hpp
 * @brief Implementation of the `exporter` component logging summarisations of nodes.
 */

#ifndef FCPP_COMPONENT_EXPORTER_H_
#define FCPP_COMPONENT_EXPORTER_H_

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
#include "lib/common/distribution.hpp"
#include "lib/common/mutex.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"
#include "lib/component/identifier.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Tag associating to the main name of an object.
    struct name {};

    //! @brief Tag associating to an output stream.
    struct output {};
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
    //! @brief Makes a stream reference to `std::cout` from a null pointer.
    template <typename S, typename T>
    std::shared_ptr<std::ostream> make_stream(nullptr_t, const common::tagged_tuple<S,T>& t) {
        return make_stream(&std::cout, t);
    }
}
//! @endcond


/**
 * @brief Component logging summarisations of nodes.
 *
 * Initialises `net` with tag `output` associating to the output stream where exported data should be written.
 * Admissible values are:
 * <ul>
 * <li>`nullptr`, which is interpreted as `std::cout` (default);</li>
 * <li>a pointer to a stream `std::ostream*`;</li>
 * <li>a file name (as `std::string` or `const char*`);</li>
 * <li>a directory name ending in `/` or `\`, to which a generated file name will be appended (starting with a name set by tag `name`).</li>
 * </ul>
 * Must be unique in a composition of components.
 * Requires a `storage` parent component.
 * If a `randomizer` parent component is not found, `crand` is used as random generator.
 * If `push` is false, it also requires an `identifier` parent component and can be initialised with `threads`.
 * If `push` is true, all aggregators need to support erasing.
 *
 * @param push  If true, updates are pushed immediately to aggregators, otherwise are pulled when needed.
 * @param G     A sequence generator type scheduling writing of data.
 * @param Ss    The sequence of storage tags and corresponding aggregators (intertwined).
 */
template <bool push, typename G, typename... Ss>
struct exporter {
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
        //! @brief Marks that an exporter component is present.
        struct exporter_tag {};
        
        //! @brief Checks if T has a `exporter_tag`.
        template <typename T, typename = int>
        struct has_etag : std::false_type {};
        template <typename T>
        struct has_etag<T, std::conditional_t<true,int,typename T::exporter_tag>> : std::true_type {};
        
        //! @brief Asserts that P has no `exporter_tag`.
        static_assert(not has_etag<P>::value, "cannot combine multiple exporter components");
        
        //! @brief Checks if T has a `storage_tag`.
        template <typename T, typename = int>
        struct has_stag : std::false_type {};
        template <typename T>
        struct has_stag<T, std::conditional_t<true,int,typename T::storage_tag>> : std::true_type {};
        
        //! @brief Asserts that P has a `storage_tag`.
        static_assert(has_stag<P>::value, "missing storage parent for exporter component");
        
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
        static_assert(push or has_itag<P>::value, "missing identifier parent for exporter component");

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
                if (push) P::node::net.aggregator_insert(P::node::storage_tuple());
            }
            
            //! @brief Destructor erasing values from aggregators.
            ~node() {
                if (push) P::node::net.aggregator_erase(P::node::storage_tuple());
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                if (push) P::node::net.aggregator_erase(P::node::storage_tuple());
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                if (push) P::node::net.aggregator_insert(P::node::storage_tuple());
            }
        };
        
        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) : P::net(t), m_stream(details::make_stream(common::get_or<tags::output>(t, nullptr), t)), m_schedule(get_generator(common::bool_pack<has_rtag<P>::value>(), *this),t), m_threads(common::get_or<tags::threads>(t, FCPP_THREADS)) {
                std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                std::string tstr = std::string(ctime(&time));
                tstr.pop_back();
                *m_stream << "##########################################################\n";
                *m_stream << "# FC++ data export started at:  " << tstr << " #\n";
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
                *m_stream << "# FC++ data export finished at: " << tstr << " #\n";
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
                    *m_stream << m_schedule.next() << " ";
                    m_schedule.step(get_generator(common::bool_pack<has_rtag<P>::value>(), *this));
                    data_puller(common::bool_pack<not push>(), *this);
                    print_output(t_tags());
                    *m_stream << std::endl;
                    if (not push) m_aggregators = common::tagged_tuple_t<Ss...>();
                } else P::net::update();
            }
            
            //! @brief Erases data from the aggregators.
            template <typename S, typename T>
            void aggregator_erase(const common::tagged_tuple<S,T>& t) {
                assert(push); // disabled for pull-based exporters
                common::lock_guard<push and FCPP_PARALLEL> lock(m_aggregators_mutex);
                aggregator_erase_impl(m_aggregators, t, t_tags());
            }
            
            //! @brief Inserts data into the aggregators.
            template <typename S, typename T>
            void aggregator_insert(const common::tagged_tuple<S,T>& t) {
                assert(push); // disabled for pull-based exporters
                common::lock_guard<push and FCPP_PARALLEL> lock(m_aggregators_mutex);
                aggregator_insert_impl(m_aggregators, t, t_tags());
            }
            
          private: // implementation details
            //! @brief The tagged tuple tags.
            using t_tags = typename common::tagged_tuple_t<Ss...>::tags;
            
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
            inline random::crand get_generator(common::bool_pack<false>, N&) {
                return random::crand();
            }
            
            //! @brief Collects data actively from nodes if `identifier` is available.
            template <typename N>
            inline void data_puller(common::bool_pack<true>, N& n) {
                if (m_threads == 1) {
                    for (auto it = n.node_begin(); it != n.node_end(); ++it)
                        aggregator_insert_impl(m_aggregators, it->second.storage_tuple(), t_tags());
                    return;
                }
                std::vector<common::tagged_tuple_t<Ss...>> thread_aggregators(m_threads);
                auto a = n.node_begin();
                auto b = n.node_end();
                common::parallel_for(common::tags::general_execution<FCPP_PARALLEL>(m_threads), b-a, [&thread_aggregators,&a,this] (size_t i, size_t t) {
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
            G m_schedule;
            
            //! @brief The aggregator tuple.
            common::tagged_tuple_t<Ss...> m_aggregators;
            
            //! @brief A mutex for accessing aggregation.
            common::mutex<push and FCPP_PARALLEL> m_aggregators_mutex;
            
            //! @brief The number of threads to be used.
            const size_t m_threads;
        };
    };
};


}


}

#endif // FCPP_COMPONENT_EXPORTER_H_
