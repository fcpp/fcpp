// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file traits.hpp
 * @brief Constants and typedefs for andvanced operations and querying on types.
 */

#ifndef FCPP_COMMON_TRAITS_H_
#define FCPP_COMMON_TRAITS_H_

#ifdef __has_include
    #if __has_include(<cxxabi.h>)
    #include <cxxabi.h>
    #else
    #define NABI
    #endif
#else
    #ifndef NABI
    #include <cxxabi.h>
    #endif
#endif

#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


/**
 * @name type_sequence
 *
 * Helper empty class encapsulating a sequence of types.
 * Mimics operations in standard stl containers.
 */
//! @{
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
    template <typename, typename>
    struct type_subtract;

    // Recursive form.
    template <typename T, typename... Ts, typename... Ss>
    struct type_subtract<type_sequence<T, Ts...>, type_sequence<Ss...>> {
        using type = std::conditional_t<
            type_count<T, Ss...> != 0,
            typename type_subtract<type_sequence<Ts...>, type_sequence<Ss...>>::type,
            typename type_subtract<type_sequence<Ts...>, type_sequence<Ss...>>::type::template push_front<T>
        >;
    };

    // Base case.
    template <typename... Ss>
    struct type_subtract<type_sequence<>, type_sequence<Ss...>> {
        using type = type_sequence<>;
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

    // General form.
    template <typename... Ts>
    struct type_cat;

    // Empty base case.
    template <>
    struct type_cat<> {
        using type = type_sequence<>;
    };

    // Base case.
    template <typename... Ts, typename... Ss>
    struct type_cat<type_sequence<Ts...>, type_sequence<Ss...>> {
        using type = type_sequence<Ts..., Ss...>;
    };

    // Recursive form.
    template <typename T, typename... Ss>
    struct type_cat<T, Ss...> {
        using type = typename type_cat<T, typename type_cat<Ss...>::type>::type;
    };

    // General form.
    template <typename... Ts>
    struct type_product;

    // Empty base case.
    template <>
    struct type_product<> {
        using type = type_sequence<type_sequence<>>;
    };

    // Base case with single option.
    template <typename... Ts, typename S>
    struct type_product<type_sequence<Ts...>, type_sequence<S>> {
        using type = type_sequence<typename type_cat<Ts,S>::type...>;
    };

    // Base case.
    template <typename T, typename... Ss>
    struct type_product<T, type_sequence<Ss...>> {
        using type = typename type_cat<typename type_product<T,type_sequence<Ss>>::type...>::type;
    };

    // Recursive form.
    template <typename T, typename... Ss>
    struct type_product<T, Ss...> {
        using type = typename type_product<T, typename type_product<Ss...>::type>::type;
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

//! @brief The type sequence intersection of two type sequences.
template <typename T, typename S>
using type_intersect = typename details::type_intersect<T,S>::type;

//! @brief The type sequence union of two type sequences.
template <typename T, typename S>
using type_unite = typename details::type_unite<T,S>::type;

//! @brief The sequence of types that are in the first type sequence but not in the second.
template <typename T, typename S>
using type_subtract = typename details::type_subtract<T,S>::type;

//! @brief Concatenates a sequence of type sequences.
template <typename... Ts>
using type_cat = typename details::type_cat<Ts...>::type;

//! @brief Computes the cartesian product of a sequence of type sequences of type sequences.
template <typename... Ts>
using type_product = typename details::type_product<Ts...>::type;


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
    using intersect = type_intersect<type_sequence<T, Ts...>, type_sequence<Ss...>>;

    //! @brief Set union with other sequence.
    template<typename... Ss>
    using unite = type_unite<type_sequence<T, Ts...>, type_sequence<Ss...>>;

    //! @brief Set difference with other sequence.
    template<typename... Ss>
    using subtract = type_subtract<type_sequence<T, Ts...>, type_sequence<Ss...>>;

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
    template <int start, int end = -1, int stride = 1>
    using slice = type_sequence<>;

    template <typename... Ss>
    using push_front = type_sequence<Ss...>;

    template <typename... Ss>
    using push_back = type_sequence<Ss...>;

    template<typename... Ss>
    using intersect = type_sequence<>;

    template<typename... Ss>
    using unite = type_sequence<Ss...>;

    template<typename... Ss>
    using subtract = type_sequence<>;

    using repeated = type_sequence<>;

    using uniq = type_sequence<>;

    template <typename S>
    static constexpr size_t count = 0;

    static constexpr size_t size = 0;
};
//! @}


//! @cond INTERNAL
namespace details {
    // General form.
    template <typename... Ts>
    struct export_list {
        using type = type_sequence<>;
    };
    // Type argument.
    template <typename T, typename... Ts>
    struct export_list<T,Ts...> : public type_unite<type_sequence<T>, typename export_list<Ts...>::type> {};
    // Type sequence argument.
    template <typename... Ts, typename... Ss>
    struct export_list<type_sequence<Ts...>,Ss...> : public export_list<Ts...,Ss...> {};
}
//! @endcond

//! @brief Merges export lists and types into a single type sequence.
template <typename... Ts>
using export_list = typename details::export_list<Ts...>::type;


/**
 * @name boolean operators
 *
 * Constexpr computing boolean combinations of their arguments.
 */
//! @{
//! @brief Helper class holding arbitrary boolean template parameters.
template <bool...> struct bool_pack {};

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
//! @}


/**
 * @name is_sized_template
 *
 * Constant which is true if and only if the second (type) parameter is an instantiation of the first (template) parameter with class arguments.
 */
//! @{
//! @brief False in general.
template <template<class,size_t> class T, class A>
constexpr bool is_sized_template = false;

//! @brief Ignore constness.
template <template<class,size_t> class T, class A>
constexpr bool is_sized_template<T, A const> = is_sized_template<T, A>;

//! @brief Ignore lvalue references.
template <template<class,size_t> class T, class A>
constexpr bool is_sized_template<T, A&> = is_sized_template<T, A>;

//! @brief Ignore rvalue references.
template <template<class,size_t> class T, class A>
constexpr bool is_sized_template<T, A&&> = is_sized_template<T, A>;

//! @brief True if second parameter is of the form T<A,N>.
template <template<class,size_t> class T, class A, size_t N>
constexpr bool is_sized_template<T, T<A, N>> = true;
//! @}

//! @brief Enables if type is instance of a sized template.
template <template<class,size_t> class T, class A, class B = void>
using if_sized_template = std::enable_if_t<is_sized_template<T, A>, B>;

//! @brief Enables if type is instance of a sized template.
template <template<class,size_t> class T, class A, class B = void>
using ifn_sized_template = std::enable_if_t<not is_sized_template<T, A>, B>;


/**
 * @name is_class_template
 *
 * Constant which is true if and only if the second (type) parameter is an instantiation of the first (template) parameter with class arguments.
 */
//! @{
//! @brief False in general.
template <template<class...> class T, class A>
constexpr bool is_class_template = false;

//! @brief Ignore constness.
template <template<class...> class T, class A>
constexpr bool is_class_template<T, A const> = is_class_template<T, A>;

//! @brief Ignore lvalue references.
template <template<class...> class T, class A>
constexpr bool is_class_template<T, A&> = is_class_template<T, A>;

//! @brief Ignore rvalue references.
template <template<class...> class T, class A>
constexpr bool is_class_template<T, A&&> = is_class_template<T, A>;

//! @brief True if second parameter is of the form T<A>.
template <template<class...> class T, class... A>
constexpr bool is_class_template<T, T<A...>> = true;
//! @}

//! @brief Enables if type is instance of a class template.
template <template<class...> class T, class A, class B = void>
using if_class_template = std::enable_if_t<is_class_template<T, A>, B>;

//! @brief Enables if type is not instance of a class template.
template <template<class...> class T, class A, class B = void>
using ifn_class_template = std::enable_if_t<not is_class_template<T, A>, B>;


/**
 * @name has_template
 *
 * Constant which is true if and only if the second (type) parameter is built through array-like
 * and tuple-like classes from specializations of the first (template) parameter.
 */
//! @{
//! @brief False in general.
template <template<class...> class T, class A>
constexpr bool has_template = false;

//! @brief Ignore constness.
template <template<class...> class T, class A>
constexpr bool has_template<T, A const> = has_template<T, A>;

//! @brief Ignore lvalue references.
template <template<class...> class T, class A>
constexpr bool has_template<T, A&> = has_template<T, A>;

//! @brief Ignore rvalue references.
template <template<class...> class T, class A>
constexpr bool has_template<T, A&&> = has_template<T, A>;

//! @brief True if second parameter is of the form T<A>.
template <template<class...> class T, class... A>
constexpr bool has_template<T, T<A...>> = true;

//! @brief Recurse on tuple-like type arguments.
template <template<class...> class T, template<class...> class U, class... A>
constexpr bool has_template<T, U<A...>> = some_true<has_template<T, A>...>;

//! @brief Recurse on array-like type arguments.
template <template<class...> class T, template<class,size_t> class U, class A, size_t N>
constexpr bool has_template<T, U<A, N>> = has_template<T, A>;


//! @cond INTERNAL
namespace details {
    //! @brief Value type case ignoring constness.
    template <typename T>
    struct partial_decay {
        using type = std::remove_const_t<T>;
    };

    //! @brief Preserves lvalue references with constness.
    template <typename T>
    struct partial_decay<T&> {
        using type = T&;
    };

    //! @brief Ignores rvalue references and constness.
    template <typename T>
    struct partial_decay<T&&> {
        using type = std::remove_const_t<T>;
    };
    //! @}
}

//! @brief The type that should be returned by a function forwarding an argument of type T.
template <typename T>
using partial_decay = typename details::partial_decay<T>::type;


//! @cond INTERNAL
namespace details {
    //! @brief Type referencing vector values (value type case).
    template <typename T>
    struct vectorize {
        using type = typename std::vector<T>::value_type;
    };

    //! @brief Type referencing vector values (reference case).
    template <typename T>
    struct vectorize<T&> {
        using type = typename std::vector<T>::reference;
    };

    //! @brief Type referencing vector values (const reference case).
    template <typename T>
    struct vectorize<T const&> {
        using type = typename std::vector<T>::const_reference;
    };

    //! @brief General form.
    template <template<class> class T, class A, bool b = has_template<T, A>>
    struct extract_template;

    //! @brief Base case assuming no occurrences of the template.
    template <template<class> class T, class A>
    struct extract_template<T, A, false> {
        using type = A;
    };

    //! @brief Propagate & assuming no occurrences of the template.
    template <template<class> class T, class A>
    struct extract_template<T, A&, false> {
        using type = A const&;
    };

    //! @brief Propagate & assuming occurrences of the template are present.
    template <template<class> class T, template<class...> class U, class... A>
    struct extract_template<T, U<A...>&, true> {
        using type = typename extract_template<T, U<A&...>, true>::type;
    };

    //! @brief Propagate const& assuming occurrences of the template are present.
    template <template<class> class T, template<class...> class U, class... A>
    struct extract_template<T, U<A...> const&, true> {
        using type = typename extract_template<T, U<A const&...>, true>::type;
    };

    //! @brief Propagate & assuming occurrences of the template are present.
    template <template<class> class T, template<class,size_t> class U, class A, size_t N>
    struct extract_template<T, U<A, N>&, true> {
        using type = typename extract_template<T, U<A&, N>, true>::type;
    };

    //! @brief Propagate const& assuming occurrences of the template are present.
    template <template<class> class T, template<class,size_t> class U, class A, size_t N>
    struct extract_template<T, U<A, N> const&, true> {
        using type = typename extract_template<T, U<A const&, N>, true>::type;
    };

    //! @brief Extracts occurrences from the arguments of a tuple-like type.
    template <template<class> class T, template<class...> class U, class... A>
    struct extract_template<T, U<A...>, true> {
        using type = U<typename extract_template<T, common::partial_decay<A>>::type...>;
    };

    //! @brief Extracts occurrences from the argument of an array-like type.
    template <template<class> class T, template<class,size_t> class U, class A, size_t N>
    struct extract_template<T, U<A, N>, true> {
        using type = U<typename extract_template<T, common::partial_decay<A>>::type, N>;
    };

    //! @brief If the second parameter is of the form T<A>.
    template <template<class> class T, class A>
    struct extract_template<T, T<A>, true> {
        using type = typename vectorize<common::partial_decay<A>>::type;
    };
}
//! @endcond

/**
 * @brief Deletes occurrences of the first (template) parameter within the second (type) parameter, which is built through array-like and tuple-like classes, adding constness where the template is not present.
 */
template <template<class> class T, class A>
using extract_template = typename details::extract_template<T, partial_decay<A>>::type;


//! @cond INTERNAL
namespace details {
    //! @brief general case.
    template <class A>
    struct template_args;

    //! @brief tuple-like case.
    template <template<class...> class T, class... A>
    struct template_args<T<A...>> {
        using type = type_sequence<A...>;
    };

    //! @brief const tuple-like case.
    template <template<class...> class T, class... A>
    struct template_args<T<A...> const> {
        using type = type_sequence<A const...>;
    };

    //! @brief & tuple-like case.
    template <template<class...> class T, class... A>
    struct template_args<T<A...>&> {
        using type = type_sequence<A&...>;
    };

    //! @brief const& tuple-like case.
    template <template<class...> class T, class... A>
    struct template_args<T<A...> const&> {
        using type = type_sequence<A const&...>;
    };

    //! @brief && tuple-like case.
    template <template<class...> class T, class... A>
    struct template_args<T<A...>&&> {
        using type = type_sequence<A&&...>;
    };

    //! @brief const&& tuple-like case.
    template <template<class...> class T, class... A>
    struct template_args<T<A...> const&&> {
        using type = type_sequence<A const&&...>;
    };

    //! @brief array-like case.
    template <template<class,size_t> class T, class A, size_t N>
    struct template_args<T<A, N>> {
        using type = type_sequence<A>;
    };

    //! @brief const array-like case.
    template <template<class,size_t> class T, class A, size_t N>
    struct template_args<T<A, N> const> {
        using type = type_sequence<A const>;
    };

    //! @brief & array-like case.
    template <template<class,size_t> class T, class A, size_t N>
    struct template_args<T<A, N>&> {
        using type = type_sequence<A&>;
    };

    //! @brief const& array-like case.
    template <template<class,size_t> class T, class A, size_t N>
    struct template_args<T<A, N> const&> {
        using type = type_sequence<A const&>;
    };

    //! @brief && array-like case.
    template <template<class,size_t> class T, class A, size_t N>
    struct template_args<T<A, N>&&> {
        using type = type_sequence<A&&>;
    };

    //! @brief const&& array-like case.
    template <template<class,size_t> class T, class A, size_t N>
    struct template_args<T<A, N> const&&> {
        using type = type_sequence<A const&&>;
    };
}
//! @endcond

/**
 * @brief Returns the class arguments of a template types as a type sequence, propagating value type.
 */
template <class A>
using template_args = typename details::template_args<A>::type;


/**
 * @name if_signature
 *
 * Enables template resolution if a callable class @param G complies to a given signature @param F.
 */
template <typename G, typename F, typename T = void>
using if_signature = std::enable_if_t<std::is_convertible<G,std::function<F>>::value, T>;


//! @cond INTERNAL
namespace details {
    //! @brief general case.
    template <typename T>
    struct type_unwrap;

    //! @brief defined case.
    template <typename T>
    struct type_unwrap<void(type_sequence<T>)> {
        using type = T;
    };
}
//! @endcond

//! @brief Allows passing a type with commas as macro argument.
template <typename T>
using type_unwrap = typename details::type_unwrap<T>::type;


/**
 * @name number_sequence
 *
 * Helper empty class encapsulating a sequence of numbers.
 * Mimics operations in standard stl containers.
 */
//! @{
//! @brief General form.
template <intmax_t... xs>
struct number_sequence;


// Base case (0 if not found).
template <intmax_t a, intmax_t... xs>
constexpr size_t number_count = 0;

// General recursive pattern.
template <intmax_t a, intmax_t b, intmax_t... xs>
constexpr size_t number_count<a, b, xs...> = number_count<a, xs...> + (a == b ? 1 : 0);

// General recursive pattern.
template <intmax_t a, intmax_t b, intmax_t... xs>
constexpr size_t number_find = number_find<a, xs...> + 1;

// Base case (the searched type is first).
template <intmax_t a, intmax_t... xs>
constexpr size_t number_find<a, a, xs...> = 0;


//! @cond INTERNAL
namespace details {
    // General form.
    template<int start, int end, int stride, intmax_t... xs>
    struct number_slice {
        using type = number_sequence<>;
    };

    // Recursive form.
    template<int start, int end, int stride, intmax_t x, intmax_t... xs>
    struct number_slice<start, end, stride, x, xs...> {
        using type = std::conditional_t<end == 0, number_sequence<>, std::conditional_t<
            start == 0,
            typename number_slice<stride - 1, end - 1, stride, xs...>::type::template push_front<x>,
            typename number_slice<start  - 1, end - 1, stride, xs...>::type
        >>;
    };
}
//! @endcond

/**
 * @brief Extracts a subsequence from the number sequence.
 * @param start  first element extracted
 * @param end    no element extracted after end (defaults to -1 = end of the sequence)
 * @param stride interval between element extracted (defaults to 1)
 */
template <int start, int end, int stride, intmax_t... xs>
using number_slice = typename details::number_slice<start, end, stride, xs...>::type;

//! @brief Extracts the n-th type from the sequence.
template <int n, intmax_t... xs>
constexpr intmax_t number_get = number_slice<n, n+1, 1, xs...>::front;


//! @brief Non-empty form, allows for extracting elements and subsequences.
template <intmax_t x, intmax_t... xs>
struct number_sequence<x, xs...> {
    //! @brief Extracts the n-th number from the sequence.
    template <int n>
    static constexpr intmax_t get = number_get<n, x, xs...>;

    /**
     * @brief Extracts a subsequence from the number sequence.
     * @param start  first element extracted
     * @param end    no element extracted after end (defaults to -1 = end of the sequence)
     * @param stride interval between element extracted (defaults to 1)
     */
    template <int start, int end = -1, int stride = 1>
    using slice = number_slice<start, end, stride, x, xs...>;

    //! @brief The first number of the sequence.
    static constexpr intmax_t front = x;

    //! @brief The last number of the sequence.
    static constexpr intmax_t back = get<sizeof...(xs)>;

    //! @brief Removes the first number of the sequence.
    using pop_front = number_sequence<xs...>;

    //! @brief Removes the last number of the sequence.
    using pop_back = slice<0, sizeof...(xs)>;

    //! @brief Adds types at the front of the sequence.
    template <intmax_t... ys>
    using push_front = number_sequence<ys..., x, xs...>;

    //! @brief Adds types at the back of the sequence.
    template <intmax_t... ys>
    using push_back = number_sequence<x, xs..., ys...>;

//    //! @brief Set intersection with other sequence.
//    template<intmax_t... ys>
//    using intersect = number_intersect<number_sequence<x, xs...>, number_sequence<ys...>>;
//
//    //! @brief Set union with other sequence.
//    template<intmax_t... ys>
//    using unite = number_unite<number_sequence<x, xs...>, number_sequence<ys...>>;
//
//    //! @brief Set difference with other sequence.
//    template<intmax_t... ys>
//    using subtract = number_subtract<number_sequence<x, xs...>, number_sequence<ys...>>;
//
//    //! @brief Extract the types that are repeated more than once.
//    using repeated = number_repeated<x, xs...>;
//
//    //! @brief Extract the subsequence in which each type appears once (opposite of repeated).
//    using uniq = number_uniq<x, xs...>;

    //! @brief Constant equal to the index of `y` among the sequence. Fails to compile if not present, indices start from 0.
    template <intmax_t y>
    static constexpr size_t find = number_find<y, x, xs...>;

    //! @brief Constant which is true if and only if the type parameter is in the sequence.
    template <intmax_t y>
    static constexpr size_t count = number_count<y, x, xs...>;

    //! @brief The length of the sequence.
    static constexpr size_t size = 1 + sizeof...(xs);
};

//! @brief Empty form, cannot extract elements and subsequences.
template <>
struct number_sequence<> {
    template <int start, int end = -1, int stride = 1>
    using slice = number_sequence<>;

    template <intmax_t... ys>
    using push_front = number_sequence<ys...>;

    template <intmax_t... ys>
    using push_back = number_sequence<ys...>;

    template<intmax_t... ys>
    using intersect = number_sequence<>;

    template<intmax_t... ys>
    using unite = number_sequence<ys...>;

    template<intmax_t... ys>
    using subtract = number_sequence<>;

    using repeated = number_sequence<>;

    using uniq = number_sequence<>;

    template <intmax_t y>
    static constexpr size_t count = 0;

    static constexpr size_t size = 0;
};
//! @}


//! @cond INTERNAL
namespace details {
    // Identity function on type sequences.
    template <typename... Ts>
    type_sequence<Ts...> type_sequence_identity(type_sequence<Ts...>) {
        return {};
    }

    // Decays a type into a type sequence (type not convertible to type sequence).
    template <typename T, typename = void>
    struct type_sequence_decay_impl {
        using type = type_sequence<>;
    };

    // Decays a type into a type sequence (type is convertible to type sequence).
    template <typename T>
    struct type_sequence_decay_impl<T(), decltype(void(type_sequence_identity(std::declval<T>())))> {
        using type = decltype(type_sequence_identity(std::declval<T>()));
    };

    // Decays a type into a type sequence.
    template <typename T>
    using type_sequence_decay = typename type_sequence_decay_impl<T()>::type;

    // Extracts a boolean option (no arguments).
    template <template<bool> class T, bool d, typename... Ss>
    struct option_flag : public std::integral_constant<bool, d> {};

    // Extracts a boolean option (option in first place).
    template <template<bool> class T, bool d, bool b, typename... Ss>
    struct option_flag<T,d,T<b>,Ss...> : public std::integral_constant<bool, b> {};

    // Extracts a boolean option (type sequence in first place).
    template <template<bool> class T, bool d, typename... Ts, typename... Ss>
    struct option_flag<T,d,type_sequence<Ts...>,Ss...> : public option_flag<T,d,Ts...,Ss...> {};

    // Extracts a boolean option (something else in first place).
    template <template<bool> class T, bool d, typename S, typename... Ss>
    struct option_flag<T,d,S,Ss...> : public option_flag<T,d,type_sequence_decay<S>,Ss...> {};

    // Extracts a numeric option (no arguments).
    template <template<intmax_t> class T, intmax_t d, typename... Ss>
    struct option_num : public std::integral_constant<intmax_t, d> {};

    // Extracts a numeric option (option in first place).
    template <template<intmax_t> class T, intmax_t d, intmax_t i, typename... Ss>
    struct option_num<T,d,T<i>,Ss...> : public std::integral_constant<intmax_t, i> {};

    // Extracts a numeric option (type sequence in first place).
    template <template<intmax_t> class T, intmax_t d, typename... Ts, typename... Ss>
    struct option_num<T,d,type_sequence<Ts...>,Ss...> : public option_num<T,d,Ts...,Ss...> {};

    // Extracts a numeric option (something else in first place).
    template <template<intmax_t> class T, intmax_t d, typename S, typename... Ss>
    struct option_num<T,d,S,Ss...> : public option_num<T,d,type_sequence_decay<S>,Ss...> {};

    // Prepends indexes to an index sequence (general form).
    template <typename T, intmax_t... is>
    struct nums_prepend;

    // Prepends indexes to an index sequence (active form).
    template <intmax_t... js, intmax_t... is>
    struct nums_prepend<number_sequence<js...>, is...> {
        using type = number_sequence<is..., js...>;
    };

    // Extracts a multinumeric option (no arguments).
    template <template<intmax_t...> class T, typename... Ss>
    struct option_nums {
        using type = number_sequence<>;
    };

    // Extracts a multinumeric option (option in first place).
    template <template<intmax_t...> class T, intmax_t... is, typename... Ss>
    struct option_nums<T, T<is...>, Ss...> : public nums_prepend<typename option_nums<T, Ss...>::type, is...> {};

    // Extracts a multinumeric option (type sequence in first place).
    template <template<intmax_t...> class T, typename... Ts, typename... Ss>
    struct option_nums<T, type_sequence<Ts...>, Ss...> : public option_nums<T, Ts..., Ss...> {};

    // Extracts a multinumeric option (something else in first place).
    template <template<intmax_t...> class T, typename S, typename... Ss>
    struct option_nums<T, S, Ss...> : public option_nums<T, type_sequence_decay<S>, Ss...> {};

    // Extracts a floating-point numeric option (no arguments).
    template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, typename... Ss>
    struct option_float {
        static constexpr double value = dnum / (double)dden;
    };

    // Extracts a floating-point numeric option (option in first place).
    template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, intmax_t i, intmax_t j, typename... Ss>
    struct option_float<T,dnum,dden,T<i,j>,Ss...> {
        static constexpr double value = i / (double)j;
    };

    // Extracts a floating-point numeric option (type sequence in first place).
    template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, typename... Ts, typename... Ss>
    struct option_float<T,dnum,dden,type_sequence<Ts...>,Ss...> : public option_float<T,dnum,dden,Ts...,Ss...> {};

    // Extracts a floating-point numeric option (something else in first place).
    template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, typename S, typename... Ss>
    struct option_float<T,dnum,dden,S,Ss...> : public option_float<T,dnum,dden,type_sequence_decay<S>,Ss...> {};

    // Extracts a type option (no arguments).
    template <template<class> class T, typename D, typename... Ss>
    struct option_type {
        using type = D;
    };

    // Extracts a type option (option in first place).
    template <template<class> class T, typename D, typename S, typename... Ss>
    struct option_type<T,D,T<S>,Ss...> {
        using type = S;
    };

    // Extracts a type option (type sequence in first place).
    template <template<class> class T, typename D, typename... Ts, typename... Ss>
    struct option_type<T,D,type_sequence<Ts...>,Ss...> : public option_type<T,D,Ts...,Ss...> {};

    // Extracts a type option (something else in first place).
    template <template<class> class T, typename D, typename S, typename... Ss>
    struct option_type<T,D,S,Ss...> : public option_type<T,D,type_sequence_decay<S>,Ss...> {};

    // Extracts a multitype option (no arguments).
    template <template<class...> class T, typename... Ss>
    struct option_types {
        using type = type_sequence<>;
    };

    // Extracts a multitype option (option in first place).
    template <template<class...> class T, typename... Ts, typename... Ss>
    struct option_types<T, T<Ts...>, Ss...> {
        using type = typename option_types<T, Ss...>::type::template push_front<Ts...>;
    };

    // Extracts a multitype option (type sequence in first place).
    template <template<class...> class T, typename... Ts, typename... Ss>
    struct option_types<T, type_sequence<Ts...>, Ss...> : public option_types<T, Ts..., Ss...> {};

    // Extracts a multitype option (something else in first place).
    template <template<class...> class T, typename S, typename... Ss>
    struct option_types<T, S, Ss...> : public option_types<T, type_sequence_decay<S>, Ss...> {};

    // Extracts a multitype option (no arguments).
    template <template<class...> class T, typename... Ss>
    struct option_multitypes {
        using type = type_sequence<>;
    };

    // Extracts a multitype option (option in first place).
    template <template<class...> class T, typename... Ts, typename... Ss>
    struct option_multitypes<T, T<Ts...>, Ss...> {
        using type = typename option_multitypes<T, Ss...>::type::template push_front<common::type_sequence<Ts...>>;
    };

    // Extracts a multitype option (type sequence in first place).
    template <template<class...> class T, typename... Ts, typename... Ss>
    struct option_multitypes<T, type_sequence<Ts...>, Ss...> : public option_multitypes<T, Ts..., Ss...> {};

    // Extracts a multitype option (something else in first place).
    template <template<class...> class T, typename S, typename... Ss>
    struct option_multitypes<T, S, Ss...> : public option_multitypes<T, type_sequence_decay<S>, Ss...> {};

    // Applies templates to arguments modelled as type sequences (base case).
    template <typename S, template<class...> class... T>
    struct apply_templates {
        using type = S;
    };

    // Applies templates to arguments modelled as type sequences (recursive form).
    template <typename... Ss, template<class...> class T, template<class...> class... Ts>
    struct apply_templates<type_sequence<Ss...>, T, Ts...> {
        using type = T<typename apply_templates<Ss, Ts...>::type...>;
    };
}
//! @endcond

/**
 * @brief Extracts a boolean option from a sequence of options.
 *
 * @param T Flag option name.
 * @param d Default value if the option is missing.
 * @param Ss Sequence of options.
 */
template <template<bool> class T, bool d, typename... Ss>
constexpr bool option_flag = details::option_flag<T,d,Ss...>::value;

/**
 * @brief Extracts a numeric option from a sequence of options.
 *
 * @param T Numeric option name.
 * @param d Default value if the option is missing.
 * @param Ss Sequence of options.
 */
template <template<intmax_t> class T, intmax_t d, typename... Ss>
constexpr intmax_t option_num = details::option_num<T,d,Ss...>::value;

/**
 * @brief Extracts a numeric or multi-numeric option from a sequence of options as an index sequence.
 *
 * @param T Multi-numeric option name.
 * @param Ss Sequence of options.
 */
template <template<intmax_t...> class T, typename... Ss>
using option_nums = typename details::option_nums<T, Ss...>::type;

/**
 * @brief Extracts a floating-point numeric option from a sequence of options.
 *
 * @param T Floating-point numeric option name.
 * @param dnum Default numerator of the value if the option is missing.
 * @param dden Default denumerator of the value if the option is missing.
 * @param Ss Sequence of options.
 */
template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, typename... Ss>
constexpr double option_float = details::option_float<T,dnum,dden,Ss...>::value;

/**
 * @brief Extracts a type option from a sequence of options.
 *
 * @param T Type option name.
 * @param D Default type if the option is missing.
 * @param Ss Sequence of options.
 */
template <template<class> class T, typename D, typename... Ss>
using option_type = typename details::option_type<T,D,Ss...>::type;

/**
 * @brief Extracts a type or multi-type option from a sequence of options as a type sequence.
 *
 * @param T Multi-type option name.
 * @param Ss Sequence of options.
 */
template <template<class...> class T, typename... Ss>
using option_types = typename details::option_types<T, Ss...>::type;

/**
 * @brief Extracts a multi-type option from a sequence of options as a type sequence of type sequences.
 *
 * @param T Multi-type option name.
 * @param Ss Sequence of options.
 */
template <template<class...> class T, typename... Ss>
using option_multitypes = typename details::option_multitypes<T, Ss...>::type;


/**
 * @brief Instantiates (possibly nested) templates with types wrapped in (possibly nested) type sequences.
 *
 * @param S The arguments as (possibly nested) type sequence.
 * @param Ts Sequence of templates, to be applied in nested levels.
 */
template <typename S, template<class...> class... Ts>
using apply_templates = typename details::apply_templates<S,Ts...>::type;


//! @cond INTERNAL
namespace details {
    //! @brief Representation of constness and value type of a type.
    //! @{
    //! @brief base case.
    template <typename T>
    struct reference_string {
        inline static std::string mangled() {
            return "";
        }
        inline static std::string demangled() {
            return "";
        }
    };
    //! @brief const case.
    template <typename T>
    struct reference_string<T const> {
        inline static std::string mangled() {
            return "K";
        }
        inline static std::string demangled() {
            return " const";
        }
    };
    //! @brief lvalue reference case.
    template <typename T>
    struct reference_string<T&> {
        inline static std::string mangled() {
            return "R" + reference_string<T>::mangled();
        }
        inline static std::string demangled() {
            return reference_string<T>::demangled() + "&";
        }
    };
    //! @brief rvalue reference case.
    template <typename T>
    struct reference_string<T&&> {
        inline static std::string mangled() {
            return "O" + reference_string<T>::mangled();
        }
        inline static std::string demangled() {
            return reference_string<T>::demangled() + "&&";
        }
    };
    //! @}
}
//! @endcond

#ifdef NABI
//! @brief Returns the string representation of a type `T`.
template <typename T>
std::string type_name() {
    return details::reference_string<T>::mangled() + typeid(T).name();
}
#else
//! @brief Returns the string representation of a type `T`.
template <typename T>
std::string type_name() {
    int status = 42;
    char const* name = typeid(T).name();
    std::unique_ptr<char, void(*)(void*)> res{abi::__cxa_demangle(name, NULL, NULL, &status), std::free};
    return (status==0) ? res.get() + details::reference_string<T>::demangled() : details::reference_string<T>::mangled() + name;
}
#endif

//! @brief Returns the string representation of the type of the argument.
template <typename T>
inline std::string type_name(T&&) {
    return type_name<T>();
}


//! @brief Escapes a value for clear printing.
//! @{
inline std::string escape(bool x) {
    return x ? "true" : "false";
}
inline std::string escape(char x) {
    if (x < 32 or x > 126) {
        int y = x < 0 ? x + 256 : x;
        return "'\\" + std::to_string(y/64) + std::to_string((y/8)%8) + std::to_string(y%8) + "'";
    }
    if (x == '\'') return "'\\''";
    return std::string("'") + x + "'";
}
inline int escape(int8_t x) {
    return x;
}
inline int escape(uint8_t x) {
    return x;
}
inline std::string escape(std::string x) {
    std::string r;
    r.push_back('"');
    for (char c : x) {
        if (c == '"') r.push_back('\\');
        r.push_back(c);
    }
    r.push_back('"');
    return r;
}
inline std::string escape(char const* x) {
    return escape(std::string(x));
}
template <typename T, typename = std::enable_if_t<std::is_empty<T>::value>>
inline std::string escape(T) {
    return type_name<T>();
}
template <typename T, typename = std::enable_if_t<type_count<T, bool, char, int8_t, uint8_t, std::string, char const*> == 0 and not std::is_empty<T>::value>>
inline T const& escape(T const& x) {
    return x;
}
//! @}

/**
 * @brief Declares that the type @param T is usable as an output stream.
 * Enabled by default for all subtypes of std::ostream.
 */
template <typename T>
struct is_ostream : public std::is_base_of<std::ostream, T> {};

//! @brief Corresponds to T only if A is an output stream according to fcpp::common::is_ostream.
template <typename A, typename T = void>
using if_ostream = std::enable_if_t<fcpp::common::is_ostream<A>::value, T>;

}


}

#endif  // FCPP_COMMON_TRAITS_H_
