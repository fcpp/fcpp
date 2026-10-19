// Copyright © 2025 Giorgio Audrito. All Rights Reserved.

/**
 * @file placed.hpp
 * @brief Implementation and helper functions for the `placed<tier,T,p,q>` class template for placed neighboring fields.
 */

#ifndef FCPP_DATA_PLACED_H_
#define FCPP_DATA_PLACED_H_

#include "lib/common/option.hpp"
#include "lib/data/field.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Compile-time function checking whether a bitmask is a subset of another bitmask.
template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr bool bitsubset(T x, T y) {
    return not bool(x & ~y);
}

//! @brief Compile-time bitwise intersection of multiple tier bitmasks (base case).
template <tier_t... xs>
constexpr tier_t tier_inf = (tier_t)-1;

//! @brief Compile-time bitwise intersection of multiple tier bitmasks (general case).
template <tier_t x, tier_t... xs>
constexpr tier_t tier_inf<x, xs...> = x & tier_inf<xs...>;

//! @brief Compile-time bitwise union of multiple tier bitmasks (base case).
template <tier_t... xs>
constexpr tier_t tier_sup = (tier_t)0;

//! @brief Compile-time bitwise union of multiple tier bitmasks (general case).
template <tier_t x, tier_t... xs>
constexpr tier_t tier_sup<x, xs...> = x | tier_sup<xs...>;


//! @cond INTERNAL
//! @brief Forward declaration
template <tier_t tier, typename T, tier_t p = tier_t(-1), tier_t q = 0> class placed;
//! @endcond


/**
 * @name extract_tier
 *
 * Constant which extracts the tier information from a sequence of types,
 * provided that at least one of them is placed or a placed tuple.
 * If no argument is placed, it returns zero (invalid empty tier).
 */
//! @{
//! @brief Invalid case.
template <class... Ts>
constexpr tier_t extract_tier = 0;

//! @brief Ignore constness.
template <class A, class... Ts>
constexpr tier_t extract_tier<A const, Ts...> = extract_tier<A, Ts...>;

//! @brief Ignore lvalue references.
template <class A, class... Ts>
constexpr tier_t extract_tier<A&, Ts...> = extract_tier<A, Ts...>;

//! @brief Ignore rvalue references.
template <class A, class... Ts>
constexpr tier_t extract_tier<A&&, Ts...> = extract_tier<A, Ts...>;

//! @brief Ignoring non-placed types.
template <class A, class... Ts>
constexpr tier_t extract_tier<A, Ts...> = extract_tier<Ts...>;

//! @brief Extract from placed type.
template <tier_t tier, class A, tier_t p, tier_t q, class... Ts>
constexpr tier_t extract_tier<placed<tier,A,p,q>, Ts...> = tier;

//! @brief Expand tuple-like type.
template <template<class...> class U, class... As, class... Ts>
constexpr tier_t extract_tier<U<As...>, Ts...> = extract_tier<As..., Ts...>;

//! @brief Expand array-like type.
template <template<class,size_t> class U, class A, size_t N, class... Ts>
constexpr tier_t extract_tier<U<A, N>, Ts...> = extract_tier<A, Ts...>;
//! @}


/**
 * Constant which is true if and only if at least one of the type parameters are built through array-like
 * and tuple-like classes from specializations of the placed template.
 */
template <class... Ts>
constexpr bool is_placed = extract_tier<Ts...> != 0;


//! @cond INTERNAL
namespace details {
    //! @brief General form.
    template <tier_t tier, typename A, bool b = is_placed<A> or common::has_template<field, A>>
    struct to_placed;

    //! @brief Base case assuming no occurrences of the template.
    template <tier_t tier, class A>
    struct to_placed<tier, A, false> {
        using value_type = common::partial_decay<A>;
        static constexpr tier_t p_value = tier_t(-1);
        static constexpr tier_t q_value = 0;
        using type = placed<tier, std::decay_t<A>>;
    };

    //! @brief Mixing multiple placed types.
    template <tier_t tier, template<class...> class U, class... A>
    struct mix_placed {
        using value_type = U<typename A::value_type...>;
        static constexpr tier_t p_value = tier_inf<A::p_value...>;
        static constexpr tier_t q_value = tier_sup<A::q_value...>;
        using type = placed<tier, value_type, p_value, q_value>;
    };

    //! @brief Extracts occurrences from the arguments of a tuple-like type.
    template <tier_t tier, template<class...> class U, class... A>
    struct to_placed<tier, U<A...>, true> : public mix_placed<tier, U, to_placed<tier, common::partial_decay<A>>...> {};

    //! @brief Extracts occurrences from the arguments of a tuple&-like type.
    template <tier_t tier, template<class...> class U, class... A>
    struct to_placed<tier, U<A...>&, true> : public to_placed<tier, U<A&...>, true> {};

    //! @brief Extracts occurrences from the arguments of a tuple const&-like type.
    template <tier_t tier, template<class...> class U, class... A>
    struct to_placed<tier, U<A...> const&, true> : public to_placed<tier, U<A const&...>, true> {};

    //! @brief Extracts occurrences from the argument of an array-like type.
    template <tier_t tier, template<class,size_t> class U, class A, size_t N>
    struct to_placed<tier, U<A, N>, true> {
        using subtype = to_placed<tier, common::partial_decay<A>, true>;
        using value_type = U<std::decay_t<typename subtype::value_type>, N>;
        static constexpr tier_t p_value = subtype::p_value;
        static constexpr tier_t q_value = subtype::q_value;
        using type = placed<tier, value_type, p_value, q_value>;
    };

    //! @brief Propagate & assuming occurrences of the template are present.
    template <tier_t tier, template<class,size_t> class U, class A, size_t N>
    struct to_placed<tier, U<A, N>&, true> : public to_placed<tier, U<std::decay_t<A>, N>, true> {};

    //! @brief Propagate const& assuming occurrences of the template are present.
    template <tier_t tier, template<class,size_t> class U, class A, size_t N>
    struct to_placed<tier, U<A, N> const&, true> : public to_placed<tier, U<std::decay_t<A>, N>, true> {};

    //! @brief For a single placed<...> argument.
    template <tier_t tier, tier_t t, typename T, tier_t p, tier_t q>
    struct single_placed {
        static_assert(tier == t, "mixing up different tiers: cannot appy to_placed<tier, T> to a T with a different tier");
        using value_type = typename common::details::vectorize<common::partial_decay<T>>::type;
        static constexpr tier_t p_value = p;
        static constexpr tier_t q_value = q;
        using type = placed<tier, std::decay_t<T>, p, q>;
    };

    //! @brief If the second parameter is of the form placed<...>.
    template <tier_t tier, tier_t t, typename T, tier_t p, tier_t q>
    struct to_placed<tier, placed<t,T,p,q>, true> : public single_placed<tier, t, T, p, q> {};

    //! @brief If the second parameter is of the form placed<...>&.
    template <tier_t tier, tier_t t, typename T, tier_t p, tier_t q>
    struct to_placed<tier, placed<t,T,p,q>&, true> : public single_placed<tier, t, T&, p, q> {};

    //! @brief If the second parameter is of the form placed<...> const&.
    template <tier_t tier, tier_t t, typename T, tier_t p, tier_t q>
    struct to_placed<tier, placed<t,T,p,q> const&, true> : public single_placed<tier, t, T const&, p, q> {};

    //! @brief If the second parameter is of the form field<...>.
    template <tier_t tier, typename T>
    struct to_placed<tier, field<T>, true> : public single_placed<tier, tier, T, tier_t(-1), tier_t(-1)> {};

    //! @brief If the second parameter is of the form field<...>&.
    template <tier_t tier, typename T>
    struct to_placed<tier, field<T>&, true> : public single_placed<tier, tier, T&, tier_t(-1), tier_t(-1)> {};

    //! @brief If the second parameter is of the form field<...> const&.
    template <tier_t tier, typename T>
    struct to_placed<tier, field<T> const&, true> : public single_placed<tier, tier, T const&, tier_t(-1), tier_t(-1)> {};


    //! @brief Base case assuming no occurrences of the template.
    template <typename T>
    struct decay_placed {
        using type = T;
    };

    //! @brief Decay rvalue references.
    template <typename T>
    struct decay_placed<T&&> {
        using type = typename decay_placed<T>::type&&;
    };

    //! @brief Decay lvalue references.
    template <typename T>
    struct decay_placed<T&> {
        using type = typename decay_placed<T>::type&;
    };

    //! @brief Decay const references.
    template <typename T>
    struct decay_placed<T const&> {
        using type = typename decay_placed<T>::type const&;
    };

    //! @brief Decay placed types.
    template <tier_t tier, typename T, tier_t p, tier_t q>
    struct decay_placed<placed<tier,T,p,q>> {
        using type = typename placed<tier,T,p,q>::field_type;
    };

    //! @brief Decay tuple-like types.
    template <template<class...> class U, class... As>
    struct decay_placed<U<As...>> {
        using type = U<typename decay_placed<As>::type...>;
    };

    //! @brief Decay array-like types.
    template <template<class,size_t> class U, class A, size_t N>
    struct decay_placed<U<A, N>> {
        using type = U<typename decay_placed<A>::type, N>;
    };
}
//! @endcond INTERNAL

//! @brief Converts a type to a placed field.
template <tier_t tier, typename A>
using to_placed = typename details::to_placed<tier, common::partial_decay<A>>::type;


//! @brief Extract the non-placed type from a placed type.
template <tier_t tier, typename A>
using del_placed = typename details::to_placed<tier, A>::value_type;


//! @brief Removes placement descriptors if present.
template <typename A>
using decay_placed = typename details::decay_placed<A>::type;


//! @cond INTERNAL
//! @brief Forward declarations
//! @{
namespace details {
    template <tier_t tier, typename T, tier_t p, tier_t q, typename F>
    placed<tier,T,p,q> place_data(common::type_sequence<placed<tier,T,p,q>>, F&&);
    template <typename T>
    fcpp::decay_placed<T&&> maybe_get_data(T&&);
}
//! @}
//! @endcond


/**
 * @brief Class representing a placed neighboring field of T values.
 */
template <tier_t tier, typename T, tier_t p, tier_t q>
class placed {
    // TODO: base class for conversion to bool & conversion from field if compatible with parameters
    static_assert(tier != 0 and (tier & (tier-1)) == 0, "tier must be atomic (with a single bit set)");
    static_assert(not common::has_template<field, T>, "cannot instantiate a placed field of fields");
    static_assert(not is_placed<T>, "cannot instantiate a placed field of placed fields");

    //! @brief Checks whether the current type is compatible with another placed type.
    template <typename A, tier_t p1, tier_t q1>
    static constexpr bool is_compatible = std::is_convertible<A,T>::value and bitsubset(p, p1) and bitsubset(q1, q);

    //! @cond INTERNAL
    //! @brief Class friendships
    //! @{
    template <tier_t t1, typename A, tier_t p1, tier_t q1>
    friend class placed;
    //! @}

    //! @brief Function friendships
    //! @{
    template <tier_t tier1, typename T1, tier_t p1, tier_t q1, typename F>
    friend placed<tier1,T1,p1,q1> details::place_data(common::type_sequence<placed<tier1,T1,p1,q1>>, F&&);
    template <typename T1>
    friend decay_placed<T1&&> details::maybe_get_data(T1&&);
    //! @}
    //! @endcond

  public:
    //! @brief The dual placed type.
    using dual_type = placed<tier, T, q, p>;
    //! @brief The local placed type.
    using local_type = placed<tier, T, p, 0>;
    //! @brief The contained type.
    using value_type = T;
    //! @brief The underlying field type.
    using field_type = std::conditional_t<bool(q), field<T>, T>;
    //! @brief The overal optional type.
    using option_type = common::option<field_type, bool(tier & p)>;
    //! @brief The atomic tier of the device currently running the program.
    static constexpr tier_t tier_value = tier;
    //! @brief The tier class of the devices where the data exists.
    static constexpr tier_t p_value = p;
    //! @brief The tier class of the devices from which neighbouring data comes from.
    static constexpr tier_t q_value = q;

    //! @name constructors
    //! @{

    //! @brief Default constructor (dangerous: if q>0 creates a placed field in an invalid state).
    placed() = default;

    //! @brief Constant placed field (copying).
    placed(T const& d) : m_data(d) {}

    //! @brief Constant placed field (moving).
    placed(T&& d) : m_data(std::move(d)) {}

    //! @brief Copy constructor.
    placed(placed const&) = default;

    //! @brief Move constructor.
    placed(placed&&) = default;

    //! @brief Implicit conversion copy constructor.
    template <typename A, tier_t p1, tier_t q1, typename = std::enable_if_t<is_compatible<A,p1,q1>>>
    placed(placed<tier, A, p1, q1> const& f) : m_data(f.m_data) {}

    //! @brief Implicit conversion move constructor.
    template <typename A, tier_t p1, tier_t q1, typename = std::enable_if_t<is_compatible<A,p1,q1>>>
    placed(placed<tier, A, p1, q1>&& f) : m_data(std::move(f.m_data)) {}
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Copy assignment.
    placed& operator=(placed const&) = default;

    //! @brief Move assignment.
    placed& operator=(placed&&) = default;

    //! @brief Implicit conversion copy assignment.
    template <typename A, tier_t p1, tier_t q1, typename = std::enable_if_t<is_compatible<A,p1,q1>>>
    placed& operator=(placed<tier, A, p1, q1> const& f) {
        m_data = f.m_data;
        return *this;
    }

    //! @brief Implicit conversion move assignment.
    template <typename A, tier_t p1, tier_t q1, typename = std::enable_if_t<is_compatible<A,p1,q1>>>
    placed& operator=(placed<tier, A, p1, q1>&& f) {
        m_data = std::move(f.m_data);
        return *this;
    }
    //! @}

    //! @brief Write access to the underlying optional field.
    option_type& get() {
        return m_data;
    }

    //! @brief Const access to the underlying optional field.
    option_type const& get() const {
        return m_data;
    }

    //! @brief Const access to the underlying field, with a default if no value available.
    field_type const& get_or(field_type const& f) const {
        return m_data.get_or(f);
    }

    //! @brief Exchanges the content of the `placed` objects.
    void swap(placed& f) {
        swap(m_data, f.m_data);
    }

    //! @brief Serialises the content from a given input stream.
    common::isstream& serialize(common::isstream& s) {
        return s & m_data;
    }

    //! @brief Serialises the content to a given output stream.
    template <typename S>
    S& serialize(S& s) const {
        return s & m_data;
    }

    //! @brief Prints the content of the placed field to a given output stream.
    template <typename O>
    void print(O& o) const {
        if (bool(tier & p)) {
            o << m_data.front();
        } else {
            o << common::type_name<T>();
        }
        o << "@" << int(p) << "," << int(q);
    }

  private:
    //! @brief Field constructor (copying).
    placed(field<T> const& f) : m_data(maybe_field(std::integral_constant<bool, (tier&p) and q>{}, f)) {}

    //! @brief Field constructor (moving).
    placed(field<T>&& f) : m_data(maybe_field(std::integral_constant<bool, (tier&p) and q>{}, std::move(f))) {}

    //! @brief Field constructor argument forwarding (active).
    template <typename F>
    F&& maybe_field(std::true_type, F&& f) {
        return std::forward<F>(f);
    }

    //! @brief Field constructor argument forwarding (inactive).
    template <typename F>
    option_type maybe_field(std::false_type, F&&) {
        return {};
    }

    //! @brief Ordered IDs of exceptions.
    option_type m_data;
};


//! @cond INTERNAL
namespace details {
    //! @brief Accesses private constructors for placed.
    template <tier_t tier, typename T, tier_t p, tier_t q, typename F>
    placed<tier,T,p,q> place_data(common::type_sequence<placed<tier,T,p,q>>, F&& f) {
        return std::forward<F>(f);
    }

    //! @brief Builds a placed field from member values.
    template <tier_t tier, typename T, tier_t p, tier_t q>
    inline placed<tier,T,p,q> make_placed(std::vector<device_t>&& ids, std::vector<T>&& vals) {
        return place_data(common::type_sequence<placed<tier,T,p,q>>{}, make_field(std::move(ids), std::move(vals)));
    }

    //! @brief Accesses inner data of a possibly placed field.
    template <typename T>
    fcpp::decay_placed<T&&> maybe_get_data(T&& x) {
        return reinterpret_cast<fcpp::decay_placed<T&&>>(std::forward<T>(x));
    }

    //! @brief Applies a void function on a sequence of placed fields (empty overload).
    template <typename... Ts>
    inline void maybe_do(std::false_type, Ts&&...) {}
    //! @brief Applies a void function on a sequence of placed fields (active overload).
    template <typename F, typename... Ts>
    inline void maybe_do(std::true_type, F&& op, Ts&&... xs) {
        std::forward<F>(op)(maybe_get_data(std::forward<Ts>(xs))...);
    }
    //! @brief Applies a function on a sequence of placed fields (active overload).
    template <tier_t tier, typename T, tier_t p, tier_t q, typename... Ts>
    inline void maybe_do(common::type_sequence<placed<tier,T,p,q>>, Ts&&... xs) {
        maybe_do(std::integral_constant<bool, bool(tier&p)>{}, std::forward<Ts>(xs)...);
    }

    //! @brief Applies a function on a sequence of placed fields (empty overload).
    template <typename T, typename... Ts>
    inline T maybe_perform(std::false_type, common::type_sequence<T>, Ts&&...) {
        return {};
    }
    //! @brief Applies a function on a sequence of placed fields (active overload).
    template <typename T, typename F, typename... Ts>
    inline T maybe_perform(std::true_type, common::type_sequence<T> t, F&& op, Ts&&... xs) {
        return place_data(t, std::forward<F>(op)(maybe_get_data(std::forward<Ts>(xs))...));
    }
    //! @brief Applies a function on a sequence of placed fields (active overload).
    template <tier_t tier, typename T, tier_t p, tier_t q, typename... Ts>
    inline placed<tier,T,p,q> maybe_perform(common::type_sequence<placed<tier,T,p,q>> t, Ts&&... xs) {
        return maybe_perform(std::integral_constant<bool, bool(tier&p)>{}, t, std::forward<Ts>(xs)...);
    }

    //! @brief Accesses the value from a placed field corresponing to a certain device, returning a local placed value.
    template <tier_t tier, typename T>
    inline auto self(std::integer_sequence<tier_t, tier>, T&& x, device_t i) {
        using P = fcpp::to_placed<tier, T>;
        using L = typename P::local_type;
        return maybe_perform(common::type_sequence<L>{}, [i](auto&& y){
            return self(std::forward<decltype(y)>(y), i);
        }, std::forward<T>(x));
    }

    //! @brief Applies an operator pointwise on a sequence of placed fields (not placed overload).
    template <typename P, typename... Ss, typename F, typename... Ts>
    auto pmap_hood(common::number_sequence<0>, P, common::type_sequence<tuple<Ss...>>, F&& op, Ts const&... xs) {
        return map_hood(op, xs...);
    }
    //! @brief Applies an operator pointwise on a sequence of placed fields (placed overload).
    template <intmax_t tier, typename P, typename... Ss, typename F, typename... Ts>
    auto pmap_hood(common::number_sequence<tier>, P, common::type_sequence<tuple<Ss...>>, F&& op, Ts const&... xs) {
        using T = local_result<F, Ss...>;
        return maybe_perform(common::type_sequence<placed<tier,T,P::p_value,P::q_value>>{}, [&op](auto const&... ys){
            return map_hood(op, ys...);
        }, xs...);
    }
}
//! @endcond

//! @brief Applies an operator pointwise on a sequence of placed fields.
template <typename F, typename... Ts>
auto pmap_hood(F&& op, Ts const&... xs) {
    constexpr tier_t tier = extract_tier<Ts...>;
    using P = details::to_placed<tier, tuple<Ts const&...>>;
    return details::pmap_hood(common::number_sequence<tier>{}, P{}, common::type_sequence<typename P::value_type>{}, op, xs...);
}
// TODO: rename to map_hood while avoiding conflicts with that in field.hpp


//! @cond INTERNAL
namespace details {
    //! @brief The result type of a placed fold operation.
    template <tier_t tier, typename F, typename A, typename B, tier_t p>
    using fold_result = placed<tier, common::invoke_result_t<F, A const&, B const&>, p, 0>;

    //! @{
    //! @brief Exclusive folding.
    //! Reduces the values in a part of a placed field (determined by domain) to a single value through a binary operation.
    template <typename F, tier_t tier, typename A, tier_t p, tier_t q, typename B>
    inline fold_result<tier, F, A, B, p>
    fold_hood(F&& op, placed<tier,A,p,q> const& f, B const& b, std::vector<device_t> const& dom, device_t i) {
        return maybe_perform(common::type_sequence<fold_result<tier, F, A, B, p>>{}, [&op, &b, &dom, i](auto const& x){
            return fold_hood(op, x, b, dom, i);
        }, f);
    }

    //! @brief Performs the domain union of multiple placed fields (no arguments).
    template <typename R>
    inline R get_or(common::type_sequence<R>) {
        return {};
    }

    //! @brief Performs the domain union of multiple placed fields (forward declaration).
    template <typename R, tier_t tier, typename T, tier_t p, tier_t q, tier_t... ps, tier_t... qs>
    inline R get_or(common::type_sequence<R>, placed<tier,T,p,q> const&, placed<tier,T,ps,qs> const&...);

    //! @brief Performs the domain union of multiple placed fields (first undefined).
    template <typename R, tier_t tier, typename T, tier_t p, tier_t q, tier_t... ps, tier_t... qs>
    inline R get_or(common::type_sequence<R> r, std::false_type, placed<tier,T,p,q> const&, placed<tier,T,ps,qs> const&... fs) {
        return get_or(r, fs...);
    }

    //! @brief Performs the domain union of multiple placed fields (first defined).
    template <typename R, tier_t tier, typename T, tier_t p, tier_t q, tier_t... ps, tier_t... qs>
    inline R get_or(common::type_sequence<R> r, std::true_type, placed<tier,T,p,q> const& f, placed<tier,T,ps,qs> const&...) {
        return place_data(common::type_sequence<R>{}, maybe_get_data(f));
    }

    //! @brief Performs the domain union of multiple placed fields (some arguments).
    template <typename R, tier_t tier, typename T, tier_t p, tier_t q, tier_t... ps, tier_t... qs>
    inline R get_or(common::type_sequence<R> r, placed<tier,T,p,q> const& f, placed<tier,T,ps,qs> const&... fs) {
        return get_or(r, std::integral_constant<bool, bool(tier&p)>{}, f, fs...);
    }
}
//! @endcond

//! @brief Performs the domain union of multiple placed fields.
template <tier_t tier, typename T, tier_t... ps, tier_t... qs, typename R = placed<tier,T,tier_sup<ps...>,tier_sup<qs...>>>
R get_or(placed<tier,T,ps,qs> const&... fs) {
    return details::get_or(common::type_sequence<R>{}, fs...);
}

/**
 * @brief Overloads unary operators for placed fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_UOP(op)                                                                         \
template <typename A, tier_t tier = extract_tier<A>, typename = std::enable_if_t<tier != 0>> \
auto operator op(A&& x) {                                                                    \
    return pmap_hood([](auto const& a){ return op a; }, std::forward<A>(x));                 \
}

/**
 * @brief Overloads binary operators for placed fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_BOP(op)                                                                                              \
template <typename A, typename B, tier_t tier = extract_tier<A, B>, typename = std::enable_if_t<tier != 0>>       \
auto operator op(A&& x, B&& y) {                                                                                  \
    return pmap_hood([](auto const& a, auto const& b){ return a op b; }, std::forward<A>(x), std::forward<B>(y)); \
}

///**
// * @brief Overloads composite assignment operators for fields.
// *
// * Used to overload every operator available for the base type.
// * Macro not available outside of the scope of this file.
// */
//#define _DEF_IOP(op)                                                                                    \
//template <typename A, typename B>                                                                       \
//field<A>& operator op##=(field<A>& x, B const& y) {                                                     \
//    return pmod_hood([](A const& a, to_local<B> const& b) { return std::move(a) op b; }, x, y);          \
//}


_DEF_UOP(+)
_DEF_UOP(-)
_DEF_UOP(~)
_DEF_UOP(!)

_DEF_BOP(+)
_DEF_BOP(-)
_DEF_BOP(*)
_DEF_BOP(/)
_DEF_BOP(%)
_DEF_BOP(^)
_DEF_BOP(&)
_DEF_BOP(|)
_DEF_BOP(<)
_DEF_BOP(>)
_DEF_BOP(<=)
_DEF_BOP(>=)
_DEF_BOP(==)
_DEF_BOP(!=)
_DEF_BOP(&&)
_DEF_BOP(||)

//_DEF_IOP(+)
//_DEF_IOP(-)
//_DEF_IOP(*)
//_DEF_IOP(/)
//_DEF_IOP(%)
//_DEF_IOP(^)
//_DEF_IOP(&)
//_DEF_IOP(|)
//_DEF_IOP(>>)
//_DEF_IOP(<<)


#undef _DEF_UOP
#undef _DEF_BOP
#undef _DEF_IOP


} // namespace fcpp

#endif // FCPP_DATA_PLACED_H_
