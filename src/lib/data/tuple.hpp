// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file tuple.hpp
 * @brief Implementation of the `tuple` class extending `std::tuple` with point-wise operators.
 */

#ifndef FCPP_DATA_TUPLE_H_
#define FCPP_DATA_TUPLE_H_

#include <tuple>
#include <type_traits>

#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Object holding a collection of elements of possibly different types, as `std::tuple` with point-wise operators.
 */
//! @{
//! @brief General case.
template <typename... Ts>
class tuple {
    //! @cond INTERNAL
    template <typename... Us> friend class tuple;
    //! @endcond

  public:
    //! @brief constructors
    //! @{
    //! @brief Default constructor.
    constexpr tuple() = default;
    //! @brief Copy constructor.
    tuple(tuple const&) = default;
    //! @brief Move constructor.
    tuple(tuple&&) = default;
    //! @brief Implicit conversion copy constructor.
    template <typename... Us, typename = std::enable_if_t<common::number_all_true<std::is_convertible<Us,Ts>::value...>>>
    tuple(tuple<Us...> const& t) : m_tuple(t.m_tuple) {}
    //! @brief Implicit conversion move constructor.
    template <typename... Us, typename = std::enable_if_t<common::number_all_true<std::is_convertible<Us,Ts>::value...>>>
    tuple(tuple<Us...>&& t) : m_tuple(std::move(t.m_tuple)) {}
    //! @brief Initialization copy constructor.
    tuple(Ts const&... xs) : m_tuple(xs...) {}
    //! @brief Initialization convert constructor.
    template <typename... Us, typename = std::enable_if_t<common::number_all_true<std::is_convertible<Us,Ts>::value...>>>
    tuple(Us&&... xs) : m_tuple(std::forward<Us>(xs)...) {}
    //! @}

    //! @brief assignment
    //! @{
    //! @brief Copy assignment.
    tuple& operator=(tuple const& t) = default;
    //! @brief Move assignment.
    tuple& operator=(tuple&& t) = default;
    //! @brief Implicit conversion copy assignment.
    template <typename... Us>
    tuple& operator=(tuple<Us...> const& t) {
        m_tuple = t.m_tuple;
        return *this;
    }
    //! @brief Implicit conversion move assignment.
    template <typename... Us>
    tuple& operator=(tuple<Us...>&& t) {
        m_tuple = std::move(t.m_tuple);
        return *this;
    }
    //! @}

    //! @brief Exchanges the content of the `tuple` objects.
    void swap(tuple& t) {
        m_tuple.swap(t.m_tuple);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_tuple;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_tuple;
    }

  private:
    std::tuple<Ts...> m_tuple;
};

//! @brief Special case of empty tuple.
template <>
class tuple<> {
    //! @cond INTERNAL
    template <typename... Us> friend class tuple;
    //! @endcond

  public:
    //! @brief constructors
    //! @{
    //! @brief Default constructor.
    constexpr tuple() = default;
    //! @brief Copy constructor.
    tuple(tuple const&) = default;
    //! @brief Move constructor.
    tuple(tuple&&) = default;
    //! @}

    //! @brief assignment
    //! @{
    //! @brief Copy assignment.
    tuple& operator=(tuple const& t) = default;
    //! @brief Move assignment.
    tuple& operator=(tuple&& t) = default;
    //! @}

    //! @brief Exchanges the content of the `tuple` objects.
    void swap(tuple&) {}

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S&) const {}
};
//! @}


//! @cond INTERNAL
namespace details {
    //! @brief Helper class unwrapping `std::reference_wrapper` objects (general case).
    template <typename T>
    struct unwrap_refwrapper {
        using type = T;
    };
     //! @brief Helper class unwrapping `std::reference_wrapper` objects (special case).
    template <typename T>
    struct unwrap_refwrapper<std::reference_wrapper<T>> {
        using type = T&;
    };

    //! @brief Decays a type, eliminating reference wrappers.
    template <typename T>
    using special_decay_t = typename unwrap_refwrapper<std::decay_t<T>>::type;

    //! @brief Converts an `std::tuple` into a `tuple` (rvalue).
    template <typename... Ts>
    tuple<Ts...> tuple_promote(std::tuple<Ts...>&& t) {
        return (tuple<Ts...>&&)t;
    }
    //! @brief Converts an `std::tuple` into a `tuple` (lvalue).
    template <typename... Ts>
    tuple<Ts...> const& tuple_promote(std::tuple<Ts...> const& t) {
        return (tuple<Ts...> const&)t;
    }

    //! @brief Converts a `tuple` into an `std::tuple` (rvalue).
    template <typename... Ts>
    std::tuple<Ts...> tuple_demote(tuple<Ts...>&& t) {
        return (std::tuple<Ts...>&&)t;
    }
    //! @brief Converts a `tuple` into an `std::tuple` (lvalue).
    template <typename... Ts>
    std::tuple<Ts...> const& tuple_demote(tuple<Ts...> const& t) {
        return (std::tuple<Ts...> const&)t;
    }
}
//! @endcond


//! @brief Constructs an appropriate `tuple` containing the elements specified in xs, as `std::make_tuple`.
template <typename... Ts>
tuple<details::special_decay_t<Ts>...> make_tuple(Ts&&... xs) {
    return {std::forward<Ts>(xs)...};
}


//! @brief Construct a `tuple` suitable to be forwarded as argument to a function, as `std::forward_as_tuple`.
template <typename... Ts>
constexpr tuple<Ts&&...> forward_as_tuple(Ts&&... xs) noexcept {
    return {std::forward<Ts>(xs)...};
}


//! @brief Construct a `tuple` referencing lvalues and moving rvalues.
template <typename... Ts>
constexpr tuple<Ts...> capture_as_tuple(Ts&&... xs) noexcept {
    return {std::forward<Ts>(xs)...};
}


//! @brief Concatenate the tuples given as arguments, as `std::tuple_cat`.
template <typename... Ts>
auto tuple_cat(Ts&&... ts) {
    return details::tuple_promote(std::tuple_cat(details::tuple_demote(std::forward<Ts>(ts))...));
}


/**
 * @name tuple element type
 *
 * Access the type of the n-th element of a tuple.
 */
//! @{
//! @brief general form
template <size_t n, typename T>
struct tuple_element;

//! @brief active form
template <size_t n, typename... Ts>
struct tuple_element<n, tuple<Ts...>> : public std::tuple_element<n, std::tuple<Ts...>> {};

//! @brief type definition
template <size_t n, typename T>
using tuple_element_t = typename tuple_element<n, T>::type;
//! @}


/**
 * @name get element
 *
 * Accesses an element of a tuple, as `std::get`.
 */
//! @{
//! @brief lvalue overload
template <size_t n, typename... Ts>
auto& get(tuple<Ts...>& t) noexcept {
    return std::get<n>((std::tuple<Ts...>&)t);
}
//! @brief rvalue overload
template <size_t n, typename... Ts>
auto&& get(tuple<Ts...>&& t) noexcept {
    return std::get<n>((std::tuple<Ts...>&&)t);
}
//! @brief const lvalue overload
template <size_t n, typename... Ts>
auto const& get(tuple<Ts...> const& t) noexcept {
    return std::get<n>((std::tuple<Ts...> const&)t);
}
//! @}


//! @brief Object ignoring any value assigned to it, as `std::ignore`.
using std::ignore;


/**
 * @brief Tie arguments to tuple elements, as `std::tie`.
 *
 * Constructs a `tuple` object whose elements are references to the arguments, in the same order.
 * This allows a set of objects to act as a tuple, which is especially useful to unpack `tuple` objects.
 * The special constant `ignore` can be used to specify elements of a tuple to be ignored instead of
 * tied to a specific object.
 */
template <typename... Ts>
constexpr tuple<Ts&...> tie(Ts&... args) noexcept {
    return {args...};
}


//! @brief Swaps contents of x and y, as `std::swap`.
template <typename... Ts>
void swap(tuple<Ts...>& x, tuple<Ts...>& y) noexcept {
    x.swap(y);
}


//! @cond INTERNAL
namespace details {
    //! @brief A reference to a tuple, together with an index sequence as parameter.
    template <typename T, typename I>
    struct tuple_wrapper {
        //! @brief Constructor from a reference.
        tuple_wrapper(T t) : m_tuple(static_cast<T>(t)) {}

        //! @brief Accesses the tuple.
        T tuple() const {
            return static_cast<T>(m_tuple);
        }

      private:
        //! @brief The internal reference.
        T m_tuple;
    };

    //! @brief Wraps a tuple, depending on its lvalue/rvalue/const lvalue status.
    //! @{
    //! @brief lvalue overload
    template <typename... Ts>
    inline tuple_wrapper<tuple<Ts...>&, std::make_index_sequence<sizeof...(Ts)>> tuple_wrap(tuple<Ts...>& x) {
        return {x};
    }
    //! @brief rvalue overload
    template <typename... Ts>
    inline tuple_wrapper<tuple<Ts...>&&, std::make_index_sequence<sizeof...(Ts)>> tuple_wrap(tuple<Ts...>&& x) {
        return {std::move(x)};
    }
    //! @brief const lvalue overload
    template <typename... Ts>
    inline tuple_wrapper<tuple<Ts...> const&, std::make_index_sequence<sizeof...(Ts)>> tuple_wrap(tuple<Ts...> const& x) {
        return {x};
    }
    //! @}

    //! @brief Macros defining every operator on a tuple_wrapper pointwise on the referenced tuple.
    //! @{
    #define _DEF_UOP(op)                                                        \
    template <typename T, size_t... is>                                         \
    auto operator op(tuple_wrapper<T, std::index_sequence<is...>> x) {          \
        return fcpp::make_tuple((op get<is>(x.tuple()))...);                    \
    }

    #define _DEF_BOP(op)                                                        \
    template <typename T, typename U, size_t... is>                             \
    auto operator op(tuple_wrapper<T, std::index_sequence<is...>> x,            \
                     tuple_wrapper<U, std::index_sequence<is...>> y) {          \
        return fcpp::make_tuple((get<is>(x.tuple()) op get<is>(y.tuple()))...); \
    }

    #define _DEF_IOP(op)                                                        \
    template <typename T, typename U, size_t... is>                             \
    auto operator op##=(tuple_wrapper<T, std::index_sequence<is...>> x,         \
                        tuple_wrapper<U, std::index_sequence<is...>> y) {       \
        common::ignore_args((get<is>(x.tuple()) op##= get<is>(y.tuple()))...);  \
        return x.tuple();                                                       \
    }

    template <typename T, typename U, typename I>
    inline auto tw_lt(tuple_wrapper<T, I> const&, tuple_wrapper<U, I> const&, std::index_sequence<>) {
        return false;
    }

    template <typename T, typename U, typename I, size_t i>
    inline auto tw_lt(tuple_wrapper<T, I> const& x, tuple_wrapper<U, I> const& y, std::index_sequence<i>) {
        return get<i>(x.tuple()) < get<i>(y.tuple());
    }

    template <typename T, typename U, typename I, size_t i1, size_t i2, size_t... is>
    inline auto tw_lt(tuple_wrapper<T, I> const& x, tuple_wrapper<U, I> const& y, std::index_sequence<i1, i2, is...>) {
        return ( get<i1>(x.tuple()) < get<i1>(y.tuple()) ) or (( get<i1>(x.tuple()) == get<i1>(y.tuple()) ) and tw_lt(x, y, std::index_sequence<i2, is...>{}));
    }

    template <typename T, typename U, typename I>
    auto operator <(tuple_wrapper<T, I> const& x, tuple_wrapper<U, I> const& y) {
        return tw_lt(x, y, I{});
    }

    template <typename T, typename U, typename I>
    auto operator >(tuple_wrapper<T, I> x, tuple_wrapper<U, I> y) {
        return tw_lt(y, x, I{});
    }

    template <typename T, typename U, typename I>
    inline auto tw_le(tuple_wrapper<T, I> const&, tuple_wrapper<U, I> const&, std::index_sequence<>) {
        return true;
    }

    template <typename T, typename U, typename I, size_t i>
    inline auto tw_le(tuple_wrapper<T, I> const& x, tuple_wrapper<U, I> const& y, std::index_sequence<i>) {
        return get<i>(x.tuple()) <= get<i>(y.tuple());
    }

    template <typename T, typename U, typename I, size_t i1, size_t i2, size_t... is>
    inline auto tw_le(tuple_wrapper<T, I> const& x, tuple_wrapper<U, I> const& y, std::index_sequence<i1, i2, is...>) {
        return ( get<i1>(x.tuple()) < get<i1>(y.tuple()) ) or (( get<i1>(x.tuple()) == get<i1>(y.tuple()) ) and tw_le(x, y, std::index_sequence<i2, is...>{}));
    }

    template <typename T, typename U, typename I>
    auto operator <=(tuple_wrapper<T, I> x, tuple_wrapper<U, I> y) {
        return tw_le(x, y, I{});
    }

    template <typename T, typename U, typename I>
    auto operator >=(tuple_wrapper<T, I> x, tuple_wrapper<U, I> y) {
        return tw_le(y, x, I{});
    }

    template <typename T, typename U, typename I>
    inline auto tw_eq(tuple_wrapper<T, I> const&, tuple_wrapper<U, I> const&, std::index_sequence<>) {
        return true;
    }

    template <typename T, typename U, typename I, size_t i, size_t... is>
    inline auto tw_eq(tuple_wrapper<T, I> const& x, tuple_wrapper<U, I> const& y, std::index_sequence<i, is...>) {
        return ( get<i>(x.tuple()) == get<i>(y.tuple()) ) and tw_eq(x, y, std::index_sequence<is...>{});
    }

    template <typename T, typename U, typename I>
    auto operator ==(tuple_wrapper<T, I> const& x, tuple_wrapper<U, I> const& y) {
        return tw_eq(x, y, I{});
    }

    template <typename T, typename U, typename I>
    inline auto tw_ne(tuple_wrapper<T, I> const&, tuple_wrapper<U, I> const&, std::index_sequence<>) {
        return false;
    }

    template <typename T, typename U, typename I, size_t i, size_t... is>
    inline auto tw_ne(tuple_wrapper<T, I> const& x, tuple_wrapper<U, I> const& y, std::index_sequence<i, is...>) {
        return ( get<i>(x.tuple()) != get<i>(y.tuple()) ) or tw_ne(x, y, std::index_sequence<is...>{});
    }

    template <typename T, typename U, typename I>
    auto operator !=(tuple_wrapper<T, I> x, tuple_wrapper<U, I> y) {
        return tw_ne(x, y, I{});
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

    #undef _DEF_UOP
    #undef _DEF_BOP
    #undef _DEF_IOP
    //! @}
}
//! @endcond


//! @cond INTERNAL
//! @brief Macros defining every operator pointwise on a tuple.
#define _DEF_UOP(op)                                                                \
template <typename... Ts>                                                           \
auto operator op(tuple<Ts...>&& x) {                                                \
    return op details::tuple_wrap(std::move(x));                                    \
}                                                                                   \
template <typename... Ts>                                                           \
auto operator op(tuple<Ts...> const& x) {                                           \
    return op details::tuple_wrap(x);                                               \
}

#define _DEF_BOP(op)                                                                \
template <typename... Ts, typename... Us,                                           \
          typename = std::enable_if_t<sizeof...(Ts) == sizeof...(Us)>>              \
auto operator op(tuple<Ts...> const& x, tuple<Us...> const& y) {                    \
    return details::tuple_wrap(x) op details::tuple_wrap(y);                        \
}                                                                                   \
template <typename... Ts, typename... Us,                                           \
          typename = std::enable_if_t<sizeof...(Ts) == sizeof...(Us)>>              \
auto operator op(tuple<Ts...>&& x, tuple<Us...> const& y) {                         \
    return details::tuple_wrap(std::move(x)) op details::tuple_wrap(y);             \
}                                                                                   \
template <typename... Ts, typename... Us,                                           \
          typename = std::enable_if_t<sizeof...(Ts) == sizeof...(Us)>>              \
auto operator op(tuple<Ts...> const& x, tuple<Us...>&& y) {                         \
    return details::tuple_wrap(x) op details::tuple_wrap(std::move(y));             \
}                                                                                   \
template <typename... Ts, typename... Us,                                           \
          typename = std::enable_if_t<sizeof...(Ts) == sizeof...(Us)>>              \
auto operator op(tuple<Ts...>&& x, tuple<Us...>&& y) {                              \
    return details::tuple_wrap(std::move(x)) op details::tuple_wrap(std::move(y));  \
}

#define _DEF_IOP(op)                                                                \
template <typename... Ts, typename... Us,                                           \
          typename = std::enable_if_t<sizeof...(Ts) == sizeof...(Us)>>              \
auto operator op##=(tuple<Ts...>& x, tuple<Us...> const& y) {                       \
    return details::tuple_wrap(x) op##= details::tuple_wrap(y);                     \
}                                                                                   \
template <typename... Ts, typename... Us,                                           \
          typename = std::enable_if_t<sizeof...(Ts) == sizeof...(Us)>>              \
auto operator op##=(tuple<Ts...>& x, tuple<Us...>&& y) {                            \
    return details::tuple_wrap(x) op##= details::tuple_wrap(std::move(y));          \
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

#undef _DEF_UOP
#undef _DEF_BOP
#undef _DEF_IOP
//! @endcond


}

#endif // FCPP_DATA_TUPLE_H_
