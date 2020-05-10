// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file calculus.hpp
 * @brief Implementation of the `calculus` component providing the field calculus APIs.
 */

#ifndef FCPP_COMPONENT_CALCULUS_H_
#define FCPP_COMPONENT_CALCULUS_H_

#include <cassert>

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


//! @cond INTERNAL
namespace details {
    //! @brief Accessess the context of a node.
    template <typename node_t>
    auto& get_context(node_t& node) {
        return node.m_context;
    }

    //! @brief Accessess the exports of a node.
    template <typename node_t>
    auto& get_export(node_t& node) {
        return node.m_export;
    }
}
//! @endcond


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
 * The \p C class should be default-constructible and be callable with the following signature:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * template <typename node_t>
 * void operator()(node_t& node, times_t t);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * The \p M class should:
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
 * @param C  Callable class representing the main round execution.
 * @param M  Templated class realising a metric on exports.
 * @param Ts Types to be included in the exports.
 */
template <class C, template<class> class M, class... Ts>
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
            //! @cond INTERNAL
            //! @brief Friendship declarations.
            template <typename node_t>
            friend auto& fcpp::details::get_context(node_t& node);
            template <typename node_t>
            friend auto& fcpp::details::get_export(node_t& node);
            //! @endcond

          private: // implementation details
            //! @brief The type of the metric on exports.
            using metric_type = typename M<typename F::node>::result_type;

            //! @brief The type of the context of exports from other devices.
            using context_type = data::context<metric_type, Ts...>;
            
            //! @brief The type of the exports of the current device.
            using export_type = typename context_type::export_type;
            
          public: // visible by net objects and the main program
            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = typename P::node::message_t::template push_back<calculus_tag, export_type>;

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
                return m_context.second().size();
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                assert(stack_trace.empty());
                m_export = {};
            }
            
            //! @brief Performs computations at round middle with current time `t`.
            void round_main(times_t t) {
                P::node::round_main(t);
                m_callback(P::node::as_final(), t);
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                assert(stack_trace.empty());
                P::node::round_end(t);
                // recomputes metrics, cleaning obsolete values
                std::unordered_map<device_t, metric_type> new_metrics;
                for (const auto& x : m_context.second().metrics())
                    new_metrics[x.first] = m_metric.update(x.second, P::node::as_final());
                for (const auto& x : new_metrics)
                    m_context.second().insert(x.first, x.second);
                while (not m_context.second().empty() and m_context.second().top() > m_threshold)
                    m_context.second().pop();
            }
            
            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t t, device_t d, const common::tagged_tuple<S,T>& m) {
                P::node::receive(t, d, m);
                m_context.second().insert(d, common::get<calculus_tag>(m), m_metric.build(P::node::as_final(), t, d, m));
                if (m_context.second().size() > m_hoodsize) m_context.second().pop();
                if (FCPP_EXPORTS == 2 and d == P::node::uid)
                    m_context.first().insert(d, m_export.first(), metric_type{});
            }
            
            //! @brief Produces a message to send to a target, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t t, device_t d, common::tagged_tuple<S,T>& m) const {
                P::node::send(t, d, m);
                common::get<calculus_tag>(m) = m_export.second();
                return m;
            }

            //! @brief Stack trace maintained during aggregate function execution.
            data::trace stack_trace;
            
          private: // implementation details
            //! @brief Map associating devices to their exports (`first` for local device, `second` for others).
            common::twin<context_type, FCPP_EXPORTS == 1> m_context;
            
            //! @brief Exports of the current device (`first` for local device, `second` for others).
            common::twin<export_type,  FCPP_EXPORTS == 1> m_export;
            
            //! @brief The callable class representing the main round.
            C m_callback;

            //! @brief The metric class.
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


//! @name field operators
//! @{
//! @brief Selects the local value of a field.
template <typename node_t, typename A>
to_local<A&&> self(const node_t& node, trace_t, A&& x) {
    return details::self(std::forward<A>(x), node.uid);
}

//! @brief Read access to the default value of a local.
template <typename node_t, typename A, typename = if_local<A>>
A const&& other(const node_t&, trace_t, A&& x) {
    return static_cast<A const&&>(x);
}

//! @brief Read access to the default value of a field value.
template <typename node_t, typename A, typename = if_field<A>>
to_local<A&&> other(const node_t&, trace_t, A&& x) {
    return details::other(std::move(x));
}

//! @brief Read access to the default value of a field.
template <typename node_t, typename A, typename = if_field<A>>
to_local<A const&> other(const node_t&, trace_t, A const& x) {
    return details::other(x);
}

//! @brief Write access to the default value of a field, ensuring alignment.
template <typename node_t, typename A, typename = if_field<A>>
to_local<A&> other(node_t& node, trace_t call_point, A& x) {
    return details::other(align(node, call_point, x));
}

//! @brief Computes the restriction of a local to a given domain.
template <typename node_t, typename A, typename = if_local<A>>
A&& align(const node_t&, trace_t, A&& x) {
    return std::forward<A>(x);
}

//! @brief Computes the restriction of a field to a given domain.
template <typename node_t, typename A, typename = if_field<A>>
auto&& align(node_t& node, trace_t call_point, A&& x) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::align(std::forward<A>(x), details::get_context(node).second().align(t));
}

//! @brief Applies an operator pointwise on a sequence of field arguments.
template <typename node_t, typename O, typename... A>
field_result<O,A...> map_hood(node_t& node, trace_t call_point, O&& op, const A&... a) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    field_result<O,A...> r{op(details::other(a)...)};
    for (device_t x : details::get_context(node).second().align(t))
        details::self(r,x) = op(details::self(a,x)...);
    return r;
}

//! @brief Modifies a field in-place, by applying an operator pointwise (with a sequence of parameters).
template <typename node_t, typename O, typename A, typename... B>
A& mod_hood(node_t& node, trace_t call_point, O&& op, A& a, const B&... b) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    details::other(a) = op(details::other(a), details::other(b)...);
    for (device_t x : details::get_context(node).second().align(t))
        details::self(a,x) = op(details::self(a,x), details::self(b,x)...);
    return a;
}

//! @brief Reduces the values in the domain of a field to a single value through a binary operation.
template <typename node_t, typename O, typename A>
local_result<O,A,A> fold_hood(node_t& node, trace_t call_point, O&& op, const A& f) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::fold_hood(op, f, details::get_context(node).second().align(t));
}

//! @brief Reduces the values in the domain of a field to a single value through a binary operation.
template <typename node_t, typename O, typename A, typename B>
local_result<O,A,B> fold_hood(node_t& node, trace_t call_point, O&& op, const A& f, const B& b) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::fold_hood(op, f, b, details::get_context(node).second().align(t), node.uid);
}
//! @}


//! @name old-based coordination operators
//! @{
/**
 * @brief The previous-round value of the argument.
 *
 * Equivalent to `old(f, f)`.
 */
template <typename node_t, typename A>
const A& old(node_t& node, trace_t call_point, const A& f) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).first()->insert(t, f);
    return details::get_context(node).first().old(t, f);
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
template <typename node_t, typename A>
const A& old(node_t& node, trace_t call_point, const A& f0, const A& f) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).first()->insert(t, f);
    return details::get_context(node).first().old(t, f0);
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
template <typename node_t, typename A, typename G, typename = common::if_signature<G, A(const A&)>>
A old(node_t& node, trace_t call_point, const A& f0, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    A f = op(details::get_context(node).first().old(t, f0));
    details::get_export(node).first()->insert(t, f);
    return std::move(f);
}
/**
 * @brief The previous-round value (defaults to first argument), modified through the second argument.
 *
 * Applies if the \p op argument has return type `std::pair<B,A>`.
 * The first element of the returned pair is returned by the function.
 * The second element of the returned pair is written in the exports.
 */
template <typename node_t, typename A, typename B, typename G, typename = common::if_signature<G, std::pair<B,A>(const A&)>>
B old(node_t& node, trace_t call_point, const A& f0, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    std::pair<B,A> f = op(details::get_context(node).first().old(t, f0));
    details::get_export(node).first()->insert(t, std::move(f.second));
    return std::move(f.first);
}
//! @}


//! @name nbr-based coordination operators
//! @{
/**
 * @brief The neighbours' value of the argument.
 *
 * Equivalent to `nbr(f, f)`.
 */
template <typename node_t, typename A>
common::add_template<field, A> nbr(node_t& node, trace_t call_point, const A& f) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t, f);
    return details::get_context(node).second().nbr(t, f);
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
template <typename node_t, typename A>
common::add_template<field, A> nbr(node_t& node, trace_t call_point, const A& f0, const A& f) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t, f);
    return details::get_context(node).second().nbr(t, f0);
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
template <typename node_t, typename A, typename G, typename = common::if_signature<G, A(common::add_template<field, A>)>>
A nbr(node_t& node, trace_t call_point, const A& f0, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    A f = op(details::get_context(node).second().nbr(t, f0));
    details::get_export(node).second()->insert(t, f);
    return std::move(f);
}
/**
 * @brief The neighbours' value (defaults to first argument), modified through the second argument.
 *
 * Applies if the \p op argument has return type `std::pair<B,A>`.
 * The first element of the returned pair is returned by the function.
 * The second element of the returned pair is written in the exports.
 */
template <typename node_t, typename A, typename B, typename G, typename = common::if_signature<G, std::pair<B,A>(common::add_template<field, A>)>>
B nbr(node_t& node, trace_t call_point, const A& f0, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    std::pair<B,A> f = op(details::get_context(node).second().nbr(t, f0));
    details::get_export(node).second()->insert(t, std::move(f.second));
    return std::move(f.first);
}
//! @}


//! @name mixed coordination operators
//! @{
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
template <typename node_t, typename A, typename G, typename = common::if_signature<G, A(const A&, common::add_template<field, A>)>>
A oldnbr(node_t& node, trace_t call_point, const A& f0, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    A f = op(details::get_context(node).second().old(t, f0), details::get_context(node).second().nbr(t, f0));
    details::get_export(node).second()->insert(t, f);
    return std::move(f);
}
/**
 * @brief The result of the second argument given info from neighbours' and self.
 *
 * Applies if the \p op argument has return type `std::pair<B,A>`.
 * The first element of the returned pair is returned by the function.
 * The second element of the returned pair is written in the exports.
 */
template <typename node_t, typename A, typename B, typename G, typename = common::if_signature<G, std::pair<B,A>(const A&, common::add_template<field, A>)>>
B oldnbr(node_t& node, trace_t call_point, const A& f0, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    std::pair<B,A> f = op(details::get_context(node).second().old(t, f0), details::get_context(node).second().nbr(t, f0));
    details::get_export(node).second()->insert(t, std::move(f.second));
    return std::move(f.first);
}
//! @}


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
