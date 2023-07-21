// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file type_sequence.hpp
 * @brief Empty class encapsulating a sequence of types, mimicking operations in standard stl containers.
 */

#ifndef FCPP_COMMON_TYPE_SEQUENCE_H_
#define FCPP_COMMON_TYPE_SEQUENCE_H_

#include <cstddef>
#include <type_traits>


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


//! @brief Count operation base case (0 if not found).
template <typename A, typename... Ts>
constexpr size_t type_count = 0;

//! @brief Count operation general recursive pattern.
template <typename A, typename B, typename... Ts>
constexpr size_t type_count<A, B, Ts...> = type_count<A, Ts...> + (std::is_same<A,B>::value ? 1 : 0);

//! @brief Find operation general recursive pattern.
template <typename A, typename B, typename... Ts>
constexpr size_t type_find = type_find<A, Ts...> + 1;

//! @brief Find operation base case (the searched type is first).
template <typename A, typename... Ts>
constexpr size_t type_find<A, A, Ts...> = 0;


//! @cond INTERNAL
namespace details {
    //! @brief General form.
    template<int start, int end, int stride, typename... Ts>
    struct type_slice {
        using type = type_sequence<>;
    };

    //! @brief Recursive form.
    template<int start, int end, int stride, typename T, typename... Ts>
    struct type_slice<start, end, stride, T, Ts...> {
        using type = std::conditional_t<end == 0, type_sequence<>, std::conditional_t<
            start == 0,
            typename type_slice<stride - 1, end - 1, stride, Ts...>::type::template push_front<T>,
            typename type_slice<start  - 1, end - 1, stride, Ts...>::type
        >>;
    };

    //! @brief Base case.
    template <typename, typename>
    struct type_intersect {
        using type = type_sequence<>;
    };

    //! @brief Recursive form.
    template <typename T, typename... Ts, typename... Ss>
    struct type_intersect<type_sequence<T, Ts...>, type_sequence<Ss...>> {
        using type = std::conditional_t<
            type_count<T, Ss...> != 0,
            typename type_intersect<type_sequence<Ts...>, type_sequence<Ss...>>::type::template push_front<T>,
            typename type_intersect<type_sequence<Ts...>, type_sequence<Ss...>>::type
        >;
    };

    //! @brief General form.
    template <typename, typename>
    struct type_unite;

    //! @brief Recursive form.
    template <typename... Ts, typename S, typename... Ss>
    struct type_unite<type_sequence<Ts...>, type_sequence<S, Ss...>> {
        using type = std::conditional_t<
            type_count<S, Ts...> != 0,
            typename type_unite<type_sequence<Ts...>, type_sequence<Ss...>>::type,
            typename type_unite<type_sequence<Ts..., S>, type_sequence<Ss...>>::type
        >;
    };

    //! @brief Base case.
    template <typename... Ts>
    struct type_unite<type_sequence<Ts...>, type_sequence<>> {
        using type = type_sequence<Ts...>;
    };

    //! @brief General form.
    template <typename, typename>
    struct type_subtract;

    //! @brief Recursive form.
    template <typename T, typename... Ts, typename... Ss>
    struct type_subtract<type_sequence<T, Ts...>, type_sequence<Ss...>> {
        using type = std::conditional_t<
            type_count<T, Ss...> != 0,
            typename type_subtract<type_sequence<Ts...>, type_sequence<Ss...>>::type,
            typename type_subtract<type_sequence<Ts...>, type_sequence<Ss...>>::type::template push_front<T>
        >;
    };

    //! @brief Base case.
    template <typename... Ss>
    struct type_subtract<type_sequence<>, type_sequence<Ss...>> {
        using type = type_sequence<>;
    };

    //! @brief General form.
    template <typename...>
    struct type_repeated {
        using type = type_sequence<>;
    };

    //! @brief Recursive form.
    template <typename T, typename... Ts>
    struct type_repeated<T, Ts...> {
        using type = std::conditional_t<
            type_count<T, Ts...> == 0,
            typename type_repeated<Ts...>::type,
            typename type_repeated<Ts...>::type::template push_front<T>
        >;
    };

    //! @brief General form.
    template <typename...>
    struct type_uniq {
        using type = type_sequence<>;
    };

    //! @brief Recursive form.
    template <typename T, typename... Ts>
    struct type_uniq<T, Ts...> {
        using type = std::conditional_t<
            type_count<T, Ts...> == 0,
            typename type_uniq<Ts...>::type::template push_front<T>,
            typename type_uniq<Ts...>::type
        >;
    };

    //! @brief General form.
    template <typename... Ts>
    struct type_cat;

    //! @brief Empty base case.
    template <>
    struct type_cat<> {
        using type = type_sequence<>;
    };

    //! @brief Base case.
    template <typename... Ts, typename... Ss>
    struct type_cat<type_sequence<Ts...>, type_sequence<Ss...>> {
        using type = type_sequence<Ts..., Ss...>;
    };

    //! @brief Recursive form.
    template <typename T, typename... Ss>
    struct type_cat<T, Ss...> {
        using type = typename type_cat<T, typename type_cat<Ss...>::type>::type;
    };

    //! @brief General form.
    template <typename... Ts>
    struct type_product;

    //! @brief Empty base case.
    template <>
    struct type_product<> {
        using type = type_sequence<type_sequence<>>;
    };

    //! @brief Base case with single option.
    template <typename... Ts, typename S>
    struct type_product<type_sequence<Ts...>, type_sequence<S>> {
        using type = type_sequence<typename type_cat<Ts,S>::type...>;
    };

    //! @brief Base case.
    template <typename T, typename... Ss>
    struct type_product<T, type_sequence<Ss...>> {
        using type = typename type_cat<typename type_product<T,type_sequence<Ss>>::type...>::type;
    };

    //! @brief Recursive form.
    template <typename T, typename... Ss>
    struct type_product<T, Ss...> {
        using type = typename type_product<T, typename type_product<Ss...>::type>::type;
    };
}
//! @endcond


/**
 * @brief Extracts a subsequence from the type sequence.
 *
 * @tparam start  first element extracted
 * @tparam end    no element extracted after end (defaults to -1 = end of the sequence)
 * @tparam stride interval between element extracted (defaults to 1)
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


//! @brief Non-empty type sequence, allows for extracting elements and subsequences.
template <typename T, typename... Ts>
struct type_sequence<T, Ts...> {
    //! @brief Extracts the n-th type from the sequence.
    template <int n>
    using get = type_get<n, T, Ts...>;

    /**
     * @brief Extracts a subsequence from the type sequence.
     *
     * @tparam start  first element extracted
     * @tparam end    no element extracted after end (defaults to -1 = end of the sequence)
     * @tparam stride interval between element extracted (defaults to 1)
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
    template <typename... Ss>
    using intersect = type_intersect<type_sequence<T, Ts...>, type_sequence<Ss...>>;

    //! @brief Set union with other sequence.
    template <typename... Ss>
    using unite = type_unite<type_sequence<T, Ts...>, type_sequence<Ss...>>;

    //! @brief Set difference with other sequence.
    template <typename... Ss>
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

//! @brief Empty type sequence, cannot extract elements and subsequences.
template <>
struct type_sequence<> {
    //! @brief Extracts a subsequence from the type sequence.
    template <int start, int end = -1, int stride = 1>
    using slice = type_sequence<>;

    //! @brief Adds types at the front of the sequence.
    template <typename... Ss>
    using push_front = type_sequence<Ss...>;

    //! @brief Adds types at the back of the sequence.
    template <typename... Ss>
    using push_back = type_sequence<Ss...>;

    //! @brief Set intersection with other sequence.
    template <typename... Ss>
    using intersect = type_sequence<>;

    //! @brief Set union with other sequence.
    template <typename... Ss>
    using unite = type_sequence<Ss...>;

    //! @brief Set difference with other sequence.
    template <typename... Ss>
    using subtract = type_sequence<>;

    //! @brief Extract the types that are repeated more than once.
    using repeated = type_sequence<>;

    //! @brief Extract the subsequence in which each type appears once (opposite of repeated).
    using uniq = type_sequence<>;

    //! @brief Constant which is true if and only if the type parameter is in the sequence.
    template <typename S>
    static constexpr size_t count = 0;

    //! @brief The length of the sequence.
    static constexpr size_t size = 0;
};
//! @}


} // namespace common


} // namespace fcpp

#endif  // FCPP_COMMON_TYPE_SEQUENCE_H_
