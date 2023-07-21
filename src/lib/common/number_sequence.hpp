// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file number_sequence.hpp
 * @brief Empty class encapsulating a sequence of numbers, mimicking operations in standard stl containers in a constexpr way.
 */

#ifndef FCPP_COMMON_NUMBER_SEQUENCE_H_
#define FCPP_COMMON_NUMBER_SEQUENCE_H_

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


//! @cond INTERNAL
namespace details {
    //! @brief General form.
    template<int start, int end, int stride, intmax_t... xs>
    struct number_slice {
        using type = number_sequence<>;
    };

    //! @brief Recursive form.
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
 *
 * @tparam start  first element extracted
 * @tparam end    no element extracted after end (defaults to -1 = end of the sequence)
 * @tparam stride interval between element extracted (defaults to 1)
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
     *
     * @tparam start  first element extracted
     * @tparam end    no element extracted after end (defaults to -1 = end of the sequence)
     * @tparam stride interval between element extracted (defaults to 1)
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
    //! @brief Extracts a subsequence from the number sequence.
    template <int start, int end = -1, int stride = 1>
    using slice = number_sequence<>;

    //! @brief Adds types at the front of the sequence.
    template <intmax_t... ys>
    using push_front = number_sequence<ys...>;

    //! @brief Adds types at the back of the sequence.
    template <intmax_t... ys>
    using push_back = number_sequence<ys...>;

    //! @brief Set intersection with other sequence.
    template<intmax_t... ys>
    using intersect = number_sequence<>;

    //! @brief Set union with other sequence.
    template<intmax_t... ys>
    using unite = number_sequence<ys...>;

    //! @brief Set difference with other sequence.
    template<intmax_t... ys>
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
};
//! @}


} // namespace common


} // namespace fcpp

#endif  // FCPP_COMMON_NUMBER_SEQUENCE_H_
