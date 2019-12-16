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
 * @name type_sequence
 *
 * Helper empty class encapsulating a sequence of types.
 * Mimics operations in standard stl containers.
 */
//{@
//! @brief General form.
template <typename... Ts>
struct type_sequence;


// Base case (0 if not found).
template <typename A, typename... Ts>
constexpr size_t type_count = 0;

// General recursive pattern.
template <typename A, typename B, typename... Ts>
constexpr size_t type_count<A, B, Ts...> = type_count<A, Ts...> + (std::is_same<A,B>::value ? 1 : 0);

// General recursive pattern.
template <typename A, typename B, typename... Ts>
constexpr size_t type_find = type_find<A, Ts...> + 1;

// Base case (the searched type is first).
template <typename A, typename... Ts>
constexpr size_t type_find<A, A, Ts...> = 0;


//! @cond INTERNAL
namespace details {
    // General form.
    template<int start, int end, int stride, typename... Ts>
    struct type_slice {
        using type = type_sequence<>;
    };
    
    // Recursive form.
    template<int start, int end, int stride, typename T, typename... Ts>
    struct type_slice<start, end, stride, T, Ts...> {
        using type = std::conditional_t<end == 0, type_sequence<>, std::conditional_t<
            start == 0,
            typename type_slice<stride - 1, end - 1, stride, Ts...>::type::template push_front<T>,
            typename type_slice<start  - 1, end - 1, stride, Ts...>::type
        >>;
    };

    // Base case.
    template <typename, typename>
    struct type_intersect {
        using type = type_sequence<>;
    };

    // Recursive form.
    template <typename T, typename... Ts, typename... Ss>
    struct type_intersect<type_sequence<T, Ts...>, type_sequence<Ss...>> {
        using type = std::conditional_t<
            type_count<T, Ss...> != 0,
            typename type_intersect<type_sequence<Ts...>, type_sequence<Ss...>>::type::template push_front<T>,
            typename type_intersect<type_sequence<Ts...>, type_sequence<Ss...>>::type
        >;
    };
    
    // General form.
    template <typename, typename>
    struct type_unite;

    // Recursive form.
    template <typename... Ts, typename S, typename... Ss>
    struct type_unite<type_sequence<Ts...>, type_sequence<S, Ss...>> {
        using type = std::conditional_t<
            type_count<S, Ts...> != 0,
            typename type_unite<type_sequence<Ts...>, type_sequence<Ss...>>::type,
            typename type_unite<type_sequence<Ts..., S>, type_sequence<Ss...>>::type
        >;
    };

    // Base case.
    template <typename... Ts>
    struct type_unite<type_sequence<Ts...>, type_sequence<>> {
        using type = type_sequence<Ts...>;
    };
    
    // General form.
    template <typename...>
    struct type_repeated {
        using type = type_sequence<>;
    };
    
    // Recursive form.
    template <typename T, typename... Ts>
    struct type_repeated<T, Ts...> {
        using type = std::conditional_t<
            type_count<T, Ts...> == 0,
            typename type_repeated<Ts...>::type,
            typename type_repeated<Ts...>::type::template push_front<T>
        >;
    };
    
    // General form.
    template <typename...>
    struct type_uniq {
        using type = type_sequence<>;
    };
    
    // Recursive form.
    template <typename T, typename... Ts>
    struct type_uniq<T, Ts...> {
        using type = std::conditional_t<
            type_count<T, Ts...> == 0,
            typename type_uniq<Ts...>::type::template push_front<T>,
            typename type_uniq<Ts...>::type
        >;
    };
}
//! @endcond

    
/**
 * @brief Extracts a subsequence from the type sequence.
 * @param start  first element extracted
 * @param end    no element extracted after end (defaults to -1 = end of the sequence)
 * @param stride interval between element extracted (defaults to 1)
 */
template <int start, int end, int stride, typename... Ts>
using type_slice = typename details::type_slice<start, end, stride, Ts...>::type;

//! @brief Extracts the n-th type from the sequence.
template <int n, typename... Ts>
using type_get = typename type_slice<n, n+1, 1, Ts...>::front;

//! @brief Extract the types that are repeated more than once.
template <typename... Ts>
using type_repeated = typename details::type_repeated<Ts...>::type;

//! @brief Extract the subsequence in which each type appears once (opposite of repeated).
template <typename... Ts>
using type_uniq = typename details::type_uniq<Ts...>::type;


//! @brief Non-empty form, allows for extracting elements and subsequences.
template <typename T, typename... Ts>
struct type_sequence<T, Ts...> {
    //! @brief Extracts the n-th type from the sequence.
    template <int n>
    using get = type_get<n, T, Ts...>;

    /**
     * @brief Extracts a subsequence from the type sequence.
     * @param start  first element extracted
     * @param end    no element extracted after end (defaults to -1 = end of the sequence)
     * @param stride interval between element extracted (defaults to 1)
     */
    template <int start, int end = -1, int stride = 1>
    using slice = type_slice<start, end, stride, T, Ts...>;
    
    //! @brief The first type of the sequence.
    using front = T;
    
    //! @brief The last type of the sequence.
    using back = get<sizeof...(Ts)>;
    
    //! @brief Removes the first type of the sequence.
    using pop_front = type_sequence<Ts...>;
    
    //! @brief Removes the last type of the sequence.
    using pop_back = slice<0, sizeof...(Ts)>;
    
    //! @brief Adds types at the front of the sequence.
    template <typename... Ss>
    using push_front = type_sequence<Ss..., T, Ts...>;
    
    //! @brief Adds types at the back of the sequence.
    template <typename... Ss>
    using push_back = type_sequence<T, Ts..., Ss...>;
    
    //! @brief Set intersection with other sequence.
    template<typename... Ss>
    using intersect = typename details::type_intersect<type_sequence<T, Ts...>, type_sequence<Ss...>>::type;
    
    //! @brief Set union with other sequence.
    template<typename... Ss>
    using unite = typename details::type_unite<type_sequence<T, Ts...>, type_sequence<Ss...>>::type;
    
    //! @brief Extract the types that are repeated more than once.
    using repeated = type_repeated<T, Ts...>;

    //! @brief Extract the subsequence in which each type appears once (opposite of repeated).
    using uniq = type_uniq<T, Ts...>;
    
    //! @brief Constant equal to the index of `S` among the sequence. Fails to compile if not present, indices start from 0.
    template <typename S>
    static constexpr size_t find = type_find<S, T, Ts...>;

    //! @brief Constant which is true if and only if the type parameter is in the sequence.
    template <typename S>
    static constexpr size_t count = type_count<S, T, Ts...>;
    
    //! @brief The length of the sequence.
    static constexpr size_t size = 1 + sizeof...(Ts);
};

//! @brief Empty form, cannot extract elements and subsequences.
template <>
struct type_sequence<> {
    template <int start, int end, int stride = 1>
    using slice = type_sequence<>;
    
    template <typename... Ss>
    using push_front = type_sequence<Ss...>;
    
    template <typename... Ss>
    using push_back = type_sequence<Ss...>;
    
    template<typename... Ss>
    using intersect = type_sequence<>;
    
    template<typename... Ss>
    using unite = type_sequence<Ss...>;
    
    using repeated = type_sequence<>;
    
    using uniq = type_sequence<>;

    template <typename S>
    static constexpr size_t count = 0;
    
    static constexpr size_t size = 0;
};
//@}


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
