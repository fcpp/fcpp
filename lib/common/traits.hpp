// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file traits.hpp
 * @brief Constants and typedefs for andvanced operations and querying on types.
 */

#ifndef FCPP_COMMON_TRAITS_H_
#define FCPP_COMMON_TRAITS_H_

#include <array>
#include <tuple>
#include <type_traits>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @name type_index
 *
 * Constant equal to the index of the first parameter among the subsequent ones.
 * It fails to compile if the first type parameter is not among the subsequent ones.
 * Indexes are computed starting with 0.
 */
//@{
//! @brief General recursive pattern.
template <typename A, typename B, typename... Ts>
constexpr size_t type_index = type_index<A, Ts...> + 1;

//! @brief Base case (the searched type is first).
template <typename A, typename... Ts>
constexpr size_t type_index<A, A, Ts...> = 0;
//@}


/**
 * @name type_contains
 *
 * Constant which is true if and only if the first type parameter is among the subsequent ones.
 */
//@{
//! @brief Base case (false if not found).
template <typename A, typename... Ts>
constexpr bool type_contains = false;

//! @brief General recursive pattern.
template <typename A, typename B, typename... Ts>
constexpr bool type_contains<A, B, Ts...> = type_contains<A, Ts...>;

//! @brief Base case (the searched type is first).
template <typename A, typename... Ts>
constexpr bool type_contains<A, A, Ts...> = true;
//@}


/**
 * @name type_repeated
 *
 * Constant which is true if and only if the type parameter list contains a repeated type.
 */
//@{
//! @brief Base case (for 1 or 0 types).
template <typename... Ts>
constexpr bool type_repeated = false;

/**
 * @brief Recursive pattern: either the first type occurs among the following, or the
 * following has a repetition.
 */
template <typename A, typename... Ts>
constexpr bool type_repeated<A, Ts...> = type_contains<A, Ts...> or type_repeated<Ts...>;
//@}


/**
 * @name type_sequence
 *
 * Helper empty class encapsulating a sequence of types.
 * Allows easy prepending, appending, counting and extracting types.
 */
//@{
//! @brief General form.
template <typename... Ts>
struct type_sequence;

//! @cond INTERNAL
namespace details {
    // Base case.
    template <typename, typename>
    struct type_intersect {
        using type = type_sequence<>;
    };

    // Recursive form.
    template <typename T, typename... Ts, typename... Ss>
    struct type_intersect<type_sequence<T, Ts...>, type_sequence<Ss...>> {
        using type = std::conditional_t<
            !type_contains<T, Ss...>,
            typename type_intersect<type_sequence<Ts...>, type_sequence<Ss...>>::type,
            typename type_intersect<type_sequence<Ts...>, type_sequence<Ss...>>::type::template prepend<T>
        >;
    };
}
//! @endcond

//! @brief Non-empty form, allows for extracting index, head and tail.
template <typename T, typename... Ts>
struct type_sequence<T, Ts...> {
    template <typename... Ss>
    using prepend = type_sequence<Ss..., T, Ts...>;
    
    template <typename... Ss>
    using append = type_sequence<T, Ts..., Ss...>;
    
    template<typename... Ss>
    using intersect = typename details::type_intersect<type_sequence<T, Ts...>, type_sequence<Ss...>>::type;
    
    using head = T;
    
    using tail = type_sequence<Ts...>;
    
    template <typename S>
    static constexpr size_t index() {
        return type_index<S, T, Ts...>;
    }
    
    static constexpr size_t size() {
        return 1 + sizeof...(Ts);
    }
};

//! @brief Empty form, cannot extract index, head and tail.
template <>
struct type_sequence<> {
    template <typename... Ss>
    using prepend = type_sequence<Ss...>;
    
    template <typename... Ss>
    using append = type_sequence<Ss...>;
    
    template<typename... Ss>
    using intersect = type_sequence<>;
    
    static constexpr size_t size() {
        return 0;
    }
};
//@}

//! @cond INTERNAL
namespace details {
    // General form.
    template<int n, int m, typename... Ts>
    struct type_subseq;

    // Subsequence element not found.
    template<int n, int m, typename T, typename... Ts>
    struct type_subseq<n, m, T, Ts...> : type_subseq<n, m - 1, Ts...> {};

    // Subsequence element found.
    template<int n, typename T, typename... Ts>
    struct type_subseq<n, 0, T, Ts...> {
        typedef typename type_subseq<n, n - 1, Ts...>::type::template prepend<T> type;
    };

    // Base case (empty sequence)
    template<int n, int m>
    struct type_subseq<n, m> {
        typedef type_sequence<> type;
    };
}
//! @endcond

/**
 * @name type_subseq
 *
 * Extracts a subsequence from a parameter pack.
 * @param n distance between elements extracted
 * @param m first element extracted
 */
template<int n, int m, typename... Ts>
using type_subseq = typename details::type_subseq<n, m, Ts...>::type;

/**
 * @name nth_type
 *
 * Extracts the n-th type from a parameter pack.
 * Special case of `type_subseq`.
 */
template <int n, typename... Ts>
using nth_type = typename type_subseq<sizeof...(Ts), n, Ts...>::head;


/**
 * @name boolean operators
 *
 * Constexpr computing boolean combinations of their arguments.
 */
//@{
//! @brief Helper class holding arbitrary boolean template parameters.
template <bool...> struct bool_pack;

//! @brief Checks if every argument is `true`.
template <bool... v>
constexpr bool all_true = std::is_same<bool_pack<true, v...>, bool_pack<v..., true>>::value;

//! @brief Checks if every argument is `false`.
template <bool... v>
constexpr bool all_false = std::is_same<bool_pack<false, v...>, bool_pack<v..., false>>::value;

//! @brief Checks if some argument is `true`.
template <bool... v>
constexpr bool some_true = not all_false<v...>;

//! @brief Checks if some argument is `false`.
template <bool... v>
constexpr bool some_false = not all_true<v...>;
//@}


/**
 * @name has_template
 *
 * Constant which is true if and only if the second (type) parameter is built through arrays and tuples
 * from specializations of the first (template) parameter.
 */
//@{
//! @brief False in general.
template <template<class...> class T, class A>
constexpr bool has_template = false;

//! @brief Ignore constness.
template <template<class...> class T, class A>
constexpr bool has_template<T, const A> = has_template<T, A>;

//! @brief Ignore lvalue references.
template <template<class...> class T, class A>
constexpr bool has_template<T, A&> = has_template<T, A>;

//! @brief Ignore rvalue references.
template <template<class...> class T, class A>
constexpr bool has_template<T, A&&> = has_template<T, A>;

//! @brief True if second parameter is of the form T<A>.
template <template<class...> class T, class... A>
constexpr bool has_template<T, T<A...>> = true;

//! @brief Recurse on std::array value type arguments.
template <template<class...> class T, class A, size_t N>
constexpr bool has_template<T, std::array<A, N>> = has_template<T, A>;

//! @brief Recurse on std::tuple value type arguments.
template <template<class...> class T, class... A>
constexpr bool has_template<T, std::tuple<A...>> = some_true<has_template<T, A>...>;
//@}


//! @cond INTERNAL
namespace details {
    // If no occurrences of the template are present.
    template <template<class> class T, class A>
    struct del_template {
        typedef A type;
    };
    
    // Propagate constness.
    template <template<class> class T, class A>
    struct del_template<T, const A> {
        typedef const typename del_template<T, A>::type type;
    };
    
    // Propagate lvalue references.
    template <template<class> class T, class A>
    struct del_template<T, A&> {
        typedef typename del_template<T, A>::type& type;
    };
    
    // Propagate rvalue references.
    template <template<class> class T, class A>
    struct del_template<T, A&&> {
        typedef typename del_template<T, A>::type&& type;
    };
    
    // If the second parameter is of the form T<A>.
    template <template<class> class T, class A>
    struct del_template<T, T<A>> {
        typedef A type;
    };

    // Removes occurrences from the argument of an std::array value type.
    template <template<class> class T, class A, size_t N>
    struct del_template<T, std::array<A, N>> {
        typedef std::array<typename del_template<T, A>::type, N> type;
    };

    // Removes occurrences from the arguments of an std::tuple value types.
    template <template<class> class T, class... A>
    struct del_template<T, std::tuple<A...>> {
        typedef std::tuple<typename del_template<T, A>::type...> type;
    };
}
//! @endcond


/**
 * @brief Deletes occurrences of the first (template) parameter within the second (type) parameter, which is built through arrays and tuples.
 */
template <template<class> class T, class A>
using del_template = typename details::del_template<T, A>::type;

    
//! @brief Converts the second (type) parameter into a specialization of the first (template) parameter.
template <template<class> class T, class A>
using add_template = T<del_template<T, A>>;


/**
 * @name nested_template
 *
 * Constant which is true if and only if the second (type) parameter is built by using the first (template) parameter, possibly nested within other templates.
 */
//@{
//! @brief False if the second argument is not a template.
template <template<class> class T, class A>
constexpr bool nested_template = false;

//! @brief True if second argument is T<A> or T is found in A.
template <template<class> class T, template<class> class S, class A>
constexpr bool nested_template<T, S<A>> = std::is_same<T<A>, S<A>>::value or nested_template<T, A>;
//@}


namespace details {
//! @cond INTERNAL
    // General form.
    template <class A, template<class> class... Ts>
    struct nest_template;

    // Nesting no templates.
    template <class A>
    struct nest_template<A> {
        typedef A type;
    };
    
    // Nesting a single template.
    template <class A, template<class> class T>
    struct nest_template<A, T> {
        typedef std::conditional_t<fcpp::nested_template<T, A>, A, T<A>> type;
    };
    
    // Nesting more templates.
    template <class A, template<class> class T, template<class> class... Ts>
    struct nest_template<A, T, Ts...> {
        typedef typename nest_template<typename nest_template<A, Ts...>::type, T>::type type;
    };
//! @endcond
}

/**
 * @name nest_templates
 *
 * Nests (one or more) template to a type only if the template is not already present in the type.
 */
template <class A, template<class> class... Ts>
using nest_template = typename details::nest_template<A, Ts...>::type;

}

#endif  // FCPP_COMMON_TRAITS_H_
