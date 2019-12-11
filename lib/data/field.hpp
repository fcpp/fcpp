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


#if   FCPP_SETTING_DEVICE == 8
    typedef uint8_t device_t;
#elif FCPP_SETTING_DEVICE == 16
    typedef uint16_t device_t;
#elif FCPP_SETTING_DEVICE == 24
    typedef uint32_t device_t;
#elif FCPP_SETTING_DEVICE == 32
    typedef uint32_t device_t;
#elif FCPP_SETTING_DEVICE == 48
    typedef uint64_t device_t;
#elif FCPP_SETTING_DEVICE == 64
    typedef uint64_t device_t;
#else
    static_assert(false, "invalid value for FCPP_SETTING_DEVICE");
    //! @brief Type for device identifiers (depends on @ref FCPP_SETTING_DEVICE).
    typedef uint64_t device_t;
#endif


//! @cond INTERNAL
template<typename T> class field;
//! @endcond


/**
 * @brief Namespace containing implementation details, which should <b>never</b> be used in FCPP programs.
 */
namespace details {
//! @cond INTERNAL
    template <class T>
    struct field_base {};
    
    template <typename A>
    field<A> make_field(A, std::unordered_map<device_t, A>);
    
    template <typename A>
    A& self(field<A>&, device_t);
    
    template <typename A>
    const A& self(const field<A>&, device_t);
    
    template <typename A>
    typename std::enable_if<not has_template<field, A>, const A&>::type
    self(const A&, device_t);
    
    template <typename A>
    field<A> align(field<A>&&, const std::unordered_set<device_t>&);
    
    template <typename A>
    field<A> align(const field<A>&, const std::unordered_set<device_t>&);
    
    template <typename A>
    typename std::enable_if<not has_template<field, A>, const A&>::type
    align(const A&, const std::unordered_set<device_t>&);

    template <typename... T>
    void ignore(T...) {}
    
    template <typename T>
    bool add_domain(const std::unordered_map<device_t,T>& data, std::unordered_set<device_t>& domain) {
        for (const auto& t : data) domain.insert(t.first);
        return true;
    }
    
    template <typename F, typename A>
    typename std::result_of<F(A,A)>::type
    fold_hood(F&&, const field<A>&, const std::unordered_set<device_t>&);
    
    template <typename F, typename A>
    typename std::enable_if<not has_template<field, A>, typename std::result_of<F(A,A)>::type>::type
    fold_hood(F&&, const A&, const std::unordered_set<device_t>&);
//! @endcond
}


/**
 * @brief Class representing a neighboring field of T values.
 */
template <typename T>
class field : public details::field_base<T> {
    static_assert(not has_template<field, T>, "cannot instantiate a field of fields");
    
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
    template <typename A, typename = typename std::enable_if<std::is_convertible<T,A>::value>::type>
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
    //! @brief Write access.
    template<typename A>
    friend A& other(field<A>&);
    
    //! @brief Read-only access.
    template<typename A>
    friend const A& other(const field<A>&);
    //@}
    
    //! @cond INTERNAL
    template<typename A>
    friend A& details::self(field<A>&, device_t);
    
    template<typename A>
    friend const A& details::self(const field<A>&, device_t);
    
    template<typename A>
    friend field<A> details::align(field<A>&&, const std::unordered_set<device_t>&);
    
    template<typename A>
    friend field<A> details::align(const field<A>&, const std::unordered_set<device_t>&);
    //! @endcond
    
    /**
     * @name map_hood
     *
     * Applies an operator pointwise on a sequence of fields.
     */
    //@{
    template <typename F, typename... A>
    friend field<typename std::result_of<F(A...)>::type> map_hood(F&&, const field<A>&...);
    
    template <typename F, typename A>
    friend field<typename std::result_of<F(A)>::type> map_hood(F&&, const field<A>&);
    //@}
    
    /**
     * @name mod_hood
     *
     * Modifies a field in-place, by applying an operator pointwise (with a sequence of parameters).
     */
    //@{
    //! @brief General case.
    template <typename F, typename A, typename... B>
    friend field<A>& mod_hood(F&&, field<A>&, const field<B>&...);
    
    //! @brief Optimization for unary operators.
    template <typename F, typename A>
    friend field<A>& mod_hood(F&&, field<A>&);
    //@}
    
    //! @cond INTERNAL
    template <typename F, typename A>
    friend typename std::result_of<F(A,A)>::type details::fold_hood(F&&, const field<A>&, const std::unordered_set<device_t>&);
    //! @endcond
    
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
//! @brief Write access.
template <typename A>
A& other(field<A>& x) {
    return x.def;
}

//! @brief Read-only access.
template <typename A>
const A& other(const field<A>& x) {
    return x.def;
}

//! @brief Write access for non-field values (treated as constant fields).
template <typename A>
typename std::enable_if<not has_template<field, A>, A&>::type other(A& x) {
    return x;
}

//! @brief Read-only access for non-field values (treated as constant fields).
template <typename A>
typename std::enable_if<not has_template<field, A>, const A&>::type other(const A& x) {
    return x;
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
const A& details::self(const field<A>& x, device_t i) {
    if (x.m_data.count(i)) return x.m_data.at(i);
    return x.def;
}

template <typename A>
typename std::enable_if<not has_template<field, A>, const A&>::type
details::self(const A& x, device_t) {
    return x;
}

/*
 * Computes the restriction of a field to a given domain.
 * The resulting field has exactly the given domain.
 */
template <typename A>
field<A> details::align(field<A>&& x, const std::unordered_set<device_t>& s) {
    for (auto it = x.m_data.begin(); it != x.m_data.end(); ) {
        if (s.count(it->first)) ++it;
        else it = x.m_data.erase(it);
    }
    for (device_t i : s) if (not x.m_data.count(i)) x.m_data[i] = x.def;
    return x;
}

template <typename A>
field<A> details::align(const field<A>& x, const std::unordered_set<device_t>& s) {
    return details::align(field<A>(x), s);
}

template <typename A>
typename std::enable_if<not has_template<field, A>, const A&>::type
details::align(const A& x, const std::unordered_set<device_t>&) {
    return x;
}
//! @endcond

/**
 * @name map_hood
 *
 * Applies an operator pointwise on a sequence of fields.
 */
//@{
template <typename F, typename... A>
field<typename std::result_of<F(A...)>::type> map_hood(F&& op, const field<A>&... args) {
    field<typename std::result_of<F(A...)>::type> r(op(args.def...));
    std::unordered_set<device_t> domain;
    details::ignore(details::add_domain(args.m_data, domain)...);
    for (device_t x : domain) r.m_data[x] = op(details::self(args,x)...);
    return r;
}

template <typename F, typename A>
field<typename std::result_of<F(A)>::type> map_hood(F&& op, const field<A>& a) {
    field<typename std::result_of<F(A)>::type> r(op(a.def));
    for (const auto& x : a.m_data) r.m_data[x.first] = op(x.second);
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
field<A>& mod_hood(F&& op, field<A>& f, const field<B>&... args) {
    std::unordered_set<device_t> domain;
    details::ignore(details::add_domain(f.m_data, domain), details::add_domain(args.m_data, domain)...);
    for (device_t x : domain) {
        A z = op(details::self(f,x), details::self(args,x)...);
        f.m_data[x] = z;
    }
    f.def = op(f.def, args.def...);
    return f;
}

//! @brief Optimization for unary operators.
template <typename F, typename A>
field<A>& mod_hood(F&& op, field<A>& f) {
    f.def = op(f.def);
    for (auto& x : f.m_data) x.second = op(x.second);
    return f;
}
//@}

/*
 * Reduces the values in a part of a field (determined by domain) to a single value through a binary operation.
 */
//! @cond INTERNAL
template <typename F, typename A>
typename std::result_of<F(A,A)>::type details::fold_hood(F&& op, const field<A>& f, const std::unordered_set<device_t>& domain) {
    auto it = domain.begin();
    typename std::result_of<F(A,A)>::type res = details::self(f, *it);
    for (++it; it != domain.end(); ++it)
        res = op(details::self(f, *it), res);
    return res;
}

template <typename F, typename A>
typename std::enable_if<not has_template<field, A>, typename std::result_of<F(A,A)>::type>::type
details::fold_hood(F&& op, const A& x, const std::unordered_set<device_t>& domain) {
    int n = domain.size();
    typename std::result_of<F(A,A)>::type res = x;
    for (--n; n>0; --n) res = op(x, res);
    return res;
}
//! @endcond

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
typename std::enable_if<not has_template<field,A> and not std::is_convertible<A,std::ostream>::value, _BOP_TYPE(A,op,B)>::type

#define _BOP_IFTB(A,op,B)                                                   \
typename std::enable_if<not has_template<field,B>, _BOP_TYPE(A,op,B)>::type
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
