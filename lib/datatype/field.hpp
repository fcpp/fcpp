// Copyright © 2017 Giorgio Audrito. All Rights Reserved.

// class field

// methods:
//  - constructors, assignment

// external methods:
//  - map_hood(fun, fields...): return result of pointwise mapping
//  - mod_hood(fun, field, fields...): modify field in-place according to fun
//  - fold_hood(fun, field, people): align and folds

//  - other(data): access to default
//  - self(data, people): access to data if not field, data[people] otherwise
//  - align(data, people): data if not field, data.align(people) otherwise
//  - every possible operator is overloaded

// WHERE AN IMPLICIT CONTEXT IS AVAILABLE (defined in program class):
//  - self(data) senza people
//  - align(data) senza people
//  - fold_hood(fun, field) senza people

#ifndef FCPP_DATATYPES_FIELD_H_
#define FCPP_DATATYPES_FIELD_H_

#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include "lib/util/traits.hpp"


namespace fcpp {


template<typename T> class field;


namespace details {
    template <typename A>
    field<A> make_field(A, std::unordered_map<size_t, A>);
    
    template <typename A>
    A& self(field<A>&, size_t);
    
    template <typename A>
    const A& self(const field<A>&, size_t);
    
    template <typename A>
    typename std::enable_if<not is_template<field, A>, A&>::type self(A& x, size_t i) {
        return x;
    }
    
    template <typename A>
    typename std::enable_if<not is_template<field, A>, const A&>::type self(const A& x, size_t i) {
        return x;
    }
    
    template <typename A>
    field<A> align(field<A>&&, const std::unordered_set<size_t>&);
    
    template <typename A>
    field<A> align(const field<A>&, const std::unordered_set<size_t>&);
    
    template <typename A>
    typename std::enable_if<not is_template<field, A>, A&>::type align(A& x, const std::unordered_set<size_t>& s) {
        return x;
    }
    
    template <typename A>
    typename std::enable_if<not is_template<field, A>, const A&>::type align(const A& x, const std::unordered_set<size_t>& s) {
        return x;
    }

    template <typename... T>
    void ignore(T... t) {}
    
    template <typename T>
    bool add_domain(const std::unordered_map<size_t,T>& data, std::unordered_set<size_t>& domain) {
        for (const auto& t : data) domain.insert(t.first);
        return true;
    }
    
    template <typename F, typename A>
    typename std::result_of<F(A,A)>::type fold_hood(F&&, const field<A>&, const std::unordered_set<size_t>&);
}


template <typename T>
class field {
    template <typename A>
    friend class field;
    
  private:
    std::unordered_map<size_t, T> data;
    T def;
    
    field(T _def, std::unordered_map<size_t, T> _data) : data(_data), def(_def) {}
  
  public:
    // deprecated but convenient sometimes
    field() = default;
    
    field(const T& d) : data(), def(d) {}
    
    field(const field<T>&) = default;
    
    field(field<T>&&) = default;
    
    field<T>& operator=(const field<T>&) = default;
    
    field<T>& operator=(field<T>&&) = default;
    
    template <typename A, typename = typename std::enable_if<std::is_convertible<T,A>::value>::type>
    operator field<A>() const {
        field<A> r(static_cast<A>(def));
        for (const auto& x : data)
            r.data[x.first] = static_cast<A>(x.second);
        return r;
    }
    
    template <typename A>
    friend field<A> details::make_field(A, std::unordered_map<size_t, A>);
    
    template<typename A>
    friend A& other(field<A>&);
    
    template<typename A>
    friend const A& other(const field<A>&);
    
    template<typename A>
    friend A& details::self(field<A>&, size_t);
    
    template<typename A>
    friend const A& details::self(const field<A>&, size_t);
    
    template<typename A>
    friend field<A> details::align(field<A>&&, const std::unordered_set<size_t>&);
    
    template<typename A>
    friend field<A> details::align(const field<A>&, const std::unordered_set<size_t>&);
    
    template <typename F, typename... A>
    friend field<typename std::result_of<F(A...)>::type> map_hood(F&&, const field<A>&...);
    
    template <typename F, typename A>
    friend field<typename std::result_of<F(A)>::type> map_hood(F&&, const field<A>&);
    
    template <typename F, typename A, typename... B>
    friend field<A>& mod_hood(F&&, field<A>&, const field<B>&...);
    
    template <typename F, typename A>
    friend field<A>& mod_hood(F&&, field<A>&);
    
    template <typename F, typename A>
    friend typename std::result_of<F(A,A)>::type details::fold_hood(F&&, const field<A>&, const std::unordered_set<size_t>&);
    
    template <typename A>
    friend std::ostream& operator<<(std::ostream&, const field<A>&);
};


template <typename A>
field<A> details::make_field(A def, std::unordered_map<size_t, A> data) {
    field<A> r(def, data);
    return r;
}

template <typename A>
A& other(field<A>& x) {
    return x.def;
}

template <typename A>
const A& other(const field<A>& x) {
    return x.def;
}

template <typename A>
typename std::enable_if<not is_template<field, A>, A&>::type other(A& x) {
    return x;
}

template <typename A>
typename std::enable_if<not is_template<field, A>, const A&>::type other(const A& x) {
    return x;
}

template <typename A>
A& details::self(field<A>& x, size_t i) {
    if (x.data.count(i)) return x.data.at(i);
    return x.data[i] = x.def;
}

template <typename A>
const A& details::self(const field<A>& x, size_t i) {
    if (x.data.count(i)) return x.data.at(i);
    return x.def;
}

template <typename A>
field<A> details::align(field<A>&& x, const std::unordered_set<size_t>& s) {
    for (auto it = x.data.begin(); it != x.data.end(); ) {
        if (s.count(it->first)) ++it;
        else it = x.data.erase(it);
    }
    return x;
}

template <typename A>
field<A> details::align(const field<A>& x, const std::unordered_set<size_t>& s) {
    return details::align(field<A>(x), s);
}

template <typename F, typename... A>
field<typename std::result_of<F(A...)>::type> map_hood(F&& op, const field<A>&... args) {
    field<typename std::result_of<F(A...)>::type> r(op(args.def...));
    std::unordered_set<size_t> domain;
    details::ignore(details::add_domain(args.data, domain)...);
    for (size_t x : domain) r.data[x] = op(details::self(args,x)...);
    return r;
}

template <typename F, typename A>
field<typename std::result_of<F(A)>::type> map_hood(F&& op, const field<A>& a) {
    field<typename std::result_of<F(A)>::type> r(op(a.def));
    for (const auto& x : a.data) r.data[x.first] = op(x.second);
    return r;
}
template <typename F, typename A, typename... B>
field<A>& mod_hood(F&& op, field<A>& f, const field<B>&... args) {
    std::unordered_set<size_t> domain;
    details::ignore(details::add_domain(f.data, domain), details::add_domain(args.data, domain)...);
    for (size_t x : domain) {
        A z = op(details::self(f,x), details::self(args,x)...);
        f.data[x] = z;
    }
    f.def = op(f.def, args.def...);
    return f;
}

template <typename F, typename A>
field<A>& mod_hood(F&& op, field<A>& f) {
    f.def = op(f.def);
    for (auto& x : f.data) x.second = op(x.second);
    return f;
}

// assumes that domain is non-empty
template <typename F, typename A>
typename std::result_of<F(A,A)>::type details::fold_hood(F&& op, const field<A>& f, const std::unordered_set<size_t>& domain) {
    auto it = domain.begin();
    typename std::result_of<F(A,A)>::type res = details::self(f, *it);
    for (++it; it != domain.end(); ++it)
        res = op(details::self(f, *it), res);
    return res;
}

template <typename A>
std::ostream& operator<<(std::ostream& o, const field<A>& x) {
    o << "{";
    for (const auto& i : x.data) {
        o << i.first << ":" << i.second << ", ";
    }
    return o << "*:" << x.def << "}";
}


#define _UOP_TYPE(op,A)                                                     \
field<decltype(op std::declval<A>())>

#define _BOP_TYPE(A,op,B)                                                   \
field<decltype(std::declval<A>() op std::declval<B>())>

#define _BOP_IFTA(A,op,B)                                                   \
typename std::enable_if<not is_template<field,A> and not std::is_convertible<A,std::ostream>::value, _BOP_TYPE(A,op,B)>::type

#define _BOP_IFTB(A,op,B)                                                   \
typename std::enable_if<not is_template<field,B>, _BOP_TYPE(A,op,B)>::type

#define _DEF_UOP(op)                                                        \
template <typename A>                                                       \
_UOP_TYPE(op,A) operator op(const field<A>& x) {                            \
    return map_hood([] (const A& a) {return op a;}, x);                     \
}

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

#endif  // FCPP_DATATYPES_FIELD_H_
