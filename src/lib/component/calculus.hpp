// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file calculus.hpp
 * @brief Implementation of the `calculus` component providing the field calculus APIs.
 */

#ifndef FCPP_COMPONENT_CALCULUS_H_
#define FCPP_COMPONENT_CALCULUS_H_

#include <functional>
#include <limits>
#include <unordered_map>

#include "lib/settings.hpp"
#include "lib/common/multitype_map.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"
#include "lib/common/twin.hpp"
#include "lib/data/context.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Tag associating to the maximum size for a neighbourhood.
    struct hoodsize {};

    //! @brief Tag associating to a threshold regulating discard of old messages.
    struct threshold {};
}


/**
 * @brief Component providing the field calculus APIs.
 *
 * Initialises `node` with tags `hoodsize` associating to the maximum number of neighbours allowed (defaults to `std::numeric_limits<device_t>::max()`), and `threshold` associating to a `M::result_type` threshold regulating discarding of old messages (defaults to the result of `M::build()`).
 * Must be unique in a composition of components.
 * The \p M callable class should:
 * - be a template with a class argument (which will be instantiated to the final `node` type):
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   template <typename N> class M { ... };
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - provide a `result_type` type member which has to be totally ordered, for example:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   using result_type = double;
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to build a default `result_type` to be used as threshold:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   result_type build();
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to build a `result_type` from a `tagged_tuple` message:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   template <typename S, typename T>
 *   result_type build(const N& node, times_t t, device_t d, const common::tagged_tuple<S,T>& m);
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to update by comparing a `result_type` with a node data:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   result_type update(const result_type&, const N& node);
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 *   These methods are called to compute the "goodness" of a neighbour export with
 *   respect to data on the current device.
 *
 * @param M  Callable class realising a metric on exports.
 * @param Ts Types to be included in the exports.
 */
template <template<class...> class M, class... Ts>
struct calculus {
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
        //! @brief Marks that a calculus component is present.
        struct calculus_tag {};
        
        //! @brief Checks if T has a `calculus_tag`.
        template <typename T, typename = int>
        struct has_ctag : std::false_type {};
        template <typename T>
        struct has_ctag<T, std::conditional_t<true,int,typename T::calculus_tag>> : std::true_type {};
        
        //! @brief Asserts that P has no `calculus_tag`.
        static_assert(not has_ctag<P>::value, "cannot combine multiple calculus components");

        //! @brief The local part of the component.
        class node : public P::node {
          private: // implementation details
            //! @brief The type of the metric on exports.
            using metric_type = typename M<typename F::node>::result_type;
            
            //! @brief The type of the collection of exports from other devices.
            using context_type = typename context<metric_type, Ts...>::export_type;
            
            //! @brief The type of the exports of the current device (`first` is for the local device, `second` for other devices).
            using export_type = common::twin<common::multitype_map<trace_t, Ts...>, FCPP_EXPORTS == 1>;
            
          public: // visible by net objects and the main program
            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = typename P::node::message_t::template push_back<calculus_tag, context_type>;

            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_context{P::node::uid}, m_metric{}, m_hoodsize{common::get_or<tags::hoodsize>(t, std::numeric_limits<device_t>::max())}, m_threshold{common::get_or<tags::threshold>(t, m_metric.build())} {}
            
            //! @brief Number of neighbours (including self).
            size_t size() const {
                return m_context.size();
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                thread_trace = trace{}; // resets the trace
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t) {
                // recomputes metrics, cleaning obsolete values
                std::unordered_map<device_t, metric_type> new_metrics;
                for (const auto& x : m_context.metrics())
                    new_metrics[x.first] = m_metric.update(x.second, P::node::as_final());
                for (const auto& x : new_metrics)
                    m_context.insert(x.first, x.second);
                while (not m_context.empty() and m_context.top() > m_threshold)
                    m_context.pop();
            }
            
            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t t, device_t d, const common::tagged_tuple<S,T>& m) {
                P::node::receive(t, d, m);
                m_context.insert(d, common::get<calculus_tag>(m), m_metric.build(P::node::as_final(), t, d, m));
                if (m_context.size() > m_hoodsize) m_context.pop();
            }
            
            //! @brief Produces a message to send to a target, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t t, device_t d, common::tagged_tuple<S,T>& m) const {
                P::node::send(t, d, m);
                common::get<calculus_tag>(m) = context_type{m_export.second()};
                return m;
            }

          protected: // visible by node objects only
            /** @brief Stateless class for handling trace update on function call.
             *
             * The intended usage is:
             * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
             * template<trace_t __>
             * T func(...) {
             *     trace_call<__> _;
             *     ...
             * }
             * ... func<___>(...) ...
             * ~~~~~~~~~~~~~~~~~~~~~~~~~
             */
            template<trace_t x>
            struct trace_call {
                //! @brief Constructor (adds element to trace).
                trace_call() {
                    thread_trace.push<x>();
                }
                //! @brief Destructor (removes element from trace).
                ~trace_call() {
                    thread_trace.pop();
                }
            };
            
            /** @brief Stateless class for handling trace update on cycles.
             *
             * The intended usage is:
             * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
             * {
             *     trace_cycle<___> _;
             *     someway_repeating {
             *         _();
             *         ....
             *     }
             * }
             * ~~~~~~~~~~~~~~~~~~~~~~~~~
             */
            template<trace_t x>
            struct trace_cycle {
                //! @brief Constructor (adds a cycle element to trace).
                trace_cycle() {
                    thread_trace.push_cycle<x>();
                }
                //! @brief Destructor (removes all cycle elements from trace).
                ~trace_cycle() {
                    thread_trace.pop_cycle();
                }
                //! @brief Call operator (adds a further element to trace).
                void operator()() {
                    thread_trace.push<x>();
                }
            };
            
            //! @name field operators
            //@{
            //! @brief Selects the local value of a field.
            template <typename A>
            common::del_template<field, A> self(A&& x) const {
                return fcpp::details::self(std::forward<A>(x), m_context.self());
            }
            
            //! @brief Write access to the default value of a field, ensuring alignment.
            template<typename A>
            A& other(field<A>& f) {
                return fcpp::details::other(align(f));
            }

            //! @brief Computes the restriction of a field to a given domain.
            template <trace_t x, typename A>
            A align(A&& f) {
                trace_t t = thread_trace.hash<x>();
                m_export.second().insert(t);
                return fcpp::details::align(std::forward<A>(f), m_context.align(t));
            }
            
            //! @brief Reduces the values in the domain of a field to a single value through a binary operation.
            template <trace_t x, typename O, typename A>
            local_result<O,A,A> fold_hood(O&& op, const A& f) {
                trace_t t = thread_trace.hash<x>();
                m_export.second().insert(t);
                return fcpp::details::fold_hood(op, f, m_context.align(t));
            }
            //@}

            //! @name old-based coordination operators
            //@{
            /**
             * @brief The previous-round value of the argument.
             *
             * Equivalent to `old(f, f)`.
             */
            template <trace_t x, typename A>
            const A& old(const A& f) {
                trace_t t = thread_trace.hash<x>();
                m_export.first().insert(t, f);
                return m_context.old(t, f);
            }
            /**
             * @brief The previous-round value of the second argument, defaulting to the first argument if no previous value.
             *
             * Equivalent to:
             * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
             * old(f0, [](const A& fo){
             *     return std::make_pair(fo, f);
             * })
             * ~~~~~~~~~~~~~~~~~~~~~~~~~
             */
            template <trace_t x, typename A>
            const A& old(const A& f0, const A& f) {
                trace_t t = thread_trace.hash<x>();
                m_export.first().insert(t, f);
                return m_context.old(t, f0);
            }
            /**
             * @brief The previous-round value of the result (defaults to first argument), modified through the second argument.
             *
             * Applies if the \p op argument has return type `A`.
             * Corresponds to the `rep` construct of the field calculus. Equivalent to:
             * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
             * old(f0, [](const A& fo){
             *     A f = op(fo);
             *     return std::make_pair(f, f);
             * })
             * ~~~~~~~~~~~~~~~~~~~~~~~~~
             */
            template <trace_t x, typename A>
            A old(const A& f0, std::function<A(const A&)> op) {
                trace_t t = thread_trace.hash<x>();
                A f = op(m_context.old(t, f0));
                m_export.first().insert(t, f);
                return std::move(f);
            }
            /**
             * @brief The previous-round value (defaults to first argument), modified through the second argument.
             *
             * Applies if the \p op argument has return type `std::pair<B,A>`.
             * The first element of the returned pair is returned by the function.
             * The second element of the returned pair is written in the exports.
             */
            template <trace_t x, typename A, typename B>
            B old(const A& f0, std::function<std::pair<B,A>(const A&)> op) {
                trace_t t = thread_trace.hash<x>();
                std::pair<B,A> f = op(m_context.old(t, f0));
                m_export.first().insert(t, std::move(f.second));
                return std::move(f.first);
            }
            //@}

            //! @name nbr-based coordination operators
            //@{
            /**
             * @brief The neighbours' value of the argument.
             *
             * Equivalent to `nbr(f, f)`.
             */
            template <trace_t x, typename A>
            common::add_template<field, A> nbr(const A& f) {
                trace_t t = thread_trace.hash<x>();
                m_export.second().insert(t, f);
                return m_context.nbr(t, f);
            }
            /**
             * @brief The neighbours' value of the second argument, defaulting to the first argument.
             *
             * Equivalent to:
             * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
             * nbr(f0, [](add_template<field, A> fn){
             *     return std::make_pair(fn, f);
             * })
             * ~~~~~~~~~~~~~~~~~~~~~~~~~
             */
            template <trace_t x, typename A>
            common::add_template<field, A> nbr(const A& f0, const A& f) {
                trace_t t = thread_trace.hash<x>();
                m_export.second().insert(t, f);
                return m_context.nbr(t, f0);
            }
            /**
             * @brief The neighbours' value of the result (defaults to first argument), modified through the second argument.
             *
             * Applies if the \p op argument has return type `A`.
             * Equivalent to:
             * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
             * nbr(f0, [](add_template<field, A> fn){
             *     A f = op(fn);
             *     return std::make_pair(f, f);
             * })
             * ~~~~~~~~~~~~~~~~~~~~~~~~~
             */
            template <trace_t x, typename A>
            A nbr(const A& f0, std::function<A(common::add_template<field, A>)> op) {
                trace_t t = thread_trace.hash<x>();
                A f = op(m_context.nbr(t, f0));
                m_export.second().insert(t, f);
                return std::move(f);
            }
            /**
             * @brief The neighbours' value (defaults to first argument), modified through the second argument.
             *
             * Applies if the \p op argument has return type `std::pair<B,A>`.
             * The first element of the returned pair is returned by the function.
             * The second element of the returned pair is written in the exports.
             */
            template <trace_t x, typename A, typename B>
            B nbr(const A& f0, std::function<std::pair<B,A>(common::add_template<field, A>)> op) {
                trace_t t = thread_trace.hash<x>();
                std::pair<B,A> f = op(m_context.nbr(t, f0));
                m_export.second().insert(t, std::move(f.second));
                return std::move(f.first);
            }
            //@}

            //! @name mixed coordination operators
            //@{
            /**
             * @brief The result of the second argument given info from neighbours' and self.
             *
             * Applies if the \p op argument has return type `A`.
             * Equivalent to:
             * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
             * share(f0, [](const A& fo, add_template<field, A> fn){
             *     A f = op(fo, fn);
             *     return std::make_pair(f, f);
             * })
             * ~~~~~~~~~~~~~~~~~~~~~~~~~
             */
            template <trace_t x, typename A>
            A oldnbr(const A& f0, std::function<A(const A&, common::add_template<field, A>)> op) {
                trace_t t = thread_trace.hash<x>();
                A f = op(m_context.old(t, f0), m_context.nbr(t, f0));
                m_export.second().insert(t, f);
                return std::move(f);
            }
            /**
             * @brief The result of the second argument given info from neighbours' and self.
             *
             * Applies if the \p op argument has return type `std::pair<B,A>`.
             * The first element of the returned pair is returned by the function.
             * The second element of the returned pair is written in the exports.
             */
            template <trace_t x, typename A, typename B>
            B oldnbr(const A& f0, std::function<std::pair<B,A>(const A&, common::add_template<field, A>)> op) {
                trace_t t = thread_trace.hash<x>();
                std::pair<B,A> f = op(m_context.old(t, f0), m_context.nbr(t, f0));
                m_export.second().insert(t, std::move(f.second));
                return std::move(f.first);
            }
            //@}
            
          private: // implementation details
            //! @brief Map associating devices to their exports.
            context<metric_type, Ts...> m_context;
            
            //! @brief Exports of the current device.
            export_type m_export;
            
            //! @brief The metric callable class.
            M<typename F::node> m_metric;
            
            //! @brief Maximum amount of neighbours allowed.
            device_t m_hoodsize;
            
            //! @brief Maximum export metric value allowed.
            metric_type m_threshold;
        };
        
        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


//! @brief Namespace for metrics between messages.
namespace metric {
    /**
     * Metric predicate which clears out everything every round.
     *
     * @param N The node type.
     */
    template <typename N>
    struct once {
        //! @brief The data type.
        using result_type = char;
        
        //! @brief Default threshold.
        result_type build() {
            return 1;
        }
        
        //! @brief Measures an incoming message.
        template <typename S, typename T>
        result_type build(const N& n, times_t, device_t d, const common::tagged_tuple<S,T>&) {
            return d == n.uid ? 0 : 1;
        }
        
        //! @brief Updates an existing measure.
        result_type update(const result_type& r, const N&) {
            return r > 0 ? 2 : 0;
        }
    };
}


}

#endif // FCPP_COMPONENT_CALCULUS_H_
