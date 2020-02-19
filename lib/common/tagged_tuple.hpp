// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

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
//@{
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
const type_get<type_find<S, Ss...>, Ts...>&
get(const tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>& t) {
    return std::get<type_find<S, Ss...>>(t);
}
//@}


//! @brief Utility function for creating tagged tuples.
template <typename... Ss, typename... Ts>
tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>> make_tagged_tuple(Ts... vs) {
    return {vs...};
}


//! @cond INTERNAL
namespace details {
    // Helper function ignoring its arguments.
    template <class... Ts>
    void ignore(const Ts&...) {}
    
    // Copy assignment of elements of type Us.
    template <class T, class S, class... Us>
    T& tt_assign(T& t, const S& s, type_sequence<Us...>) {
        ignore((get<Us>(t) = get<Us>(s))...);
        return t;
    }
    
    // Move assignment of elements of type Us.
    template <class T, class S, class... Us>
    T& tt_assign(T& t, S&& s, type_sequence<Us...>) {
        ignore((get<Us>(t) = std::move(get<Us>(s)))...);
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
    static_assert(fcpp::type_repeated<Ss...>::size == 0, "repeated tags in tuple");
    
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
    tagged_tuple(const tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>& t) {
        details::tt_assign(*this, t, typename type_sequence<Ss...>::template intersect<OSs...>());
    }
    
    //! @brief Move constructor from another `tagged_tuple`.
    template <class... OSs, class... OTs>
    tagged_tuple(tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>&& t) {
        details::tt_assign(*this, std::move(t), typename type_sequence<Ss...>::template intersect<OSs...>());
    }
    
    //! @brief Copy assignment from another `tagged_tuple`.
    template <class... OSs, class... OTs>
    tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>&
    operator=(const tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>& t) {
        return details::tt_assign(*this, t, typename type_sequence<Ss...>::template intersect<OSs...>());
    }
    
    //! @brief Move assignment from another `tagged_tuple`.
    template <class... OSs, class... OTs>
    tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>&
    operator=(tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>&& t) {
        return details::tt_assign(*this, std::move(t), typename type_sequence<Ss...>::template intersect<OSs...>());
    }
    
    //! @brief Call operator returning a tagged tuple of call results.
    template <typename T>
    tagged_tuple<type_sequence<Ss...>, type_sequence<std::result_of_t<Ts(T)>...>> operator()(T v) {
        return make_tagged_tuple<Ss...>(get<Ss>(*this)(v)...);
    }
};


//! @cond INTERNAL
namespace details {
    std::string strip_tags(std::string s) {
        if (strncmp(s.c_str(), "tags::", 6) == 0) return s.substr(6);
        return s;
    }
    template<typename S, typename T>
    void tt_print(std::ostream&, const tagged_tuple<S, T>&, type_sequence<>, type_sequence<>) {
    }
    template<typename S, typename T, typename S1, typename T1>
    void tt_print(std::ostream& o, const tagged_tuple<S, T>& t, type_sequence<S1>, type_sequence<T1>) {
        o << strip_tags(type_name<S1>()) << " = ";
        if (std::is_same<T1,bool>::value) o << (get<S1>(t) ? "true" : "false");
        else o << get<S1>(t);
    }
    template<typename S, typename T, typename S1, typename... Ss, typename T1, typename... Ts>
    void tt_print(std::ostream& o, const tagged_tuple<S, T>& t, type_sequence<S1,Ss...>, type_sequence<T1,Ts...>) {
        tt_print(o, t, type_sequence<S1>(), type_sequence<T1>());
        o << ", ";
        tt_print(o, t, type_sequence<Ss...>(), type_sequence<Ts...>());
    }
}
//! @endcond

//! @brief Prints a tagged tuple in assignment-list format.
template<typename S, typename T>
std::ostream& operator<<(std::ostream& o, const tagged_tuple<S, T>& t) {
    details::tt_print(o, t, S(), T());
    return o;
}


//! @brief The `tagged_tuple_t` alias, allowing to express a `tagged_tuple` by interleaving tags and types.
template <typename... Ts>
using tagged_tuple_t = tagged_tuple<type_slice<0, -1, 2, Ts...>, type_slice<1, -1, 2, Ts...>>;


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


//! @cond INTERNAL
namespace details {
    template <typename... Ss, typename... Ts, typename T, typename... Us>
    const T& get_or(const tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>& t, const T& def, type_sequence<Us...>) {
        const T* r = &def;
        ignore((r = &get<Us>(t))...);
        return *r;
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
template <typename S, typename... Ss, typename... Ts, typename T>
const T& get_or(const tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>& t, const T& def) {
    return details::get_or(t, def, typename type_sequence<Ss...>::template intersect<S>());
}


}

#endif // FCPP_COMMON_TAGGED_TUPLE_H_
