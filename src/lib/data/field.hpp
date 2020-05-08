// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file field.hpp
 * @brief Implementation and helper functions for the `field<T>` class template for neighboring fields.
 */

#ifndef FCPP_DATA_FIELD_H_
#define FCPP_DATA_FIELD_H_

#include <cassert>

#include <algorithm>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/traits.hpp"
#include "lib/data/tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @cond INTERNAL
template<typename T> class field;
//! @endcond


/**
 * @name field traits
 *
 * Type operators convenient for field manipulation.
 */
//! @{
//! @brief Corresponds to T only if A is a local type.
template <typename A, typename T = void>
using if_local = std::enable_if_t<not common::has_template<field, A>, T>;
//! @brief Corresponds to T only if A is a field type.
template <typename A, typename T = void>
using if_field = std::enable_if_t<common::has_template<field, A>, T>;
//! @brief Type returned upon field collapsing.
template <typename A>
using to_local = common::extract_template<field, A>;
//! @brief Type returned upon field construction.
template <typename A>
using to_field = common::add_template<field, A>;
//! @brief Computes the result type of applying F pointwise to local versions of A.
template <typename F, typename... A>
using local_result = std::result_of_t<F(to_local<const A&>...)>;
//! @brief Computes the result type of applying F pointwise to local versions of A.
template <typename F, typename... A>
using field_result = field<local_result<F, A...>>;
//! @}


//! @cond INTERNAL
//! @brief Forward declarations for enabling friendships.
namespace details {
    template <bool b, typename T>
    struct field_base {};
    
    template <typename A>
    field<A> make_field(A, std::unordered_map<device_t, A>);

    template <typename A>
    std::unordered_map<device_t, A>& get_data(field<A>&);
    template <typename A>
    std::unordered_map<device_t, A>&& get_data(field<A>&&);
    template <typename A>
    std::unordered_map<device_t, A> const& get_data(field<A> const&);

    template <typename A>
    A& get_def(field<A>&);
    template <typename A>
    A&& get_def(field<A>&&);
    template <typename A>
    A const& get_def(field<A> const&);

    template <typename A, typename = if_local<A>>
    A const&& other(A&&);
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> other(A&&);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> other(A&&);

    template <typename A, typename = if_local<A>>
    A const&& self(A&&, device_t);
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> self(A&&, device_t);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> self(A&&, device_t);

    template <typename A, typename = if_local<A>>
    A&& align(A&&, const std::unordered_set<device_t>&);
    template <typename A>
    field<A>& align(field<A>&, const std::unordered_set<device_t>&);
    template <typename A>
    field<A>&& align(field<A>&&, const std::unordered_set<device_t>&);
    template <typename A>
    field<A> align(const field<A>&, const std::unordered_set<device_t>&);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    auto align(A&&, const std::unordered_set<device_t>&);
    
    std::unordered_set<device_t> joined_domain();
    template <typename A, typename... B, typename = if_local<A>>
    std::unordered_set<device_t> joined_domain(const A&, const B&...);
    template <typename A, typename... B>
    std::unordered_set<device_t> joined_domain(const field<A>&, const B&...);
    template <typename... A, typename... B, typename = if_field<tuple<A...>>>
    std::unordered_set<device_t> joined_domain(const tuple<A...>&, const B&...);
}
//! @endcond


/**
 * @brief Class representing a neighboring field of T values.
 */
template <typename T>
class field : public details::field_base<std::is_convertible<T, bool>::value, T> {
    static_assert(not common::has_template<field, T>, "cannot instantiate a field of fields");
    
    //! @cond INTERNAL
    //! @brief Class friendships
    //! @{
    template <typename A>
    friend class field;
    template <bool b, typename A>
    friend class details::field_base;
    //! @}
    //! @endcond
  
  public:
    //! @brief The type of the content.
    typedef T value_type;
  
    //! @name constructors
    //! @{
    /**
     * @brief Default constructor (dangerous).
     *
     * Creates a field constantly equal to the default value for T.
     * Use only if you know the field will be overwritten, or if you
     * are <b>really</b> sure that the default value for T is a
     * reasonable value for your field.
     */
    field() = default;
    
    //! @brief Constant field.
    field(const T& d) : m_data(), m_def(d) {}
    
    //! @brief Copy constructor.
    field(const field&) = default;
    
    //! @brief Move constructor.
    field(field&&) = default;
    
    //! @brief Implicit conversion copy constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field(const field<A>& f) : m_data{f.m_data.begin(), f.m_data.end()}, m_def{static_cast<T>(f.m_def)} {}
    
    //! @brief Implicit conversion move constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field(field<A>&& f) : m_data{make_move_iterator(f.m_data.begin()), make_move_iterator(f.m_data.end())}, m_def(static_cast<T>(std::move(f.m_def))) {}
    //! @}
    
    //! @name assignment operators
    //! @{
    //! @brief Copy assignment.
    field& operator=(const field&) = default;
    
    //! @brief Move assignment.
    field& operator=(field&&) = default;

    //! @brief Implicit conversion copy assignment.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field& operator=(const field<A>& f) {
        m_data.clear();
        m_data.insert(f.m_data.begin(), f.m_data.end());
        m_def = static_cast<T>(f.m_def);
        return *this;
    }

    //! @brief Implicit conversion move assignment.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field& operator=(field<A>&& f) {
        m_data.clear();
        m_data.insert(make_move_iterator(f.m_data.begin()), make_move_iterator(f.m_data.end()));
        m_def = static_cast<T>(std::move(f.m_def));
        return *this;
    }
    //! @}
    
    //! @brief Exchanges the content of the `field` objects.
    void swap(field& f) {
        swap(m_data, f.m_data);
        swap(m_def, f.m_def);
    }

    //! @cond INTERNAL
    //! @brief Function friendships
    //! @{
    template <typename A>
    friend field<A> details::make_field(A, std::unordered_map<device_t, A>);
    
    template <typename A>
    friend std::unordered_map<device_t, A>& details::get_data(field<A>&);
    template <typename A>
    friend std::unordered_map<device_t, A>&& details::get_data(field<A>&&);
    template <typename A>
    friend std::unordered_map<device_t, A> const& details::get_data(field<A> const&);

    template <typename A>
    friend A& details::get_def(field<A>&);
    template <typename A>
    friend A&& details::get_def(field<A>&&);
    template <typename A>
    friend A const& details::get_def(field<A> const&);
    //! @}
    //! @endcond

  private:
    //! @brief Exceptions, as associations device id -> value.
    std::unordered_map<device_t, T> m_data;
    
    //! @brief Default value, retained almost everywhere in the field.
    T m_def;
    
    //! @brief Member constructor, for internal use only.
    field(T def, std::unordered_map<device_t, T> data) : m_data(data), m_def(def) {}
};


//! @cond INTERNAL
namespace details {
    //! @brief Additional cast to bool for field<bool>
    template <typename T>
    struct field_base<true, T> {
        operator bool() const {
            const field<T>& f = *((const field<T>*)this);
            if (not ((bool)f.m_def)) return false;
            for (const auto& x : f.m_data) if (not ((bool)x.second)) return false;
            return true;
        }
    };

    //! @brief Builds a field for testing purposes
    template <typename A>
    field<A> make_field(A def, std::unordered_map<device_t, A> data) {
        field<A> r(def, data);
        return r;
    }

    //! @brief Accesses the private field `m_data` of a field.
    //! @{
    template <typename A>
    std::unordered_map<device_t, A>& get_data(field<A>& f) {
        return f.m_data;
    }
    template <typename A>
    std::unordered_map<device_t, A>&& get_data(field<A>&& f) {
        return std::move(f.m_data);
    }
    template <typename A>
    std::unordered_map<device_t, A> const& get_data(field<A> const& f) {
        return f.m_data;
    }
    //! @}

    //! @brief Accesses the private field `m_def` of a field.
    //! @{
    template <typename A>
    A& get_def(field<A>& f) {
        return f.m_def;
    }
    template <typename A>
    A&& get_def(field<A>&& f) {
        return std::move(f.m_def);
    }
    template <typename A>
    A const& get_def(field<A> const& f) {
        return f.m_def;
    }
    //! @}

    /**
     * @name other
     *
     * Accessor method selecting the default value of a given field.
     */
    //! @{
    //! @brief Const access for non-field values (treated as constant fields).
    template <typename A, typename = if_local<A>>
    A const&& other(A&& x) {
        return static_cast<A const&&>(x);
    }

    //! @brief Full access on fields. WARNING: may lead to unexpected results if the argument is not aligned.
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> other(A&& x) {
        return static_cast<to_local<A&&>>(get_def(std::forward<A>(x)));
    }

    //! @brief Full access on indexed structures.
    template <typename A, size_t... is>
    to_local<A&&> other(A&& x, std::index_sequence<is...>) {
        return {other(get<is>(std::forward<A>(x)))...};
    }

    //! @brief Full access on tuples.
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> other(A&& x) {
        return other(std::forward<A>(x), std::make_index_sequence<common::template_args<A>::size>{});
    }
    //! @}

    /**
     * @name self
     *
     * Accesses the value from a field corresponing to a certain device.
     */
    //! @{
    //! @brief Const access for non-field values (treated as constant fields).
    template <typename A, typename = if_local<A>>
    A const&& self(A&& x, device_t) {
        return static_cast<A const&&>(x);
    }

    template <typename A>
    A& maybe_emplace(std::unordered_map<device_t, A>& m, device_t i, A& x) {
        m.emplace(i, x);
        return m.at(i);
    }
    template <typename A>
    const A& maybe_emplace(const std::unordered_map<device_t, A>&, device_t, const A& x) {
        return x;
    }
    //! @brief Full access on fields.
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> self(A&& x, device_t i) {
        if (get_data(x).count(i)) return static_cast<to_local<A&&>>(get_data(x).at(i));
        return static_cast<to_local<A&&>>(maybe_emplace(get_data(x), i, get_def(x)));
    }

    //! @brief Full access on indexed structures.
    template <typename A, size_t... is>
    to_local<A&&> self(A&& x, device_t i, std::index_sequence<is...>) {
        return {self(get<is>(std::forward<A>(x)), i)...};
    }

    //! @brief Full access on tuples.
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> self(A&& x, device_t i) {
        return self(std::forward<A>(x), i, std::make_index_sequence<common::template_args<A>::size>{});
    }
    //! @}

    /**
     * @name align
     *
     * Computes the restriction of a field to a given domain.
     * The resulting field has exactly the given domain.
     */
    //! @{
    //! @brief align of locals.
    template <typename A, typename = if_local<A>>
    A&& align(A&& x, const std::unordered_set<device_t>&) {
        return std::forward<A>(x);
    }

    //! @brief align of fields.
    template <typename A>
    field<A>& align(field<A>& x, const std::unordered_set<device_t>& s) {
        for (auto it = get_data(x).begin(); it != get_data(x).end(); ) {
            if (s.count(it->first)) ++it;
            else it = get_data(x).erase(it);
        }
        for (device_t i : s) if (not get_data(x).count(i)) get_data(x).emplace(i, get_def(x));
        return x;
    }
    template <typename A>
    field<A>&& align(field<A>&& x, const std::unordered_set<device_t>& s) {
        return std::move(align(x, s));
    }
    template <typename A>
    field<A> align(const field<A>& x, const std::unordered_set<device_t>& s) {
        return align(field<A>{x}, s);
    }
    
    //! @brief align of tuples.
    template <typename A, size_t... is, typename = common::if_class_template<tuple, A>>
    auto align(A&& x, const std::unordered_set<device_t>& s, std::index_sequence<is...>) {
        return capture_as_tuple(align(get<is>(std::forward<A>(x)), s)...);
    }
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    auto align(A&& x, const std::unordered_set<device_t>& s) {
        return align(std::forward<A>(x), s, std::make_index_sequence<common::template_args<A>::size>{});
    }
    //! @}

    //! @brief Joins the domain of a sequence of fields.
    //! @{
    std::unordered_set<device_t> joined_domain() {
        return {};
    }
    template <typename A, typename... B, typename = if_local<A>>
    std::unordered_set<device_t> joined_domain(const A&, const B&... data) {
        return joined_domain(data...);
    }
    template <typename A, typename... B>
    std::unordered_set<device_t> joined_domain(const field<A>& f, const B&... data) {
        std::unordered_set<device_t> domain = joined_domain(data...);
        for (const auto& t : get_data(f)) domain.insert(t.first);
        return domain;
    }
    template <size_t... is, typename... A, typename... B>
    std::unordered_set<device_t> joined_domain(std::index_sequence<is...>, const tuple<A...>& x, const B&... data) {
        return joined_domain(get<is>(x)..., data...);
    }
    template <typename... A, typename... B, typename = if_field<tuple<A...>>>
    std::unordered_set<device_t> joined_domain(const tuple<A...>& x, const B&... data) {
        return joined_domain(std::make_index_sequence<sizeof...(A)>{}, x, data...);
    }
    //! @}

    /**
     * @name map_hood
     *
     * Applies an operator pointwise on a sequence of fields.
     */
    //! @{
    //! @brief General case.
    template <typename F, typename... A>
    field_result<F,A...> map_hood(F&& op, const A&... a) {
        field_result<F,A...> r(op(details::other(a)...));
        for (device_t x : details::joined_domain(a...)) details::self(r,x) = op(details::self(a,x)...);
        return r;
    }
    //! @brief Optimisation for a single field argument.
    template <typename F, typename A>
    field_result<F,field<A>> map_hood(F&& op, const field<A>& a) {
        field_result<F,A> r(op(details::other(a)));
        for (const auto& x : get_data(a)) get_data(r)[x.first] = op(x.second);
        return r;
    }
    //! @brief Optimisation for a single field argument in a binary operator.
    template <typename F, typename A, typename B, typename = if_local<B>>
    field_result<F,field<A>,B> map_hood(F&& op, const field<A>& a, const B& b) {
        field_result<F,A,B> r(op(details::other(a), b));
        for (const auto& x : get_data(a)) get_data(r)[x.first] = op(x.second, b);
        return r;
    }
    //! @brief Optimisation for a single field argument in a binary operator.
    template <typename F, typename A, typename B, typename = if_local<A>>
    field_result<F,A,field<B>> map_hood(F&& op, const A& a, const field<B>& b) {
        field_result<F,A,B> r(op(a, details::other(b)));
        for (const auto& x : get_data(b)) get_data(r)[x.first] = op(a, x.second);
        return r;
    }
    //! @}

    /**
     * @name mod_hood
     *
     * Modifies a field in-place, by applying an operator pointwise (with a sequence of parameters).
     */
    //! @{
    //! @brief General case.
    template <typename F, typename A, typename... B>
    A& mod_hood(F&& op, A& a, const B&... b) {
        for (device_t x : details::joined_domain(a, b...))
            details::self(a,x) = op(details::self(a,x), details::self(b,x)...);
        details::other(a) = op(details::other(a), details::other(b)...);
        return a;
    }
    //! @brief Optimization for unary operators.
    template <typename F, typename A>
    field<A>& mod_hood(F&& op, field<A>& a) {
        for (auto& x : get_data(a)) x.second = op(x.second);
        get_def(a) = op(get_def(a));
        return a;
    }
    //! @brief Optimization for binary operators with a local argument.
    template <typename F, typename A, typename B, typename = if_local<B>>
    field<A>& mod_hood(F&& op, field<A>& a, const B& b) {
        for (auto& x : get_data(a)) x.second = op(x.second, b);
        get_def(a) = op(get_def(a), b);
        return a;
    }
    //! @}

    /**
     * @name fold_hood
     *
     * Reduces the values in a part of a field (determined by domain) to a single value through a binary operation.
     */
    //! @{
    //! @brief Inclusive folding.
    template <typename F, typename A>
    if_field<A, local_result<F,A,A>>
    fold_hood(F&& op, const A& f, const std::unordered_set<device_t>& dom) {
        auto it = dom.begin();
        local_result<F,A,A> res = details::self(f, *it);
        for (++it; it != dom.end(); ++it)
            res = op(details::self(f, *it), res);
        return res;
    }
    //! @brief Inclusive folding (optimization for locals).
    template <typename F, typename A>
    if_local<A, local_result<F,A,A>>
    fold_hood(F&& op, const A& x, const std::unordered_set<device_t>& dom) {
        int n = dom.size();
        local_result<F,A,A> res = x;
        for (--n; n>0; --n) res = op(x, res);
        return res;
    }
    //! @brief Exclusive folding.
    template <typename F, typename A, typename B>
    if_field<A, local_result<F,A,B>>
    fold_hood(F&& op, const A& f, const B& b, const std::unordered_set<device_t>& dom, device_t i) {
        assert(dom.count(i) == 1);
        local_result<F,A,B> res = details::self(b, i);
        for (auto it = dom.begin(); it != dom.end(); ++it)
            if (*it != i) res = op(details::self(f, *it), res);
        return res;
    }
    //! @brief Exclusive folding (optimization for locals).
    template <typename F, typename A, typename B>
    if_local<A, local_result<F,A,B>>
    fold_hood(F&& op, const A& x, const B& b, const std::unordered_set<device_t>& dom, device_t i) {
        assert(dom.count(i) == 1);
        local_result<F,A,B> res = details::self(b, i);
        for (int n = dom.size(); n>1; --n) res = op(x, res);
        return res;
    }
    //! @}
}
//! @endcond


//! @brief Prints a field in dictionary-like format.
template <typename A>
std::ostream& operator<<(std::ostream& o, const field<A>& x) {
    o << "{";
    for (const auto& i : details::get_data(x)) {
        o << i.first << ":" << i.second << ", ";
    }
    return o << "*:" << details::get_def(x) << "}";
}


/**
 * @name mux
 *
 * Multiplexer operator, choosing between its arguments based on the value of the first
 * (always evaluating both arguments).
 */
//! @{
//! @brief local guard
template <typename A>
const A& mux(bool b, const A& x, const A& y) {
    return b ? x : y;
}
//! @brief local guard, moving arguments
template <typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
A mux(bool b, A&& x, A&& y) {
    return b ? std::move(x) : std::move(y);
}
//! @brief field guard
template <typename A>
to_field<A> mux(field<bool> b, const A& x, const A& y) {
    return map_hood([] (bool b, to_local<const A&> x, to_local<const A&> y) -> common::del_template<field, A> {
        return b ? x : y;
    }, b, x, y);
}
//! @brief field guard, moving arguments
template <typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
to_field<A> mux(field<bool> b, A&& x, A&& y) {
    return map_hood([] (bool b, to_local<const A&> x, to_local<const A&> y) -> common::del_template<field, A> {
        return b ? std::move(x) : std::move(y);
    }, b, x, y);
}
//! @}


/**
 * @name max
 *
 * Maximum between two values.
 */
//! @{
//! @brief max between locals.
template <typename A, typename = if_local<A>>
A&& max(A&& x, A&& y) {
    return std::max(std::forward<A>(x), std::forward<A>(y));
}

//! @brief max between fields.
template <typename A, typename = if_field<A>>
to_field<A> max(const A& x, const A& y) {
    return map_hood([] (to_local<const A&> x, to_local<const A&> y) -> common::del_template<field, A> {
        return std::max(x, y);
    }, x, y);
}
//! @}


/**
 * @name min
 *
 * Minimum between two values.
 */
//! @{
//! @brief min between locals.
template <typename A, typename = if_local<A>>
A&& min(A&& x, A&& y) {
    return std::min(std::forward<A>(x), std::forward<A>(y));
}

//! @brief min between fields.
template <typename A, typename = if_field<A>>
to_field<A> min(const A& x, const A& y) {
    return map_hood([] (to_local<const A&> x, to_local<const A&> y) -> common::del_template<field, A> {
        return std::min(x, y);
    }, x, y);
}
//! @}


//! @brief Extracts a component from a field of tuple-like structures.
template <size_t n, typename A>
auto get(const field<A>& f) {
    return map_hood([] (const A& x) {
        return get<n>(x);
    }, f);
}


//! @cond INTERNAL
#define _BOP_TYPE(A,op,B)                                                           \
field<decltype(std::declval<to_local<A>>() op std::declval<to_local<B>>())>
//! @endcond

/**
 * @brief Overloads unary operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_UOP(op)                                                                \
template <typename A>                                                               \
field<A> operator op(const field<A>& x) {                                           \
    return details::map_hood([] (const A& a) {return op a;}, x);                    \
}                                                                                   \
template <typename A>                                                               \
field<A> operator op(field<A>&& x) {                                                \
    details::mod_hood([] (const A& a) {return op std::move(a);}, x);                \
    return x;                                                                       \
}

/**
 * @brief Overloads binary operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_BOP(op)                                                                                    \
template <typename A, typename B>                                                                       \
_BOP_TYPE(field<A>,op,B) operator op(const field<A>& x, const B& y) {                                   \
    return details::map_hood([](const A& a, const to_local<B>& b) { return a op b; }, x, y);            \
}                                                                                                       \
template <typename A, typename B>                                                                       \
_BOP_TYPE(field<A>,op,B) operator op(field<A>&& x, const B& y) {                                        \
    return details::mod_hood([](const A& a, const to_local<B>& b) { return std::move(a) op b; }, x, y); \
}                                                                                                       \
template <typename A, typename B>                                                                       \
common::ifn_class_template<field, A, _BOP_TYPE(A,op,field<B>)>                                          \
operator op(const A& x, const field<B>& y) {                                                            \
    return details::map_hood([](const to_local<A>& a, const B& b) { return a op b; }, x, y);            \
}                                                                                                       \

/**
 * @brief Overloads composite assignment operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_IOP(op)                                                                                    \
template <typename A, typename B>                                                                       \
field<A>& operator op##=(field<A>& x, const B& y) {                                                     \
    return details::mod_hood([](const A& a, const to_local<B>& b) { return std::move(a) op b; }, x, y); \
}


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
_DEF_BOP(<<)
_DEF_BOP(>>)

_DEF_IOP(+)
_DEF_IOP(-)
_DEF_IOP(*)
_DEF_IOP(/)
_DEF_IOP(%)
_DEF_IOP(^)
_DEF_IOP(&)
_DEF_IOP(|)
_DEF_IOP(>>)
_DEF_IOP(<<)

#undef _BOP_TYPE
#undef _DEF_UOP
#undef _DEF_BOP
#undef _DEF_IOP


}

#endif // FCPP_DATA_FIELD_H_
