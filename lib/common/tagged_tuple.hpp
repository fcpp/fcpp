// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file tagged_tuple.hpp
 * @brief Implementation of the `tagged_tuple<Ts...>` class template for handling addressable tuple data.
 */

#ifndef FCPP_COMMON_TAGGED_TUPLE_H_
#define FCPP_COMMON_TAGGED_TUPLE_H_

#include <tuple>
#include <type_traits>

#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {
    

//! @cond INTERNAL
namespace details {
    //! @brief General form.
    template<typename S, typename T>
    struct tagged_tuple;
}
//! @endcond


/**
 * @name get
 *
 * Function getting elements of a tagged tuple by tag.
 * @param S the tag to be extracted
 */
//@{
//! @brief Write access.
template <typename S, typename... Ss, typename... Ts>
nth_type<type_index<S, Ss...>, Ts...>& get(details::tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>& t) {
    return std::get<type_index<S, Ss...>>(t);
}
//! @brief Move access.
template <typename S, typename... Ss, typename... Ts>
nth_type<type_index<S, Ss...>, Ts...>&& get(details::tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>&& t) {
    return std::get<type_index<S, Ss...>>(std::move(t));
}
//! @brief Const access.
template <typename S, typename... Ss, typename... Ts>
const nth_type<type_index<S, Ss...>, Ts...>& get(const details::tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>& t) {
    return std::get<type_index<S, Ss...>>(t);
}
//@}


//! @brief Utility function for creating tagged tuples.
template <typename... Ss, typename... Ts>
details::tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>> make_tagged_tuple(Ts... vs) {
    return {vs...};
}


namespace details {
//! @cond INTERNAL
    // Helper function ignoring its arguments.
    template <class... Ts>
    void ignore(const Ts&... vs) {}
    
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
//! @endcond

    //! @brief Implementation of `tagged_tuple`.
    template<typename... Ss, typename... Ts>
    struct tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>: public std::tuple<Ts...> {
        static_assert(not type_repeated<Ss...>, "repeated tags in tuple");
        
        //! @brief Type obtained by prepending a tagged element to the tuple.
        template <typename S, typename T>
        using prepend = tagged_tuple<type_sequence<S, Ss...>, type_sequence<T, Ts...>>;

        //! @brief Constructors inherited from `tuple`.
        using std::tuple<Ts...>::tuple;
        
        //! @brief Copy constructor from another `tagged_tuple`.
        template <class... OSs, class... OTs>
        tagged_tuple(const tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>& t) {
            tt_assign(*this, t, typename type_sequence<Ss...>::template intersect<OSs...>());
        }
        
        //! @brief Move constructor from another `tagged_tuple`.
        template <class... OSs, class... OTs>
        tagged_tuple(tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>&& t) {
            tt_assign(*this, std::move(t), typename type_sequence<Ss...>::template intersect<OSs...>());
        }
        
        //! @brief Copy assignment from another `tagged_tuple`.
        template <class... OSs, class... OTs>
        tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>&
        operator=(const tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>& t) {
            return tt_assign(*this, t, typename type_sequence<Ss...>::template intersect<OSs...>());
        }
        
        //! @brief Move assignment from another `tagged_tuple`.
        template <class... OSs, class... OTs>
        tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>&
        operator=(tagged_tuple<type_sequence<OSs...>, type_sequence<OTs...>>&& t) {
            return tt_assign(*this, std::move(t), typename type_sequence<Ss...>::template intersect<OSs...>());
        }
        
        //! @brief Call operator returning a tagged tuple of call results.
        template <typename T>
        tagged_tuple<type_sequence<Ss...>, type_sequence<std::result_of_t<Ts(T)>...>> operator()(T v) {
            return make_tagged_tuple<Ss...>(get<Ss>(*this)(v)...);
        }
    };
}


//! @brief The `tagged_tuple` class, allowing to express a `details::tagged_tuple` by interleaving tags and types.
template <typename... Ts>
using tagged_tuple = details::tagged_tuple<type_subseq<2, 0, Ts...>, type_subseq<2, 1, Ts...>>;


//! @cond INTERNAL
namespace details {
    // General form.
    template <typename... Ts>
    struct tagged_tuple_cat;
    
    // No tuple to concatenate.
    template <>
    struct tagged_tuple_cat<> {
        using type = details::tagged_tuple<type_sequence<>, type_sequence<>>;
    };
    
    // The first tuple is empty.
    template <typename... Ts>
    struct tagged_tuple_cat<details::tagged_tuple<type_sequence<>, type_sequence<>>, Ts...> : tagged_tuple_cat<Ts...> {};
    
    // The first tuple is not empty.
    template <typename S, typename... Ss, typename T, typename... Ts, typename... Us>
    struct tagged_tuple_cat<details::tagged_tuple<type_sequence<S, Ss...>, type_sequence<T, Ts...>>, Us...> {
        using type = typename tagged_tuple_cat<details::tagged_tuple<type_sequence<Ss...>, type_sequence<Ts...>>, Us...>::type::template prepend<S,T>;
    };
}
//! @endcond

//! @brief Concatenates multiple `tagged_tuple` types into a single `tagged_tuple` type.
template <typename... Ts>
using tagged_tuple_cat = typename details::tagged_tuple_cat<Ts...>::type;


}

#endif // FCPP_COMMON_TAGGED_TUPLE_H_
