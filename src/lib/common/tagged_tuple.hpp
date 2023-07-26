// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file tagged_tuple.hpp
 * @brief Implementation of the `tagged_tuple<Ts...>` class template for handling addressable tuple data.
 */

#ifndef FCPP_COMMON_TAGGED_TUPLE_H_
#define FCPP_COMMON_TAGGED_TUPLE_H_

#include <cstring>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>

#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


//! @brief Namespace of tags for common use.
namespace tags {
    //! @brief Tag for representing tagged tuples in dictionary format (default).
    struct dictionary_tuple {};

    //! @brief Tag for representing tagged tuples in assignment-list format.
    struct assignment_tuple {};

    //! @brief Tag for representing tagged tuples in compact underscore format.
    struct underscore_tuple {};

    //! @brief Tag for representing tagged tuples in arrowhead format.
    struct arrowhead_tuple {};

    //! @brief Tag for skipping tags in printing tuples.
    template <typename... Ts>
    struct skip_tags {};
}

//! @brief Tag for representing tagged tuples in dictionary format (default).
constexpr tags::dictionary_tuple dictionary_tuple{};
//! @brief Tag for representing tagged tuples in assignment-list format.
constexpr tags::assignment_tuple assignment_tuple{};
//! @brief Tag for representing tagged tuples in compact underscore format.
constexpr tags::underscore_tuple underscore_tuple{};
//! @brief Tag for representing tagged tuples in arrowhead format.
constexpr tags::arrowhead_tuple arrowhead_tuple{};
//! @brief Tag for skipping tags in printing tuples.
template <typename... Ts>
constexpr tags::skip_tags<Ts...> skip_tags{};


/**
 * @brief General form of the `tagged_tuple` class.
 *
 * @param S A `type_sequence` of types to be used as tags.
 * @param T A `type_sequence` of corresponding value types.
 */
template<typename S, typename T>
struct tagged_tuple;


/**
 * @name get
 *
 * Function getting elements of a tagged tuple by tag.
 * @param S The tag to be extracted.
 */
//! @{
//! @brief Write access.
template <typename S, typename... Ss, typename... Ts>
type_get<type_find<S, Ss...>, Ts...>&
get(tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>& t) {
    return std::get<type_find<S, Ss...>>(t);
}
//! @brief Move access.
template <typename S, typename... Ss, typename... Ts>
type_get<type_find<S, Ss...>, Ts...>&&
get(tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>&& t) {
    return std::get<type_find<S, Ss...>>(std::move(t));
}
//! @brief Const access.
template <typename S, typename... Ss, typename... Ts>
type_get<type_find<S, Ss...>, Ts...> const&
get(tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>> const& t) {
    return std::get<type_find<S, Ss...>>(t);
}
//! @}


//! @cond INTERNAL
namespace details {
    template <typename S, typename T>
    T&& get_or(S&&, T&& def, type_sequence<>) {
        return std::forward<T>(def);
    }
    template <typename S, typename T, typename U>
    auto&& get_or(S&& t, T&&, type_sequence<U>) {
        return get<U>(std::forward<S>(t));
    }
}
//! @endcond


/**
 * @name get_or
 *
 * Function getting elements of a tagged tuple by tag if present.
 * @param S The tag to be extracted.
 * @param def A default value if tag is missing.
 */
//! @{

//! @brief Const tuple overload.
template <typename S, typename Ss, typename Ts, typename T>
auto&& get_or(tagged_tuple<Ss, Ts> const& t, T&& def) {
    return details::get_or(t, std::forward<T>(def), typename Ss::template intersect<S>());
}
//! @brief Value tuple overload.
template <typename S, typename Ss, typename Ts, typename T>
auto&& get_or(tagged_tuple<Ss, Ts>&& t, T&& def) {
    return details::get_or(std::move(t), std::forward<T>(def), typename Ss::template intersect<S>());
}
//! @}


//! @cond INTERNAL
namespace details {
    template <typename S>
    wildcard& get_or_wildcard(S&&, type_sequence<>) {
        return declare_reference<wildcard>();
    }
    template <typename S, typename U>
    auto&& get_or_wildcard(S&& t, type_sequence<U>) {
        return get<U>(std::forward<S>(t));
    }
}
//! @endcond


/**
 * @name get_or_wildcard
 *
 * Function getting elements of a tagged tuple by tag if present.
 * If not present, a reference to a wildcard struct is returned to suppress compiler error messages.
 * @param S The tag to be extracted.
 */
//! @{
//! @brief Write access.
template <typename S, typename Ss, typename Ts>
auto& get_or_wildcard(tagged_tuple<Ss, Ts>& t) {
    return details::get_or_wildcard(t, typename Ss::template intersect<S>());
}
//! @brief Move access.
template <typename S, typename Ss, typename Ts>
auto&& get_or_wildcard(tagged_tuple<Ss, Ts>&& t) {
    return details::get_or_wildcard(std::move(t), typename Ss::template intersect<S>());
}
//! @brief Const access.
template <typename S, typename Ss, typename Ts>
auto const& get_or_wildcard(tagged_tuple<Ss, Ts> const& t) {
    return details::get_or_wildcard(t, typename Ss::template intersect<S>());
}
//! @}


//! @brief Utility function for creating tagged tuples.
template <typename... Ss, typename... Ts>
tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>> make_tagged_tuple(Ts... vs) {
    return {vs...};
}


//! @cond INTERNAL
namespace details {
    template <class... Ts>
    std::tuple<Ts...> capture_as_tuple(Ts&&... xs) {
        return {xs...};
    }

    // Extracts elements from a tagged tuple as a tuple.
    template <class T, class... Ss, class... Ts>
    auto tt_capture(T&& t, type_sequence<Ss...>, type_sequence<Ts...>) {
        return details::capture_as_tuple(get_or<Ss>(std::forward<T>(t), Ts{})...);
    }

    // Assignment of elements of type Us.
    template <class T, class S, class... Us>
    T& tt_assign(T& t, S&& s, type_sequence<Us...>) {
        ignore_args((get<Us>(t) = get<Us>(std::forward<S>(s)))...);
        return t;
    }

    // Helper class selecting types from tagged tuple.
    template <typename S, typename T, typename U>
    struct tag_to_type;

    // Base case (no types to select).
    template <typename... Ss, typename... Ts>
    struct tag_to_type<type_sequence<Ss...>, type_sequence<Ts...>, type_sequence<>> {
        using type = type_sequence<>;
    };

    // Inductive case (some types to select).
    template <typename... Ss, typename... Ts, typename U, typename... Us>
    struct tag_to_type<type_sequence<Ss...>, type_sequence<Ts...>, type_sequence<U, Us...>> {
        using type = typename tag_to_type<type_sequence<Ss...>, type_sequence<Ts...>, type_sequence<Us...>>::type::template push_front<typename type_sequence<Ts...,void>::template get<type_sequence<Ss...,void>::template find<U>>>;
    };

    //! @brief Struct holding separators between tags and values in tuples.
    template <typename T>
    struct separators;

    //! @brief Struct holding separators between tags and values in tuples in dictionary format.
    template <>
    struct separators<tags::dictionary_tuple> {
        //! @brief Separator between tuple tags and values.
        static constexpr char const* tag_val = ":";
        //! @brief Separator between tuple values and the following tags.
        static constexpr char const* val_tag = ", ";
    };

    //! @brief Struct holding separators between tags and values in tuples in assignment format.
    template <>
    struct separators<tags::assignment_tuple> {
        //! @brief Separator between tuple tags and values.
        static constexpr char const* tag_val = " = ";
        //! @brief Separator between tuple values and the following tags.
        static constexpr char const* val_tag = ", ";
    };

    //! @brief Struct holding separators between tags and values in tuples in underscore format.
    template <>
    struct separators<tags::underscore_tuple> {
        //! @brief Separator between tuple tags and values.
        static constexpr char const* tag_val = "-";
        //! @brief Separator between tuple values and the following tags.
        static constexpr char const* val_tag = "_";
    };

    //! @brief Struct holding separators between tags and values in tuples in arrowhead format.
    template <>
    struct separators<tags::arrowhead_tuple> {
        //! @brief Separator between tuple tags and values.
        static constexpr char const* tag_val = " => ";
        //! @brief Separator between tuple values and the following tags.
        static constexpr char const* val_tag = "; ";
    };

    //! @brief Escapes a boolean value for underscore tuple printing.
    inline std::string tt_val_print(bool x, tags::underscore_tuple) {
        return escape(x);
    }
    //! @brief Forwards other values for underscore tuple printing.
    template <typename T, typename = std::enable_if_t<not std::is_same<T,bool>::value>>
    inline T const& tt_val_print(T const& x, tags::underscore_tuple) {
        return x;
    }
    //! @brief Escapes a value for other tuple printing.
    template <typename T, typename S, typename = std::enable_if_t<not std::is_same<S,tags::underscore_tuple>::value>>
    inline auto tt_val_print(T const& x, S) {
        return escape(x);
    }

    //! @brief Prints no tags from a tagged tuple.
    template<typename S, typename T, typename O, typename F>
    void tt_print(tagged_tuple<S, T> const&, O&, F, type_sequence<>) {}

    //! @brief Prints one tag from a tagged tuple.
    template<typename S, typename T, typename O, typename F, typename S1>
    void tt_print(tagged_tuple<S, T> const& t, O& o, F, type_sequence<S1>) {
        o << strip_namespaces(type_name<S1>()) << separators<F>::tag_val << tt_val_print(get<S1>(t), F{});
    }

    //! @brief Prints multiple tags from a tagged tuple.
    template<typename S, typename T, typename O, typename F, typename S1, typename S2, typename... Ss>
    void tt_print(tagged_tuple<S, T> const& t, O& o, F f, type_sequence<S1,S2,Ss...>) {
        tt_print(t, o, f, type_sequence<S1>());
        o << separators<F>::val_tag;
        tt_print(t, o, f, type_sequence<S2,Ss...>());
    }
}
//! @endcond


/**
 * @brief Implementation of the `tagged_tuple` class.
 *
 * @param Ss The types to be used as tags.
 * @param Ts The corresponding value types.
*/
template<typename... Ss, typename... Ts>
struct tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>: public std::tuple<Ts...> {
    static_assert(type_repeated<Ss...>::size == 0, "repeated tags in tuple");

    //! @brief The type sequence of tags.
    using tags = type_sequence<Ss...>;

    //! @brief The type sequence of data types.
    using types = type_sequence<Ts...>;

    //! @brief Gets the types corresponding to multiple tags.
    template <typename... Us>
    using tag_types = typename details::tag_to_type<type_sequence<Ss...>, type_sequence<Ts...>, type_sequence<Us...>>::type;

    //! @brief Gets the type corresponding to a tag.
    template <typename S>
    using tag_type = typename tag_types<S>::front;

    //! @brief Type obtained by prepending a tagged element to the tuple.
    template <typename S, typename T>
    using push_front = tagged_tuple<type_sequence<S, Ss...>, type_sequence<T, Ts...>>;

    //! @brief Type obtained by appending a tagged element to the tuple.
    template <typename S, typename T>
    using push_back = tagged_tuple<type_sequence<Ss..., S>, type_sequence<Ts..., T>>;

    //! @brief Constructors inherited from `tuple`.
    using std::tuple<Ts...>::tuple;

    //! @brief Copy constructor from another `tagged_tuple`.
    template <class... OSs, class... OTs>
    tagged_tuple(tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>> const& t) :
        std::tuple<Ts...>(details::tt_capture(t, type_sequence<Ss...>{}, type_sequence<Ts...>{})) {}

    //! @brief Move constructor from another `tagged_tuple`.
    template <class... OSs, class... OTs>
    tagged_tuple(tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>&& t) :
        std::tuple<Ts...>(details::tt_capture(std::move(t), type_sequence<Ss...>{}, type_sequence<Ts...>{})) {}

    //! @brief Copy assignment from another `tagged_tuple`.
    template <class... OSs, class... OTs>
    tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>&
    operator=(tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>> const& t) {
        return details::tt_assign(*this, t, typename type_sequence<Ss...>::template intersect<OSs...>());
    }

    //! @brief Move assignment from another `tagged_tuple`.
    template <class... OSs, class... OTs>
    tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>&
    operator=(tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>&& t) {
        return details::tt_assign(*this, std::move(t), typename type_sequence<Ss...>::template intersect<OSs...>());
    }

    //! @brief Call operator returning a tagged tuple of call results.
    template <typename... Us>
    tagged_tuple<type_sequence<Ss...>, type_sequence<std::result_of_t<Ts(Us&&...)>...>> operator()(Us&&... vs) {
        return make_tagged_tuple<Ss...>(get<Ss>(*this)(std::forward<Us>(vs)...)...);
    }

    //! @brief Prints the content of the tagged tuple in a given format, skipping a given set of tags.
    template <typename O, typename F, typename... OSs>
    void print(O& o, F f, common::tags::skip_tags<OSs...>) const {
        details::tt_print(*this, o, f, typename tags::template subtract<OSs...>());
    }

    //! @brief Prints the content of the tagged tuple in a given format.
    template <typename O, typename F>
    void print(O& o, F f) const {
        print(o, f, common::tags::skip_tags<>());
    }

    //! @brief Prints the content of the tagged tuple in arrowhead format.
    template <typename O>
    void print(O& o) const {
        print(o, arrowhead_tuple);
    }
};


//! @cond INTERNAL
namespace details {
    // Unpacked arguments.
    template <typename... Ts>
    struct tagged_tuple_t {
        using type = tagged_tuple<common::type_slice<0, -1, 2, Ts...>, common::type_slice<1, -1, 2, Ts...>>;
    };

    // Single type sequence argument.
    template <typename... Ts>
    struct tagged_tuple_t<type_sequence<Ts...>> : public tagged_tuple_t<Ts...> {};
}
//! @endcond

//! @brief The `tagged_tuple_t` alias, allowing to express a `tagged_tuple` by interleaving tags and types (possibly wrapped in a type sequence).
template <typename... Ts>
using tagged_tuple_t = typename details::tagged_tuple_t<Ts...>::type;


//! @cond INTERNAL
namespace details {
    // General form.
    template <typename... Ts>
    struct tagged_tuple_cat;

    // No tuple to concatenate.
    template <>
    struct tagged_tuple_cat<> {
        using type = tagged_tuple<type_sequence<>, type_sequence<>>;
    };

    // The first tuple is empty.
    template <typename... Ts>
    struct tagged_tuple_cat<tagged_tuple<type_sequence<>, type_sequence<>>, Ts...> : tagged_tuple_cat<Ts...> {};

    // The first tuple is not empty.
    template <typename S, typename... Ss, typename T, typename... Ts, typename... Us>
    struct tagged_tuple_cat<tagged_tuple<type_sequence<S, Ss...>, type_sequence<T, Ts...>>, Us...> {
        using type = typename tagged_tuple_cat<tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>, Us...>::type::template push_front<S,T>;
    };
}
//! @endcond

//! @brief Concatenates multiple `tagged_tuple` types into a single `tagged_tuple` type.
template <typename... Ts>
using tagged_tuple_cat = typename details::tagged_tuple_cat<Ts...>::type;


}


}

#endif // FCPP_COMMON_TAGGED_TUPLE_H_
