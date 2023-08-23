// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file calculus.hpp
 * @brief Implementation of the `calculus` component providing the field calculus APIs.
 */

#ifndef FCPP_COMPONENT_CALCULUS_H_
#define FCPP_COMPONENT_CALCULUS_H_

#include <cassert>

#include <limits>
#include <unordered_map>
#include <unordered_set>

#include "lib/common/serialize.hpp"
#include "lib/internal/context.hpp"
#include "lib/internal/trace.hpp"
#include "lib/internal/twin.hpp"
#include "lib/option/metric.hpp"
#include "lib/component/base.hpp"


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


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of types to be used in exports (defaults to the empty sequence).
    template <typename... Ts>
    struct exports {};

    //! @brief Declaration tag associating to a callable class to be executed during rounds (defaults to \ref calculus::null_program).
    template <typename T>
    struct program {};

    //! @brief Declaration tag associating to a metric class regulating the discard of exports (defaults to \ref metric::once).
    template <typename T>
    struct retain {};

    //! @brief Declaration flag associating to whether exports are wrapped in smart pointers (defaults to \ref FCPP_EXPORT_PTR).
    template <bool b>
    struct export_pointer {};

    //! @brief Declaration flag associating to whether exports for neighbours are split from those for self (defaults to \ref FCPP_EXPORT_NUM `== 2`).
    template <bool b>
    struct export_split {};

    //! @brief Declaration flag associating to whether messages are dropped as they arrive (reduces memory footprint, defaults to \ref FCPP_ONLINE_DROP).
    template <bool b>
    struct online_drop {};

    //! @brief Node initialisation tag associating to the maximum number of neighbours allowed (defaults to `std::numeric_limits<device_t>::%max()`).
    struct hoodsize {};

    //! @brief Node initialisation tag associating to a `T::result_type` threshold (where `T` is the class specified with \ref tags::retain) regulating discarding of old messages (defaults to the result of `T::build()`).
    struct threshold {};
}


/**
 * @brief Component providing the field calculus APIs.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::exports defines a sequence of types to be used in exports (defaults to the empty sequence).
 * - \ref tags::program defines a callable class to be executed during rounds (defaults to \ref calculus::null_program).
 * - \ref tags::retain defines a metric class regulating the discard of exports (defaults to \ref metric::once).
 *
 * <b>Declaration flags:</b>
 * - \ref tags::export_pointer defines whether exports are wrapped in smart pointers (defaults to \ref FCPP_EXPORT_PTR).
 * - \ref tags::export_split defines whether exports for neighbours are split from those for self (defaults to \ref FCPP_EXPORT_NUM `== 2`).
 * - \ref tags::online_drop defines whether messages are dropped as they arrive (reduces memory footprint, defaults to \ref FCPP_ONLINE_DROP).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::hoodsize associates to the maximum number of neighbours allowed (defaults to `std::numeric_limits<device_t>::%max()`).
 * - \ref tags::threshold associates to a `T::result_type` threshold (where `T` is the class specified with \ref tags::retain) regulating discarding of old messages (defaults to the result of `T::build()`).
 *
 * Retain classes should (see \ref metric for a list of available ones):
 * - provide a `result_type` type member which has to be totally ordered, for example:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   using result_type = real_t;
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to build a default `result_type` to be used as threshold:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   result_type build();
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to build a `result_type` from a `tagged_tuple` message possibly using node data:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   template <typename N, typename S, typename T>
 *   result_type build(N const& node, times_t t, device_t d, common::tagged_tuple<S,T> const& m);
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~
 * - be able to update by comparing a `result_type` with node data:
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 *   template <typename N>
 *   result_type update(result_type const&, N const& node);
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
    using exports_type = common::export_list<common::option_types<tags::exports, Ts...>>;

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
        //! @cond INTERNAL
        DECLARE_COMPONENT(calculus);
        //! @endcond

        //! @brief The local part of the component.
        class node : public P::node {
            //! @cond INTERNAL
            //! @brief Friendship declarations.
            template <typename node_t>
            friend auto& fcpp::details::get_context(node_t& node);
            template <typename node_t>
            friend auto& fcpp::details::get_export(node_t& node);
            //! @endcond

          public: // visible by net objects and the main program
            //! @brief The type of the metric on exports.
            using metric_type = typename retain_type::result_type;

            //! @brief The type of the context of exports from other devices.
            using context_type = internal::context_t<online_drop, export_pointer, metric_type, exports_type>;

            //! @brief The type of the exports of the current device.
            using export_type = typename context_type::export_type;

            //! @brief Helper type providing access to the context for self-messages.
            template <typename A>
            struct self_context_type {
                //! @brief Inserts a value into the exports.
                inline void insert(A x) {
                    assert(n.m_export.first()->template count<A>(t) == 0);
                    n.m_export.first()->template insert<A>(t, std::move(x));
                }

                //! @brief Accesses old stored values given a default.
                inline A const& old(A const& def) {
                    return n.m_context.first().template old<A>(t, def, n.uid);
                }

              private:
                //! @brief Private constructor.
                self_context_type(node& n, trace_t t) : n(n), t(t) {}
                //! @brief Friendship declaration to allow construction from nodes.
                friend class node;
                //! @brief A reference to the node object.
                node& n;
                //! @brief The current stack trace hash.
                trace_t t;
            };

            //! @brief Helper type providing access to the context for neighbour messages.
            template <typename A>
            struct nbr_context_type {
                //! @brief Inserts a value into the exports.
                inline void insert(A x) {
                    assert(n.m_export.second()->template count<A>(t) == 0);
                    n.m_export.second()->template insert<A>(t, std::move(x));
                }

                //! @brief Accesses old stored values given a default.
                inline A const& old(A const& def) {
                    return n.m_context.second().template old<A>(t, def, n.uid);
                }

                //! @brief Accesses old stored values given a default.
                inline to_field<A> nbr(A const& def) {
                    return n.m_context.second().template nbr<A>(t, def, n.uid);
                }

              private:
                //! @brief Private constructor.
                nbr_context_type(node& n, trace_t t) : n(n), t(t) {}
                //! @brief Friendship declaration to allow construction from nodes.
                friend class node;
                //! @brief A reference to the node object.
                node& n;
                //! @brief The current stack trace hash.
                trace_t t;
            };

            //! @brief Helper type providing access to the context for neighbour call points.
            struct void_context_type {
                //! @brief Accesses the list of devices aligned with the call point.
                inline std::vector<device_t> align() {
                    n.m_export.second()->insert(t);
                    return n.m_context.second().align(t, n.uid);
                }

              private:
                //! @brief Private constructor.
                void_context_type(node& n, trace_t t) : n(n), t(t) {}
                //! @brief Friendship declaration to allow construction from nodes.
                friend class node;
                //! @brief A reference to the node object.
                node& n;
                //! @brief The current stack trace hash.
                trace_t t;
            };

            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = typename P::node::message_t::template push_back<calculus_tag, export_type>;

            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_context{}, m_metric{t}, m_hoodsize{common::get_or<tags::hoodsize>(t, std::numeric_limits<device_t>::max())}, m_threshold{common::get_or<tags::threshold>(t, m_metric.build())} {}

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                assert(stack_trace.empty());
                m_context.second().freeze(m_hoodsize, P::node::uid);
                m_export = {};
                std::vector<device_t> nbr_ids = m_context.second().align(P::node::uid);
                std::vector<device_t> nbr_vals;
                nbr_vals.emplace_back();
                nbr_vals.insert(nbr_vals.end(), nbr_ids.begin(), nbr_ids.end());
                m_nbr_uid = fcpp::details::make_field(std::move(nbr_ids), std::move(nbr_vals));
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
            void receive(times_t t, device_t d, common::tagged_tuple<S,T> const& m) {
                P::node::receive(t, d, m);
                m_context.second().insert(d, common::get<calculus_tag>(m), m_metric.build(P::node::as_final(), t, d, m), m_threshold, m_hoodsize);
                if (export_split and d == P::node::uid)
                    m_context.first().insert(d, m_export.first(), m_metric.build(P::node::as_final(), t, d, m), m_threshold, m_hoodsize);
            }

            //! @brief Produces the message to send, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t t, common::tagged_tuple<S,T>& m) const {
                P::node::send(t, m);
                common::get<calculus_tag>(m) = m_export.second();
                return m;
            }

            //! @brief Total number of neighbours (including self and those not aligned).
            size_t size() const {
                return m_context.second().size(P::node::uid);
            }

            //! @brief Identifiers of the neighbours.
            field<device_t> const& nbr_uid() const {
                return m_nbr_uid;
            }

            //! @brief Accesses the threshold for message retain.
            metric_type message_threshold() const {
                return m_threshold;
            }

            //! @brief Modifies the threshold for message retain.
            void message_threshold(metric_type t) {
                m_threshold = t;
            }

            //! @brief Accesses the context for self-messages.
            template <typename A>
            self_context_type<A> self_context(trace_t call_point) {
                return {*this, stack_trace.hash(call_point)};
            }

            //! @brief Accesses the context for neighbour messages.
            template <typename A>
            nbr_context_type<A> nbr_context(trace_t call_point) {
                return {*this, stack_trace.hash(call_point)};
            }

            //! @brief Accesses the context for neighbour call points.
            void_context_type void_context(trace_t call_point) {
                return {*this, stack_trace.hash(call_point)};
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

            //! @brief Identifiers of the neighbours.
            field<device_t> m_nbr_uid;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


} // component


} // fcpp

#endif // FCPP_COMPONENT_CALCULUS_H_
