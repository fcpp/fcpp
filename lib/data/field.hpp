// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

/**
 * @file field.hpp
 * @brief Implementation and helper functions for the `field<T>` class template for neighboring fields.
 */

#ifndef FCPP_DATA_FIELD_H_
#define FCPP_DATA_FIELD_H_

#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/traits.hpp"


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
//@{
//! @brief Corresponds to T only if A is a field type.
template <typename A, typename T>
using if_field = std::enable_if_t<common::has_template<field, A>, T>;
//! @brief Corresponds to T only if A is a field type which is not a reference.
template <typename A, typename T>
using if_plain_field = std::enable_if_t<common::has_template<field, A> and not std::is_reference<A>::value, T>;
//! @brief Corresponds to T only if A is a local type.
template <typename A, typename T>
using if_local = std::enable_if_t<not common::has_template<field, A>, T>;
//! @brief Corresponds to T only if A is a local type which is not a reference.
template <typename A, typename T>
using if_plain_local = std::enable_if_t<not common::has_template<field, A> and not std::is_reference<A>::value, T>;
//! @brief Computes the result of applying F to local versions of A.
template <typename F, typename... A>
using local_result = std::result_of_t<F(common::del_template<field, A>...)>;
//@}


//! @cond INTERNAL
namespace details {
    template <class T>
    struct field_base {};
    
    template <typename A>
    field<A> make_field(A, std::unordered_map<device_t, A>);
    
    template <typename A>
    A& other(field<A>&);
    
    template <typename A>
    A& self(field<A>&, device_t);
    
    template <typename A>
    A&& self(field<A>&&, device_t);
    
    template <typename A>
    const A& self(const field<A>&, device_t);
    
    template <typename A>
    if_local<A, const A&> self(const A&, device_t);
    
    template <typename A>
    if_plain_local<A, A&&> self(A&&, device_t);
    
    template <typename A>
    field<A>& align(field<A>&, const std::unordered_set<device_t>&);
    
    template <typename A>
    field<A>&& align(field<A>&&, const std::unordered_set<device_t>&);
    
    template <typename A>
    field<A> align(const field<A>&, const std::unordered_set<device_t>&);
    
    template <typename A>
    if_local<A, A> align(A&&, const std::unordered_set<device_t>&);
    
    std::unordered_set<device_t> joined_domain();
    
    template <typename A, typename... B>
    std::unordered_set<device_t> joined_domain(const field<A>& f, const B&... data);
    
    template <typename A, typename... B>
    if_local<A, std::unordered_set<device_t>> joined_domain(const A& f, const B&... data);

    template <typename F, typename A>
    std::result_of_t<F(A,A)>
    fold_hood(F&&, const field<A>&, const std::unordered_set<device_t>&);
    
    template <typename F, typename A>
    if_local<A, std::result_of_t<F(A,A)>>
    fold_hood(F&&, const A&, const std::unordered_set<device_t>&);
//! @endcond
}


/**
 * @brief Class representing a neighboring field of T values.
 */
template <typename T>
class field : public details::field_base<T> {
    static_assert(not common::has_template<field, T>, "cannot instantiate a field of fields");
    
    //! @cond INTERNAL
    template <typename A>
    friend class field;
    template <typename A>
    friend class details::field_base;
    //! @endcond
  
  public:
    //! @brief The type of the content.
    typedef T value_type;
    
  private:
    //! @brief Exceptions, as associations device id -> value.
    std::unordered_map<device_t, T> m_data;
    
    //! @brief Default value, retained almost everywhere in the field.
    T def;
    
    //! @brief Member constructor, for internal use only.
    field(T _def, std::unordered_map<device_t, T> data) : m_data(data), def(_def) {}
  
  public:
    //! @name constructors
    //@{
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
    field(const T& d) : m_data(), def(d) {}
    
    //! @brief Copy constructor.
    field(const field<T>&) = default;
    
    //! @brief Move constructor.
    field(field<T>&&) = default;
    //@}
    
    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    field<T>& operator=(const field<T>&) = default;
    
    //! @brief Move assignment.
    field<T>& operator=(field<T>&&) = default;
    //@}
    
    //! @brief Casts to fields of compatible base type.
    template <typename A, typename = std::enable_if_t<std::is_convertible<T,A>::value>>
    operator field<A>() const {
        field<A> r(static_cast<A>(def));
        for (const auto& x : m_data)
            r.m_data[x.first] = static_cast<A>(x.second);
        return r;
    }
    
    //! @cond INTERNAL
    template <typename A>
    friend field<A> details::make_field(A, std::unordered_map<device_t, A>);
    //! @endcond
    
    /**
     * @name other
     *
     * Accessor method selecting the default value of a given field.
     */
    //@{
    //! @brief Write access. WARNING: may lead to unexpected results if the argument is not aligned.
    template<typename A>
    friend A& details::other(field<A>&);
    
    //! @brief Move access.
    template<typename A>
    friend A&& other(field<A>&&);
    
    //! @brief Read-only access.
    template<typename A>
    friend const A& other(const field<A>&);
    //@}
    
    //! @cond INTERNAL
    template<typename A>
    friend A& details::self(field<A>&, device_t);
    
    template <typename A>
    friend A&& details::self(field<A>&&, device_t);

    template<typename A>
    friend const A& details::self(const field<A>&, device_t);
    
    template<typename A>
    friend field<A>& details::align(field<A>&, const std::unordered_set<device_t>&);
    
    template<typename A>
    friend field<A>&& details::align(field<A>&&, const std::unordered_set<device_t>&);
    
    template<typename A>
    friend field<A> details::align(const field<A>&, const std::unordered_set<device_t>&);
    
    template <typename A, typename... B>
    friend std::unordered_set<device_t> details::joined_domain(const field<A>&, const B&...);
    //! @endcond
    
    /**
     * @name map_hood
     *
     * Applies an operator pointwise on a sequence of fields.
     */
    //@{
    template <typename F, typename A>
    friend field<local_result<F,A>> map_hood(F&&, const field<A>&);
    template <typename F, typename A, typename B>
    friend if_local<B, field<local_result<F,A,B>>> map_hood(F&&, const field<A>&, const B&);
    template <typename F, typename A, typename B>
    friend if_local<A, field<local_result<F,A,B>>> map_hood(F&&, const A&, const field<B>&);
    //@}
    
    /**
     * @name mod_hood
     *
     * Modifies a field in-place, by applying an operator pointwise (with a sequence of parameters).
     */
    //@{
    template <typename F, typename A>
    friend field<A>& mod_hood(F&&, field<A>&);
    template <typename F, typename A, typename B>
    friend if_local<B, field<A>&> mod_hood(F&&, field<A>&, const B&);
    //@}
    
    //! @brief Prints a field in dictionary-like format.
    template <typename A>
    friend std::ostream& operator<<(std::ostream&, const field<A>&);
};


//! @cond INTERNAL
template <>
struct details::field_base<bool> {
    operator bool() const {
        const field<bool>* f = static_cast<const field<bool>*>(this);
        if (not f->def) return false;
        for (const auto& x : f->m_data) if (not x.second) return false;
        return true;
    }
};

template <typename A>
field<A> details::make_field(A def, std::unordered_map<device_t, A> data) {
    field<A> r(def, data);
    return r;
}
//! @endcond

/**
 * @name other
 *
 * Accessor method selecting the default value of a given field.
 */
//@{
//! @brief Write access. WARNING: may lead to unexpected results if the argument is not aligned.
template <typename A>
A& details::other(field<A>& x) {
    return x.def;
}

//! @brief Move access.
template <typename A>
A&& other(field<A>&& x) {
    return std::move(x.def);
}

//! @brief Read-only access.
template <typename A>
const A& other(const field<A>& x) {
    return x.def;
}

//! @brief Read-only access for non-field values (treated as constant fields).
template <typename A>
if_local<A, const A&> other(const A& x) {
    return x;
}

//! @brief Move access for non-field values (treated as constant fields).
template <typename A>
if_plain_local<A, A&&> other(A&& x) {
    return std::move(x);
}
//@}

/*
 * Accesses the value from a field corresponing to a certain device.
 */
//! @cond INTERNAL
template <typename A>
A& details::self(field<A>& x, device_t i) {
    if (x.m_data.count(i)) return x.m_data.at(i);
    return x.m_data[i] = x.def;
}

template <typename A>
A&& details::self(field<A>&& x, device_t i) {
    if (x.m_data.count(i)) return std::move(x.m_data.at(i));
    return std::move(x.def);
}

template <typename A>
const A& details::self(const field<A>& x, device_t i) {
    if (x.m_data.count(i)) return x.m_data.at(i);
    return x.def;
}

template <typename A>
if_local<A, const A&> details::self(const A& x, device_t) {
    return x;
}

template <typename A>
if_plain_local<A, A&&> details::self(A&& x, device_t) {
    return std::move(x);
}

/*
 * Computes the restriction of a field to a given domain.
 * The resulting field has exactly the given domain.
 */
template <typename A>
field<A>& details::align(field<A>& x, const std::unordered_set<device_t>& s) {
    for (auto it = x.m_data.begin(); it != x.m_data.end(); ) {
        if (s.count(it->first)) ++it;
        else it = x.m_data.erase(it);
    }
    for (device_t i : s) if (not x.m_data.count(i)) x.m_data[i] = x.def;
    return x;
}

template <typename A>
field<A>&& details::align(field<A>&& x, const std::unordered_set<device_t>& s) {
    return std::move(details::align(x, s));
}

template <typename A>
field<A> details::align(const field<A>& x, const std::unordered_set<device_t>& s) {
    return details::align(field<A>(x), s);
}

template <typename A>
if_local<A, A> details::align(A&& x, const std::unordered_set<device_t>&) {
    return std::forward<A>(x);
}
//! @endcond
    
std::unordered_set<device_t> details::joined_domain() {
    return {};
}

template <typename A, typename... B>
std::unordered_set<device_t> details::joined_domain(const field<A>& f, const B&... data) {
    std::unordered_set<device_t> domain = joined_domain(data...);
    for (const auto& t : f.m_data) domain.insert(t.first);
    return domain;
}

template <typename A, typename... B>
if_local<A, std::unordered_set<device_t>> details::joined_domain(const A& f, const B&... data) {
    return joined_domain(data...);
}

/*
 * Reduces the values in a part of a field (determined by domain) to a single value through a binary operation.
 */
//! @cond INTERNAL
template <typename F, typename A>
std::result_of_t<F(A,A)> details::fold_hood(F&& op, const field<A>& f, const std::unordered_set<device_t>& domain) {
    auto it = domain.begin();
    std::result_of_t<F(A,A)> res = details::self(f, *it);
    for (++it; it != domain.end(); ++it)
        res = op(details::self(f, *it), res);
    return res;
}

template <typename F, typename A>
if_local<A, std::result_of_t<F(A,A)>>
details::fold_hood(F&& op, const A& x, const std::unordered_set<device_t>& domain) {
    int n = domain.size();
    std::result_of_t<F(A,A)> res = x;
    for (--n; n>0; --n) res = op(x, res);
    return res;
}
//! @endcond

/**
 * @name map_hood
 *
 * Applies an operator pointwise on a sequence of fields.
 */
//@{
//! @brief General case.
template <typename F, typename... A>
field<local_result<F,A...>>
map_hood(F&& op, const A&... a) {
    field<local_result<F,A...>> r(op(other(a)...));
    for (device_t x : details::joined_domain(a...)) details::self(r,x) = op(details::self(a,x)...);
    return r;
}
//! @brief Optimisation for a single field argument.
template <typename F, typename A>
field<local_result<F,A>>
map_hood(F&& op, const field<A>& a) {
    field<local_result<F,A>> r(op(other(a)));
    for (const auto& x : a.m_data) r.m_data[x.first] = op(x.second);
    return r;
}
//! @brief Optimisation for a single field argument in a binary operator.
template <typename F, typename A, typename B>
if_local<B, field<local_result<F,A,B>>>
map_hood(F&& op, const field<A>& a, const B& b) {
    field<local_result<F,A,B>> r(op(other(a), b));
    for (const auto& x : a.m_data) r.m_data[x.first] = op(x.second, b);
    return r;
}
//! @brief Optimisation for a single field argument in a binary operator.
template <typename F, typename A, typename B>
if_local<A, field<local_result<F,A,B>>>
map_hood(F&& op, const A& a, const field<B>& b) {
    field<local_result<F,A,B>> r(op(a, other(b)));
    for (const auto& x : b.m_data) r.m_data[x.first] = op(a, x.second);
    return r;
}
//@}

/**
 * @name mod_hood
 *
 * Modifies a field in-place, by applying an operator pointwise (with a sequence of parameters).
 */
//@{
//! @brief General case.
template <typename F, typename A, typename... B>
field<A>& mod_hood(F&& op, field<A>& a, const B&... b) {
    for (device_t x : details::joined_domain(a, b...))
        details::self(a,x) = op(details::self(a,x), details::self(b,x)...);
    details::other(a) = op(other(a), other(b)...);
    return a;
}
//! @brief Optimization for unary operators.
template <typename F, typename A>
field<A>& mod_hood(F&& op, field<A>& a) {
    for (auto& x : a.m_data) x.second = op(x.second);
    a.def = op(a.def);
    return a;
}
//! @brief Optimization for binary operators with a local argument.
template <typename F, typename A, typename B>
if_local<B, field<A>&> mod_hood(F&& op, field<A>& a, const B& b) {
    for (auto& x : a.m_data) x.second = op(x.second, b);
    a.def = op(a.def, b);
    return a;
}
//@}

//! @brief Prints a field in dictionary-like format.
template <typename A>
std::ostream& operator<<(std::ostream& o, const field<A>& x) {
    o << "{";
    for (const auto& i : x.m_data) {
        o << i.first << ":" << i.second << ", ";
    }
    return o << "*:" << x.def << "}";
}


//! @cond INTERNAL
#define _UOP_TYPE(op,A)                                                     \
field<decltype(op std::declval<A>())>

#define _BOP_TYPE(A,op,B)                                                   \
field<decltype(std::declval<A>() op std::declval<B>())>

#define _BOP_IFTA(A,op,B)                                                   \
std::enable_if_t<not common::has_template<field,A> and not std::is_convertible<A,std::ostream>::value, _BOP_TYPE(A,op,B)>

#define _BOP_IFTB(A,op,B)                                                   \
if_local<B, _BOP_TYPE(A,op,B)>
//! @endcond

/**
 * @brief Overloads unary operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_UOP(op)                                                        \
template <typename A>                                                       \
_UOP_TYPE(op,A) operator op(const field<A>& x) {                            \
    return map_hood([] (const A& a) {return op a;}, x);                     \
}

/**
 * @brief Overloads binary operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_BOP(op)                                                        \
template <typename A, typename B>                                           \
_BOP_TYPE(A,op,B) operator op(const field<A>& x, const field<B>& y) {       \
    return map_hood([](const A& a, const B& b) { return a op b; }, x, y);   \
}                                                                           \
template <typename A, typename B>                                           \
_BOP_IFTA(A,op,B) operator op(const A& x, const field<B>& y) {              \
    return map_hood([&x](const B& b) { return x op b; }, y);                \
}                                                                           \
template <typename A, typename B>                                           \
_BOP_IFTB(A,op,B) operator op(const field<A>& x, const B& y) {              \
    return map_hood([&y](const A& a) { return a op y; }, x);                \
}

/**
 * @brief Overloads composite assignment operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_IOP(op)                                                        \
template <typename A, typename B>                                           \
_BOP_TYPE(A,op,B) operator op##=(field<A>& x, const field<B>& y) {          \
    return mod_hood([](const A& a, const B& b) { return a op b; }, x, y);   \
}                                                                           \
template <typename A, typename B>                                           \
_BOP_IFTB(A,op,B) operator op##=(field<A>& x, const B& y) {                 \
    return mod_hood([&y](const A& a) { return a op y; }, x);                \
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

#undef _UOP_TYPE
#undef _BOP_TYPE
#undef _BOP_IFTA
#undef _BOP_IFTB
#undef _DEF_UOP
#undef _DEF_BOP
#undef _DEF_IOP


}

#endif // FCPP_DATA_FIELD_H_
