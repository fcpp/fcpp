// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file basics.hpp
 * @brief Collection of field calculus built-in functions.
 */

#ifndef FCPP_COORDINATION_BASICS_H_
#define FCPP_COORDINATION_BASICS_H_

#include "lib/data/field.hpp"
#include "lib/internal/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief The status of an aggregate process in a node.
 *
 * The values mean:
 * - Termination is propagated to neighbour nodes in order to ensure the process ends.
 * - An external node is not part of the aggregate process, and its exports cannot be seen by neighbours (deprecated).
 * - A border node is part of the process, but does not cause the process to expand to neighbours.
 * - An internal node is part of the process and propagates it to neighbours.
 * - Every status may request to return the output or not to the `spawn` caller.
 *
 * Note that `status::output` is provided as a synonym of `status::internal_output`, and
 * `status::x and status::output` equals `status::x_output`.
 */
enum class status : char { terminated, external_deprecated, border, internal, terminated_output, external_output_deprecated, border_output, internal_output, output };

//! @brief String representation of a status.
std::string to_string(status);

//! @brief Printing status.
template <typename O>
O& operator<<(O& o, status s) {
    o << to_string(s);
    return o;
}

//! @brief Merges the output status with another status (undefined for other combinations of statuses).
inline constexpr status operator&&(status x, status y) {
    if (y == status::output) {
        assert(x != status::output);
        return static_cast<status>(static_cast<char>(x) | char(4));
    }
    if (x == status::output) {
        assert(y != status::output);
        return static_cast<status>(static_cast<char>(y) | char(4));
    }
    assert(false);
    return status::output;
}

//! @brief Removes the output status from another status (undefined for other combinations of statuses).
inline constexpr status operator^(status x, status y) {
    if (y == status::output) {
        assert(x != status::output);
        return static_cast<status>(static_cast<char>(x) & char(3));
    }
    if (x == status::output) {
        assert(y != status::output);
        return static_cast<status>(static_cast<char>(y) & char(3));
    }
    assert(false);
    return status::output;
}


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {


//! @name field operators
//! @{

//! @brief Computes the restriction of a local to the current domain.
template <typename node_t, typename A, typename = if_local<A>>
inline A align(node_t const&, trace_t, A&& x) {
    return x;
}

//! @brief Computes the restriction of a field to the current domain.
template <typename node_t, typename A, typename = if_field<A>>
A align(node_t& node, trace_t call_point, A const& x) {
    auto ctx = node.void_context(call_point);
    return fcpp::details::align(x, ctx.align());
}

//! @brief Computes the restriction of a field to the current domain.
template <typename node_t, typename A, typename = if_field<A>, typename = std::enable_if_t<not std::is_reference<A>::value>>
A align(node_t& node, trace_t call_point, A&& x) {
    auto ctx = node.void_context(call_point);
    return fcpp::details::align(std::move(x), ctx.align());
}

//! @brief Computes in-place the restriction of a field to the current domain.
template <typename node_t, typename A, typename = if_local<A>>
inline void align_inplace(node_t const&, trace_t, A&) {}

//! @brief Computes in-place the restriction of a field to the current domain.
template <typename node_t, typename A, typename = if_field<A>>
void align_inplace(node_t& node, trace_t call_point, A& x) {
    auto ctx = node.void_context(call_point);
    fcpp::details::align(x, ctx.align());
}

//! @brief Accesses the local value of a field.
template <typename node_t, typename A>
to_local<A const&> self(node_t const& node, trace_t, A const& x) {
    return fcpp::details::self(x, node.uid);
}

//! @brief Accesses the local value of a field (moving).
template <typename node_t, typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
to_local<A&&> self(node_t const& node, trace_t, A&& x) {
    return fcpp::details::self(std::move(x), node.uid);
}

//! @brief Accesses a given value of a field.
template <typename node_t, typename A>
to_local<A const&> self(node_t const&, trace_t, A const& x, device_t uid) {
    return fcpp::details::self(x, uid);
}

//! @brief Accesses a given value of a field (moving).
template <typename node_t, typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
to_local<A&&> self(node_t const&, trace_t, A&& x, device_t uid) {
    return fcpp::details::self(std::move(x), uid);
}

//! @brief Returns the local value of a field (modifiable).
template <typename node_t, typename A>
to_local<A&> mod_self(node_t const& node, trace_t, A& x) {
    return fcpp::details::self(x, node.uid);
}

//! @brief Modifies the local value of a field.
template <typename node_t, typename A, typename B>
to_field<std::decay_t<A>> mod_self(node_t const& node, trace_t, A&& x, B&& y) {
    return fcpp::details::mod_self(std::forward<A>(x), std::forward<B>(y), node.uid);
}

//! @brief Accesses the default value of a field.
template <typename node_t, typename A>
to_local<A const&> other(node_t const&, trace_t, A const& x) {
    return fcpp::details::other(x);
}

//! @brief Accesses the default value of a field (moving).
template <typename node_t, typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
to_local<A&&> other(node_t const&, trace_t, A&& x) {
    return fcpp::details::other(std::move(x));
}

//! @brief Returns the default value of a field (modifiable, ensuring alignment).
template <typename node_t, typename A, typename = if_field<A>>
to_local<A&> mod_other(node_t& node, trace_t call_point, A& x) {
    auto ctx = node.void_context(call_point);
    return fcpp::details::other(fcpp::details::align_inplace(x, ctx.align()));
}

//! @brief Modifies the local value of a field (ensuring alignment).
template <typename node_t, typename A, typename B>
to_field<std::decay_t<A>> mod_other(node_t& node, trace_t call_point, A const& x, B const& y) {
    auto ctx = node.void_context(call_point);
    return fcpp::details::mod_other(x, y, ctx.align());
}

/**
 * @brief Reduces a field to a single value by a binary operation.
 *
 * The folding operation \p op has to have two arguments of type
 * `to_local<A>`, with the first being the new value to be aggregated
 * and the second being the current accumulated value.
 *
 * The folding operation **may** also have a first `device_t` argument,
 * in which it will receive the id of the device that is currently being
 * added to the aggregation.
 *
 * The folding operation is applied following the order of device ids.
 * The accumulate starts from the value associated to the lowest id,
 * and the operation is applied with values from the second id onwards.
 */
template <typename node_t, typename O, typename A>
auto fold_hood(node_t& node, trace_t call_point, O&& op, A const& a) {
    auto ctx = node.void_context(call_point);
    return fcpp::details::fold_hood(op, a, ctx.align());
}

/**
 * @brief Reduces a field to a single value by a binary operation with a given value for self.
 *
 * The folding operation \p op has to have two arguments, of type
 * `to_local<A>` and `B`, with the first being the new value to be
 * aggregated and the second being the current accumulated value.
 *
 * The folding operation **may** also have a first `device_t` argument,
 * in which it will receive the id of the device that is currently being
 * added to the aggregation.
 *
 * The folding operation is applied following the order of device ids.
 * The accumulate starts from the value of argument \p b, and the
 * operation is applied with values from all ids except for the
 * self id in increasing order.
*/
template <typename node_t, typename O, typename A, typename B>
auto fold_hood(node_t& node, trace_t call_point, O&& op, A const& a, B const& b) {
    auto ctx = node.void_context(call_point);
    return fcpp::details::fold_hood(op, a, b, ctx.align(), node.uid);
}

//! @brief Computes the number of neighbours aligned to the current call point.
template <typename node_t>
size_t count_hood(node_t& node, trace_t call_point) {
    auto ctx = node.void_context(call_point);
    return ctx.align().size();
}

//! @brief Computes the identifiers of neighbours aligned to the current call point.
template <typename node_t>
field<device_t> nbr_uid(node_t& node, trace_t call_point) {
    auto ctx = node.void_context(call_point);
    std::vector<device_t> ids = ctx.align();
    std::vector<device_t> vals;
    vals.emplace_back();
    vals.insert(vals.end(), ids.begin(), ids.end());
    return fcpp::details::make_field(std::move(ids), std::move(vals));
}

//! @}


//! @cond INTERNAL
namespace details {
    template <typename D, typename T>
    struct result_unpack {
        using type = std::enable_if_t<std::is_convertible<D, T>::value, common::type_sequence<T, T>>;
    };
    template <typename D, typename R, typename A>
    struct result_unpack<D, tuple<R, A>> {
        using type = std::conditional_t<
            std::is_convertible<D, tuple<R, A>>::value,
            common::type_sequence<tuple<R, A>, tuple<R, A>>,
            std::enable_if_t<std::is_convertible<D, tuple<R, A>>::value or std::is_convertible<D, A>::value, common::type_sequence<R, A>>
        >;
    };

    template <typename D, typename R, typename A, typename = std::enable_if_t<std::is_convertible<D, A>::value>>
    inline R&& maybe_first(common::type_sequence<D>, tuple<R,A>& t) {
        return std::move(get<0>(t));
    }
    template <typename D, typename R, typename A, typename = std::enable_if_t<std::is_convertible<D, A>::value>>
    inline A&& maybe_second(common::type_sequence<D>, tuple<R,A>& t) {
        return std::move(get<1>(t));
    }
    template <typename D, typename T, typename = std::enable_if_t<std::is_convertible<D, T>::value>>
    inline T&& maybe_first(common::type_sequence<D>, T& x) {
        return std::move(x);
    }
    template <typename D, typename T, typename = std::enable_if_t<std::is_convertible<D, T>::value>>
    inline T& maybe_second(common::type_sequence<D>, T& x) {
        return x;
    }
}
//! @endcond

//! @brief The data type returned by an update function call T given default of type D.
template <typename D, typename T>
using return_result_type = typename details::result_unpack<std::decay_t<D>, std::decay_t<std::result_of_t<T>>>::type::front;

//! @brief The export type written by an update function call T given default of type D.
template <typename D, typename T>
using export_result_type = typename details::result_unpack<std::decay_t<D>, std::decay_t<std::result_of_t<T>>>::type::back;


//! @name old-based coordination operators
//! @{
/**
 * @brief The previous-round value (defaults to first argument), modified through the second argument.
 *
 * Corresponds to the `rep` construct of the field calculus.
 * The \p op argument may return a `A` or a `tuple<R,A>`, where `A` is
 * compatible with the default type `D`. In the latter case,
 * the first element of the returned pair is returned by the function, while
 * the second element of the returned pair is written in the exports.
 */
template <typename node_t, typename D, typename G>
return_result_type<D, G(D)> old(node_t& node, trace_t call_point, D const& f0, G&& op) {
    using A = export_result_type<D, G(D)>;
    auto ctx = node.template self_context<A>(call_point);
    auto f = op(ctx.old(f0));
    ctx.insert(align(node, call_point, details::maybe_second(common::type_sequence<D>{}, f)));
    return details::maybe_first(common::type_sequence<D>{}, f);
}
/**
 * @brief The previous-round value of the second argument, defaulting to the first argument if no previous value.
 *
 * Equivalent to:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * old(f0, [](A fo){
 *     return make_tuple(fo, f);
 * })
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <typename node_t, typename D, typename A, typename = std::enable_if_t<std::is_convertible<D, A>::value>>
A old(node_t& node, trace_t call_point, D const& f0, A const& f) {
    auto ctx = node.template self_context<A>(call_point);
    ctx.insert(align(node, call_point, f));
    return ctx.old(f0);
}
/**
 * @brief The previous-round value of the argument.
 *
 * Equivalent to `old(f, f)`.
 */
template <typename node_t, typename A>
inline A old(node_t& node, trace_t call_point, A const& f) {
    return old(node, call_point, f, f);
}

//! @brief The exports type used by the old construct with message type `T`.
template <typename T>
using old_t = common::export_list<T>;
//! @}


//! @name nbr-based coordination operators
//! @{
/**
 * @brief The neighbours' value of the result (defaults to first argument), modified through the second argument.
 *
 * Corresponds to the `share` construct of the field calculus.
 * The \p op argument may return a `A` or a `tuple<R,A>`, where `A` is
 * compatible with the default type `D`. In the latter case,
 * the first element of the returned pair is returned by the function, while
 * the second element of the returned pair is written in the exports.
 */
template <typename node_t, typename D, typename G>
return_result_type<D, G(to_field<D>)> nbr(node_t& node, trace_t call_point, D const& f0, G&& op) {
    using A = export_result_type<D, G(to_field<D>)>;
    auto ctx = node.template nbr_context<A>(call_point);
    auto f = op(ctx.nbr(f0));
    ctx.insert(details::maybe_second(common::type_sequence<D>{}, f));
    return details::maybe_first(common::type_sequence<D>{}, f);
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
template <typename node_t, typename D, typename A, typename = std::enable_if_t<std::is_convertible<D, A>::value>>
to_field<A> nbr(node_t& node, trace_t call_point, D const& f0, A const& f) {
    auto ctx = node.template nbr_context<A>(call_point);
    ctx.insert(f);
    return ctx.nbr(f0);
}
/**
 * @brief The neighbours' value of the argument.
 *
 * Equivalent to `nbr(f, f)`.
 */
template <typename node_t, typename A>
inline to_field<A> nbr(node_t& node, trace_t call_point, A const& f) {
    return nbr(node, call_point, f, f);
}

//! @brief The exports type used by the nbr construct with message type `T`.
template <typename T>
using nbr_t = common::export_list<T>;
//! @}


//! @name mixed coordination operators
//! @{
/**
 * @brief The result of the second argument given info from neighbours' and self.
 *
 * The \p op argument may return a `A` or a `tuple<B,A>`. In the latter case,
 * the first element of the returned pair is returned by the function, while
 * the second element of the returned pair is written in the exports.
 */
template <typename node_t, typename D, typename G>
return_result_type<D, G(D, to_field<D>)> oldnbr(node_t& node, trace_t call_point, D const& f0, G&& op) {
    using A = export_result_type<D, G(D, to_field<D>)>;
    auto ctx = node.template nbr_context<A>(call_point);
    auto f = op(ctx.old(f0), ctx.nbr(f0));
    ctx.insert(align(node, call_point, details::maybe_second(common::type_sequence<D>{}, f)));
    return details::maybe_first(common::type_sequence<D>{}, f);
}

//! @brief The exports type used by the oldnbr construct with message type `T`.
template <typename T>
using oldnbr_t = common::export_list<T>;
//! @}


//! @brief Executes code independently in a partition of the network based on the value of a given key.
template <typename node_t, typename T, typename G>
auto split(node_t& node, trace_t call_point, T&& key, G&& f) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    internal::trace_key trace_process(node.stack_trace, common::hash_to<trace_t>(std::forward<T>(key)));
    return f();
}
//! @brief The exports type used by the split construct.
using split_t = common::export_list<>;


//! @name aggregate processes operators
//! @{

//! @brief Handles a process, spawning instances of it for every key in the `key_set` and passing general arguments `xs` (overload with boolean status corresponding to `status::internal_output` and `status::border_output`).
template <typename node_t, typename G, typename S, typename... Ts, typename K = typename std::decay_t<S>::value_type, typename T = std::decay_t<std::result_of_t<G(K const&, Ts const&...)>>, typename R = std::decay_t<tuple_element_t<0,T>>, typename B = std::decay_t<tuple_element_t<1,T>>>
std::enable_if_t<std::is_same<B,bool>::value, std::unordered_map<K, R, common::hash<K>>>
spawn(node_t& node, trace_t call_point, G&& process, S&& key_set, Ts const&... xs) {
    using keyset_t = std::unordered_set<K, common::hash<K>>;
    using resmap_t = std::unordered_map<K, R, common::hash<K>>;
    auto ctx = node.template nbr_context<keyset_t>(call_point);
    field<keyset_t> fk = ctx.nbr({});
    // keys to be propagated and terminated
    keyset_t ky(key_set.begin(), key_set.end()), km;
    for (size_t i = 1; i < fcpp::details::get_vals(fk).size(); ++i)
        ky.insert(fcpp::details::get_vals(fk)[i].begin(), fcpp::details::get_vals(fk)[i].end());
    internal::trace_call trace_caller(node.stack_trace, call_point);
    resmap_t rm;
    // run process for every gathered key
    for (K const& k : ky) {
        internal::trace_key trace_process(node.stack_trace, common::hash_to<trace_t>(k));
        bool b;
        tie(rm[k], b) = process(k, xs...);
        // if true status, propagate key to neighbours
        if (b) km.insert(k);
    }
    ctx.insert(km);
    return rm;
}

//! @brief Handles a process, spawning instances of it for every key in the `key_set` and passing general arguments `xs` (overload with field<bool> status).
template <typename node_t, typename G, typename S, typename... Ts, typename K = typename std::decay_t<S>::value_type, typename T = std::decay_t<std::result_of_t<G(K const&, Ts const&...)>>, typename R = std::decay_t<tuple_element_t<0,T>>, typename B = std::decay_t<tuple_element_t<1,T>>>
std::enable_if_t<std::is_same<B,field<bool>>::value, std::unordered_map<K, R, common::hash<K>>>
spawn(node_t& node, trace_t call_point, G&& process, S&& key_set, Ts const&... xs) {
    using keyset_t = std::unordered_set<K, common::hash<K>>;
    using resmap_t = std::unordered_map<K, R, common::hash<K>>;
    auto kctx = node.template nbr_context<keyset_t>(call_point);
    field<keyset_t> fk = kctx.nbr({});
    // keys to be propagated and terminated
    keyset_t ky(key_set.begin(), key_set.end()), km, kstart = ky;
    for (size_t i = 1; i < fcpp::details::get_vals(fk).size(); ++i)
        ky.insert(fcpp::details::get_vals(fk)[i].begin(), fcpp::details::get_vals(fk)[i].end());
    internal::trace_call trace_caller(node.stack_trace, call_point);
    auto any_hood = [](field<bool> const& fb){
        for (bool b : fcpp::details::get_vals(fb)) if (b) return true;
        return false;
    };
    resmap_t rm;
    // run process for every gathered key
    for (K const& k : ky) {
        trace_t kh = common::hash_to<trace_t>(k);
        auto fctx = node.template nbr_context<field<bool>>(kh);
        if (kstart.count(k) == 0 and not any_hood(fctx.nbr(false))) continue;
        internal::trace_key trace_process(node.stack_trace, kh);
        field<bool> fb;
        tie(rm[k], fb) = process(k, xs...);
        // if status is true for something, propagate key to neighbours
        if (any_hood(fb)) {
            km.insert(k);
            fctx.insert(fb);
        }
    }
    kctx.insert(km);
    return rm;
}

/**
 * @brief Handles a process, spawning instances of it for every key in the `key_set` and passing general arguments `xs` (overload with general status).
 *
 * Does not support the "external" status, which is treated equally as "border".
 * Termination propagates causing devices to get into "border" status.
 */
template <typename node_t, typename G, typename S, typename... Ts, typename K = typename std::decay_t<S>::value_type, typename T = std::decay_t<std::result_of_t<G(K const&, Ts const&...)>>, typename R = std::decay_t<tuple_element_t<0,T>>, typename B = std::decay_t<tuple_element_t<1,T>>>
std::enable_if_t<std::is_same<B,status>::value, std::unordered_map<K, R, common::hash<K>>>
spawn(node_t& node, trace_t call_point, G&& process, S&& key_set, Ts const&... xs) {
    using keymap_t = std::unordered_map<K, B, common::hash<K>>;
    using resmap_t = std::unordered_map<K, R, common::hash<K>>;
    auto ctx = node.template nbr_context<keymap_t>(call_point);
    // keys to be propagated and terminated
    std::unordered_set<K, common::hash<K>> ky(key_set.begin(), key_set.end()), kn;
    for (auto const& m : fcpp::details::get_vals(ctx.nbr({})))
        for (auto const& k : m) {
            if (k.second == status::terminated)
                kn.insert(k.first);
            else
                ky.insert(k.first);
        }
    internal::trace_call trace_caller(node.stack_trace, call_point);
    keymap_t km;
    resmap_t rm;
    // run process for every gathered key
    for (K const& k : ky)
        if (kn.count(k) == 0) {
            internal::trace_key trace_process(node.stack_trace, common::hash_to<trace_t>(k));
            R r;
            status s;
            tie(r, s) = process(k, xs...);
            // if output status, add result to returned map
            if ((char)s >= 4) {
                rm.emplace(k, std::move(r));
                s = s == status::output ? status::internal : static_cast<status>((char)s & char(3));
            }
            // if internal or terminated, propagate key status to neighbours
            if (s == status::terminated or s == status::internal)
                km.emplace(k, s);
        } else km.emplace(k, status::terminated);
    ctx.insert(km);
    return rm;
}

//! @brief The exports type used by the spawn construct with key type `K` and status type `B`.
template <typename K, typename B>
using spawn_t = common::export_list<std::conditional_t<std::is_same<B, field<bool>>::value, common::export_list<std::unordered_set<K, common::hash<K>>, field<bool>>, std::conditional_t<std::is_same<B, bool>::value, std::unordered_set<K, common::hash<K>>, std::unordered_map<K, B, common::hash<K>>>>>;


//! @brief Handles a process, spawning instances of it for every key in the `key_set` and passing general arguments `xs` (legacy version with full status).
template <typename node_t, typename G, typename S, typename... Ts, typename K = typename std::decay_t<S>::value_type, typename T = std::decay_t<std::result_of_t<G(K const&, Ts const&...)>>, typename R = std::decay_t<tuple_element_t<0,T>>, typename B = std::decay_t<tuple_element_t<1,T>>>
std::enable_if_t<std::is_same<B,status>::value, std::unordered_map<K, R, common::hash<K>>>
spawn_deprecated(node_t& node, trace_t call_point, G&& process, S&& key_set, Ts const&... xs) {
    using keymap_t = std::unordered_map<K, B, common::hash<K>>;
    using resmap_t = std::unordered_map<K, R, common::hash<K>>;
    auto ctx = node.template nbr_context<keymap_t>(call_point);
    field<keymap_t> fk = ctx.nbr({});
    // keys to be propagated and terminated
    std::unordered_set<K, common::hash<K>> ky(key_set.begin(), key_set.end()), kn;
    for (size_t i = 1; i < fcpp::details::get_vals(fk).size(); ++i)
        for (auto const& k : fcpp::details::get_vals(fk)[i]) {
            if (k.second == status::terminated)
                kn.insert(k.first);
            else
                ky.insert(k.first);
        }
    internal::trace_call trace_caller(node.stack_trace, call_point);
    keymap_t km;
    resmap_t rm;
    // run process for every gathered key
    for (K const& k : ky)
        if (kn.count(k) == 0) {
            internal::trace_key trace_process(node.stack_trace, common::hash_to<trace_t>(k));
            auto uc = node.undo_context();
            R r;
            status s;
            tie(r, s) = process(k, xs...);
            // if output status, add result to returned map
            if ((char)s >= 4) {
                rm.emplace(k, std::move(r));
                s = s == status::output ? status::internal : static_cast<status>((char)s & char(3));
            }
            // if node in process, merge exports
            if ((char)s >= 2) uc.save();
            // if internal or terminated, propagate key status to neighbours
            if (s == status::terminated or s == status::internal)
                km.emplace(k, s);
        } else km.emplace(k, status::terminated);
    ctx.insert(km);
    return rm;
}

//! @}


} // coordination

} // fcpp

#endif // FCPP_COORDINATION_BASICS_H_
