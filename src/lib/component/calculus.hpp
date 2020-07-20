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

#include "lib/internal/context.hpp"
#include "lib/internal/trace.hpp"
#include "lib/internal/twin.hpp"
#include "lib/option/metric.hpp"


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
    //! @brief Declaration tag associating to a sequence of types to be used in exports.
    template <typename... Ts>
    struct exports {};

    //! @brief Declaration tag associating to a callable class to be executed during rounds.
    template <typename T>
    struct program {};

    //! @brief Declaration tag associating to a metric class regulating the discard of exports.
    template <typename T>
    struct retain {};

    //! @brief Declaration flag associating to whether exports are wrapped in smart pointers.
    template <bool b>
    struct export_pointer {};

    //! @brief Declaration flag associating to whether exports for neighbours are split from those for self.
    template <bool b>
    struct export_split {};

    //! @brief Declaration flag associating to whether messages are dropped as they arrive (reduces memory footprint).
    template <bool b>
    struct online_drop {};

    //! @brief Node initialisation tag associating to the maximum size for a neighbourhood.
    struct hoodsize {};

    //! @brief Node initialisation tag associating to a threshold regulating discard of old messages.
    struct threshold {};
}


/**
 * @brief Component providing the field calculus APIs.
 *
 * Must be unique in a composition of components.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::exports defines a sequence of types to be used in exports (defaults to the empty sequence).
 * - \ref tags::program defines a callable class to be executed during rounds (defaults to \ref calculus::null_program).
 * - \ref tags::retain defines a metric class regulating the discard of exports (defaults to \ref metric::once).
 *
 * <b>Declaration flags:</b>
 * - \ref tags::export_pointer defines whether exports are wrapped in smart pointers (defaults to \ref FCPP_EXPORT_PTR).
 * - \ref tags::export_split defines whether exports for neighbours are split from those for self (defaults to \ref FCPP_EXPORT_NUM `== 2`).
 * - \ref tags::online_drop defines whether messages are dropped as they arrive (defaults to \ref FCPP_ONLINE_DROP).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::hoodsize associates to the maximum number of neighbours allowed (defaults to `std::numeric_limits<device_t>::%max()`).
 * - \ref tags::threshold associates to a `T::result_type` threshold (where `T` is the class specified with \ref tags::retain) regulating discarding of old messages (defaults to the result of `T::build()`).
 *
 * Retain classes should (see \ref metric for a list of available ones):
 * - provide a `result_type` type member which has to be totally ordered, for example:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   using result_type = double;
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to build a default `result_type` to be used as threshold:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   result_type build();
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to build a `result_type` from a `tagged_tuple` message possibly using node data:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   template <typename N, typename S, typename T>
 *   result_type build(const N& node, times_t t, device_t d, const common::tagged_tuple<S,T>& m);
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to update by comparing a `result_type` with node data:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   template <typename N>
 *   result_type update(const result_type&, const N& node);
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * Round classes should be default-constructible and be callable with the following signature:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * template <typename node_t>
 * void operator()(node_t& node, times_t t);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <class... Ts>
struct calculus {
    //! @brief Callable class performing no operation.
    struct null_program {
        //! @brief Call operator doing nothing.
        template <typename node_t>
        void operator()(node_t&, times_t) {}
    };

    //! @brief Callable class to be executed during rounds.
    using program_type = common::option_type<tags::program, null_program, Ts...>;

    //! @brief Metric class regulating the discard of exports.
    using retain_type = common::option_type<tags::retain, metric::once, Ts...>;

    //! @brief Sequence of types to be used in exports.
    using exports_type = common::option_types<tags::exports, Ts...>;

    //! @brief Whether exports are wrapped in smart pointers.
    constexpr static bool export_pointer = common::option_flag<tags::export_pointer, FCPP_EXPORT_PTR, Ts...>;

    //! @brief Whether exports for neighbours are split from those for self.
    constexpr static bool export_split = common::option_flag<tags::export_split, FCPP_EXPORT_NUM == 2, Ts...>;

    //! @brief Whether messages are dropped as they arrive.
    constexpr static bool online_drop = common::option_flag<tags::online_drop, FCPP_ONLINE_DROP, Ts...>;

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
            using metric_type = typename retain_type::result_type;

            //! @brief The type of the context of exports from other devices.
            using context_type = internal::context_t<online_drop, export_pointer, metric_type, exports_type>;

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
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_context{}, m_metric{}, m_hoodsize{common::get_or<tags::hoodsize>(t, std::numeric_limits<device_t>::max())}, m_threshold{common::get_or<tags::threshold>(t, m_metric.build())} {}

            //! @brief Number of neighbours (including self).
            size_t size() const {
                return m_context.second().size(P::node::uid);
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                assert(stack_trace.empty());
                m_context.second().freeze(m_hoodsize, P::node::uid);
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
                m_context.second().unfreeze(P::node::as_final(), m_metric, m_threshold);
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t t, device_t d, const common::tagged_tuple<S,T>& m) {
                P::node::receive(t, d, m);
                m_context.second().insert(d, common::get<calculus_tag>(m), m_metric.build(P::node::as_final(), t, d, m), m_threshold, m_hoodsize);
                if (export_split and d == P::node::uid)
                    m_context.first().insert(d, m_export.first(), m_metric.build(P::node::as_final(), t, d, m), m_threshold, m_hoodsize);
            }

            //! @brief Produces a message to send to a target, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t t, device_t d, common::tagged_tuple<S,T>& m) const {
                P::node::send(t, d, m);
                common::get<calculus_tag>(m) = m_export.second();
                return m;
            }

            //! @brief Stack trace maintained during aggregate function execution.
            internal::trace stack_trace;

          private: // implementation details
            //! @brief Map associating devices to their exports (`first` for local device, `second` for others).
            internal::twin<context_type, not export_split> m_context;

            //! @brief Exports of the current device (`first` for local device, `second` for others).
            internal::twin<export_type, not export_split> m_export;

            //! @brief The callable class representing the main round.
            program_type m_callback;

            //! @brief The metric class.
            retain_type m_metric;

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
//! @brief Computes the restriction of a local to the current domain.
template <typename node_t, typename A, typename = if_local<A>>
A align(const node_t&, trace_t, A&& x) {
    return std::forward<A>(x);
}

//! @brief Computes the restriction of a field to the current domain.
template <typename node_t, typename A, typename = if_field<A>>
decltype(auto) align(node_t& node, trace_t call_point, A&& x) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::align(std::forward<A>(x), details::get_context(node).second().align(t, node.uid));
}

//! @brief Accesses the local value of a field.
template <typename node_t, typename A>
to_local<A const&> self(const node_t& node, trace_t, A const& x) {
    return details::self(x, node.uid);
}

//! @brief Accesses the local value of a field (moving).
template <typename node_t, typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
to_local<A&&> self(const node_t& node, trace_t, A&& x) {
    return details::self(std::move(x), node.uid);
}

//! @brief Accesses a given value of a field.
template <typename node_t, typename A>
to_local<A const&> self(const node_t&, trace_t, A const& x, device_t uid) {
    return details::self(x, uid);
}

//! @brief Accesses a given value of a field (moving).
template <typename node_t, typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
to_local<A&&> self(const node_t&, trace_t, A&& x, device_t uid) {
    return details::self(std::move(x), uid);
}

//! @brief Returns the local value of a field (modifiable).
template <typename node_t, typename A>
to_local<A&> mod_self(const node_t& node, trace_t, A& x) {
    return details::self(x, node.uid);
}

//! @brief Modifies the local value of a field.
template <typename node_t, typename A, typename B>
to_field<std::decay_t<A>> mod_self(const node_t& node, trace_t, A&& x, B&& y) {
    return details::mod_self(std::forward<A>(x), std::forward<B>(y), node.uid);
}

//! @brief Accesses the default value of a field.
template <typename node_t, typename A>
to_local<A const&> other(const node_t&, trace_t, A const& x) {
    return details::other(x);
}

//! @brief Accesses the default value of a field (moving).
template <typename node_t, typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
to_local<A&&> other(const node_t&, trace_t, A&& x) {
    return details::other(std::move(x));
}

//! @brief Returns the default value of a field (modifiable, ensuring alignment).
template <typename node_t, typename A, typename = if_field<A>>
to_local<A&> mod_other(node_t& node, trace_t call_point, A& x) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::other(details::align_inplace(x, details::get_context(node).second().align(t, node.uid)));
}

//! @brief Modifies the local value of a field (ensuring alignment).
template <typename node_t, typename A, typename B>
to_field<std::decay_t<A>> mod_other(const node_t& node, trace_t call_point, A const& x, B const& y) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::mod_other(x, y, details::get_context(node).second().align(t, node.uid));
}

//! @brief Reduces a field to a single value by a binary operation.
template <typename node_t, typename O, typename A>
auto fold_hood(node_t& node, trace_t call_point, O&& op, const A& a) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::fold_hood(op, a, details::get_context(node).second().align(t, node.uid));
}

//! @brief Reduces a field to a single value by a binary operation with a default value for self.
template <typename node_t, typename O, typename A, typename B>
auto fold_hood(node_t& node, trace_t call_point, O&& op, const A& a, const B& b) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::fold_hood(op, a, b, details::get_context(node).second().align(t, node.uid), node.uid);
}

//! @brief Computes the number of neighbours aligned to the current call point.
template <typename node_t>
size_t count_hood(node_t& node, trace_t call_point) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t);
    return details::get_context(node).second().align(t, node.uid).size();
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
    details::get_export(node).first()->insert(t, align(node, call_point, f));
    return details::get_context(node).first().old(t, f, node.uid);
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
    details::get_export(node).first()->insert(t, align(node, call_point, f));
    return details::get_context(node).first().old(t, f0, node.uid);
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
    A f = op(details::get_context(node).first().old(t, f0, node.uid));
    details::get_export(node).first()->insert(t, align(node, call_point, f));
    return f;
}
/**
 * @brief The previous-round value (defaults to first argument), modified through the second argument.
 *
 * Applies if the \p op argument has return type `std::pair<B,A>`.
 * The first element of the returned pair is returned by the function.
 * The second element of the returned pair is written in the exports.
 */
template <typename node_t, typename A, typename B, typename G, typename = common::if_signature<G, std::pair<B,A>(const A&)>>
B old(node_t& node, trace_t call_point, const A& f0, const B&, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    std::pair<B,A> f = op(details::get_context(node).first().old(t, f0, node.uid));
    details::get_export(node).first()->insert(t, align(node, call_point, std::move(f.second)));
    return f.first;
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
to_field<A> nbr(node_t& node, trace_t call_point, const A& f) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t, align(node, call_point, f));
    return details::get_context(node).second().nbr(t, f, node.uid);
}
/**
 * @brief The neighbours' value of the second argument, defaulting to the first argument.
 *
 * Equivalent to:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * nbr(f0, [](to_field<A> fn){
 *     return std::make_pair(fn, f);
 * })
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <typename node_t, typename A>
to_field<A> nbr(node_t& node, trace_t call_point, const A& f0, const A& f) {
    trace_t t = node.stack_trace.hash(call_point);
    details::get_export(node).second()->insert(t, align(node, call_point, f));
    return details::get_context(node).second().nbr(t, f0, node.uid);
}
/**
 * @brief The neighbours' value of the result (defaults to first argument), modified through the second argument.
 *
 * Applies if the \p op argument has return type `A`.
 * Equivalent to:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * nbr(f0, [](to_field<A> fn){
 *     A f = op(fn);
 *     return std::make_pair(f, f);
 * })
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <typename node_t, typename A, typename G, typename = common::if_signature<G, A(to_field<A>)>>
A nbr(node_t& node, trace_t call_point, const A& f0, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    A f = op(details::get_context(node).second().nbr(t, f0, node.uid));
    details::get_export(node).second()->insert(t, align(node, call_point, f));
    return f;
}
/**
 * @brief The neighbours' value (defaults to first argument), modified through the second argument.
 *
 * Applies if the \p op argument has return type `std::pair<B,A>`.
 * The first element of the returned pair is returned by the function.
 * The second element of the returned pair is written in the exports.
 */
template <typename node_t, typename A, typename B, typename G, typename = common::if_signature<G, std::pair<B,A>(to_field<A>)>>
B nbr(node_t& node, trace_t call_point, const A& f0, const B&, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    std::pair<B,A> f = op(details::get_context(node).second().nbr(t, f0, node.uid));
    details::get_export(node).second()->insert(t, align(node, call_point, std::move(f.second)));
    return f.first;
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
 * oldnbr(f0, [](const A& fo, to_field<A> fn){
 *     A f = op(fo, fn);
 *     return std::make_pair(f, f);
 * })
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <typename node_t, typename A, typename G, typename = common::if_signature<G, A(const A&, to_field<A>)>>
A oldnbr(node_t& node, trace_t call_point, const A& f0, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    A f = op(details::get_context(node).second().old(t, f0, node.uid), details::get_context(node).second().nbr(t, f0, node.uid));
    details::get_export(node).second()->insert(t, align(node, call_point, f));
    return f;
}
/**
 * @brief The result of the second argument given info from neighbours' and self.
 *
 * Applies if the \p op argument has return type `std::pair<B,A>`.
 * The first element of the returned pair is returned by the function.
 * The second element of the returned pair is written in the exports.
 */
template <typename node_t, typename A, typename B, typename G, typename = common::if_signature<G, std::pair<B,A>(const A&, to_field<A>)>>
B oldnbr(node_t& node, trace_t call_point, const A& f0, const B&, G&& op) {
    trace_t t = node.stack_trace.hash(call_point);
    std::pair<B,A> f = op(details::get_context(node).second().old(t, f0, node.uid), details::get_context(node).second().nbr(t, f0, node.uid));
    details::get_export(node).second()->insert(t, align(node, call_point, std::move(f.second)));
    return f.first;
}
//! @}


}

#endif // FCPP_COMPONENT_CALCULUS_H_
