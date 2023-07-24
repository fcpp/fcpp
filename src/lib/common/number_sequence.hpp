// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file number_sequence.hpp
 * @brief Empty class encapsulating a sequence of numbers, mimicking operations in standard stl containers in a constexpr way.
 */

#ifndef FCPP_COMMON_NUMBER_SEQUENCE_H_
#define FCPP_COMMON_NUMBER_SEQUENCE_H_

#include <cstddef>
#include <cstdint>
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
 * @name number_sequence
 *
 * Helper empty class encapsulating a sequence of numbers.
 * Mimics operations in standard stl containers.
 */
//! @{

//! @brief General form.
template <intmax_t... xs>
struct number_sequence;


//! @brief Count operation base case (0 if not found).
template <intmax_t a, intmax_t... xs>
constexpr size_t number_count = 0;

//! @brief Count operation general recursive pattern.
template <intmax_t a, intmax_t b, intmax_t... xs>
constexpr size_t number_count<a, b, xs...> = number_count<a, xs...> + (a == b ? 1 : 0);

//! @brief Find operation general recursive pattern.
template <intmax_t a, intmax_t b, intmax_t... xs>
constexpr size_t number_find = number_find<a, xs...> + 1;

//! @brief Find operation base case (the searched type is first).
template <intmax_t a, intmax_t... xs>
constexpr size_t number_find<a, a, xs...> = 0;

//! @brief True if every argument converts to `true`.
template <intmax_t... xs>
constexpr bool number_all_true = std::is_same<number_sequence<true, bool(xs)...>, number_sequence<bool(xs)..., true>>::value;

//! @brief True if every argument converts to `false`.
template <intmax_t... xs>
constexpr bool number_all_false = std::is_same<number_sequence<false, bool(xs)...>, number_sequence<bool(xs)..., false>>::value;

//! @brief True if some argument converts to `true`.
template <intmax_t... xs>
constexpr bool number_some_true = not number_all_false<xs...>;

//! @brief True if some argument converts to `false`.
template <intmax_t... xs>
constexpr bool number_some_false = not number_all_true<xs...>;


//! @cond INTERNAL
namespace details {
    //! @brief General form.
    template<intmax_t start, intmax_t end, intmax_t stride, intmax_t... xs>
    struct number_slice {
        using type = number_sequence<>;
    };

    //! @brief Recursive form.
    template<intmax_t start, intmax_t end, intmax_t stride, intmax_t x, intmax_t... xs>
    struct number_slice<start, end, stride, x, xs...> {
        using type = std::conditional_t<end == 0, number_sequence<>, std::conditional_t<
            start == 0,
            typename number_slice<stride - 1, end - 1, stride, xs...>::type::template push_front<x>,
            typename number_slice<start  - 1, end - 1, stride, xs...>::type
        >>;
    };

    //! @brief Base case.
    template <typename, typename>
    struct number_intersect {
        using type = number_sequence<>;
    };

    //! @brief Recursive form.
    template <intmax_t x, intmax_t... xs, intmax_t... ys>
    struct number_intersect<number_sequence<x, xs...>, number_sequence<ys...>> {
        using type = std::conditional_t<
            number_count<x, ys...> != 0,
            typename number_intersect<number_sequence<xs...>, number_sequence<ys...>>::type::template push_front<x>,
            typename number_intersect<number_sequence<xs...>, number_sequence<ys...>>::type
        >;
    };

    //! @brief General form.
    template <typename, typename>
    struct number_unite;

    //! @brief Recursive form.
    template <intmax_t... xs, intmax_t y, intmax_t... ys>
    struct number_unite<number_sequence<xs...>, number_sequence<y, ys...>> {
        using type = std::conditional_t<
            number_count<y, xs...> != 0,
            typename number_unite<number_sequence<xs...>, number_sequence<ys...>>::type,
            typename number_unite<number_sequence<xs..., y>, number_sequence<ys...>>::type
        >;
    };

    //! @brief Base case.
    template <intmax_t... xs>
    struct number_unite<number_sequence<xs...>, number_sequence<>> {
        using type = number_sequence<xs...>;
    };

    //! @brief General form.
    template <typename, typename>
    struct number_subtract;

    //! @brief Recursive form.
    template <intmax_t x, intmax_t... xs, intmax_t... ys>
    struct number_subtract<number_sequence<x, xs...>, number_sequence<ys...>> {
        using type = std::conditional_t<
            number_count<x, ys...> != 0,
            typename number_subtract<number_sequence<xs...>, number_sequence<ys...>>::type,
            typename number_subtract<number_sequence<xs...>, number_sequence<ys...>>::type::template push_front<x>
        >;
    };

    //! @brief Base case.
    template <intmax_t... ys>
    struct number_subtract<number_sequence<>, number_sequence<ys...>> {
        using type = number_sequence<>;
    };

    //! @brief General form.
    template <intmax_t...>
    struct number_repeated {
        using type = number_sequence<>;
    };

    //! @brief Recursive form.
    template <intmax_t x, intmax_t... xs>
    struct number_repeated<x, xs...> {
        using type = std::conditional_t<
            number_count<x, xs...> == 0,
            typename number_repeated<xs...>::type,
            typename number_repeated<xs...>::type::template push_front<x>
        >;
    };

    //! @brief General form.
    template <intmax_t...>
    struct number_uniq {
        using type = number_sequence<>;
    };

    //! @brief Recursive form.
    template <intmax_t x, intmax_t... xs>
    struct number_uniq<x, xs...> {
        using type = std::conditional_t<
            number_count<x, xs...> == 0,
            typename number_uniq<xs...>::type::template push_front<x>,
            typename number_uniq<xs...>::type
        >;
    };

    //! @brief General form.
    template <typename... Ts>
    struct number_cat;

    //! @brief Empty base case.
    template <>
    struct number_cat<> {
        using type = number_sequence<>;
    };

    //! @brief Base case.
    template <intmax_t... xs, intmax_t... ys>
    struct number_cat<number_sequence<xs...>, number_sequence<ys...>> {
        using type = number_sequence<xs..., ys...>;
    };

    //! @brief Recursive form.
    template <typename T, typename... Ss>
    struct number_cat<T, Ss...> {
        using type = typename number_cat<T, typename number_cat<Ss...>::type>::type;
    };
}
//! @endcond

/**
 * @brief Extracts a subsequence from the number sequence.
 *
 * @tparam start  first element extracted
 * @tparam end    no element extracted after end (defaults to -1 = end of the sequence)
 * @tparam stride interval between element extracted (defaults to 1)
 */
template <intmax_t start, intmax_t end, intmax_t stride, intmax_t... xs>
using number_slice = typename details::number_slice<start, end, stride, xs...>::type;

//! @brief Extracts the n-th type from the sequence.
template <intmax_t n, intmax_t... xs>
constexpr intmax_t number_get = number_slice<n, n+1, 1, xs...>::front;

//! @brief Extract the types that are repeated more than once.
template <intmax_t... xs>
using number_repeated = typename details::number_repeated<xs...>::type;

//! @brief Extract the subsequence in which each type appears once (opposite of repeated).
template <intmax_t... xs>
using number_uniq = typename details::number_uniq<xs...>::type;

//! @brief The type sequence intersection of two type sequences.
template <typename T, typename S>
using number_intersect = typename details::number_intersect<T,S>::type;

//! @brief The type sequence union of two type sequences.
template <typename T, typename S>
using number_unite = typename details::number_unite<T,S>::type;

//! @brief The sequence of types that are in the first type sequence but not in the second.
template <typename T, typename S>
using number_subtract = typename details::number_subtract<T,S>::type;

//! @brief Concatenates a sequence of type sequences.
template <typename... Ts>
using number_cat = typename details::number_cat<Ts...>::type;


//! @brief Non-empty form, allows for extracting elements and subsequences.
template <intmax_t x, intmax_t... xs>
struct number_sequence<x, xs...> {
    //! @brief Extracts the n-th number from the sequence.
    template <intmax_t n>
    static constexpr intmax_t get = number_get<n, x, xs...>;

    /**
     * @brief Extracts a subsequence from the number sequence.
     *
     * @tparam start  first element extracted
     * @tparam end    no element extracted after end (defaults to -1 = end of the sequence)
     * @tparam stride interval between element extracted (defaults to 1)
     */
    template <intmax_t start, intmax_t end = -1, intmax_t stride = 1>
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

    //! @brief Set intersection with other sequence.
    template <intmax_t... ys>
    using intersect = number_intersect<number_sequence<x, xs...>, number_sequence<ys...>>;

    //! @brief Set union with other sequence.
    template <intmax_t... ys>
    using unite = number_unite<number_sequence<x, xs...>, number_sequence<ys...>>;

    //! @brief Set difference with other sequence.
    template <intmax_t... ys>
    using subtract = number_subtract<number_sequence<x, xs...>, number_sequence<ys...>>;

    //! @brief Extract the types that are repeated more than once.
    using repeated = number_repeated<x, xs...>;

    //! @brief Extract the subsequence in which each type appears once (opposite of repeated).
    using uniq = number_uniq<x, xs...>;

    //! @brief Constant equal to the index of `y` among the sequence. Fails to compile if not present, indices start from 0.
    template <intmax_t y>
    static constexpr size_t find = number_find<y, x, xs...>;

    //! @brief Constant which is true if and only if the type parameter is in the sequence.
    template <intmax_t y>
    static constexpr size_t count = number_count<y, x, xs...>;

    //! @brief The length of the sequence.
    static constexpr size_t size = 1 + sizeof...(xs);

    //! @brief True if every argument converts to `true`.
    static constexpr bool all_true = number_all_true<bool(x), bool(xs)...>;

    //! @brief True if every argument converts to `false`.
    static constexpr bool all_false = number_all_false<bool(x), bool(xs)...>;

    //! @brief True if some argument converts to `true`.
    static constexpr bool some_true = number_some_true<bool(x), bool(xs)...>;

    //! @brief True if some argument converts to `false`.
    static constexpr bool some_false = number_some_false<bool(x), bool(xs)...>;
};

//! @brief Empty form, cannot extract elements and subsequences.
template <>
struct number_sequence<> {
    //! @brief Extracts a subsequence from the number sequence.
    template <intmax_t start, intmax_t end = -1, intmax_t stride = 1>
    using slice = number_sequence<>;

    //! @brief Adds types at the front of the sequence.
    template <intmax_t... ys>
    using push_front = number_sequence<ys...>;

    //! @brief Adds types at the back of the sequence.
    template <intmax_t... ys>
    using push_back = number_sequence<ys...>;

    //! @brief Set intersection with other sequence.
    template <intmax_t... ys>
    using intersect = number_sequence<>;

    //! @brief Set union with other sequence.
    template <intmax_t... ys>
    using unite = number_sequence<ys...>;

    //! @brief Set difference with other sequence.
    template <intmax_t... ys>
    using subtract = number_sequence<>;

    //! @brief Extract the types that are repeated more than once.
    using repeated = number_sequence<>;

    //! @brief Extract the subsequence in which each type appears once (opposite of repeated).
    using uniq = number_sequence<>;

    //! @brief Constant which is true if and only if the type parameter is in the sequence.
    template <intmax_t y>
    static constexpr size_t count = 0;

    //! @brief The length of the sequence.
    static constexpr size_t size = 0;

    //! @brief True if every argument converts to `true`.
    static constexpr bool all_true = number_all_true<>;

    //! @brief True if every argument converts to `false`.
    static constexpr bool all_false = number_all_false<>;

    //! @brief True if some argument converts to `true`.
    static constexpr bool some_true = number_some_true<>;

    //! @brief True if some argument converts to `false`.
    static constexpr bool some_false = number_some_false<>;
};
//! @}


} // namespace common


} // namespace fcpp

#endif  // FCPP_COMMON_NUMBER_SEQUENCE_H_
