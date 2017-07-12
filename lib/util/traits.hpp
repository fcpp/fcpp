// Copyright © 2017 Giorgio Audrito. All Rights Reserved.

/**
 * @file traits.hpp
 * @brief Constants and typedefs for andvanced operations and querying on types.
 */

#ifndef FCPP_UTIL_TRAITS_H_
#define FCPP_UTIL_TRAITS_H_

#include <cstddef>
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
/**
 * @brief Recursive pattern: either the first type occurs among the following, or the
 * following has a repetition.
 */
template <typename A, typename... Ts>
constexpr bool type_repeated = type_contains<A, Ts...> or type_repeated<Ts...>;

//! @brief Base case (a single type cannot be repeated).
template <typename A>
constexpr bool type_repeated<A> = false;
//@}


/**
 * @name is_template
 *
 * Constant which is true if and only if the second (type) parameter is a specialization of
 * the first (template) parameter.
 */
//@{
//! @brief False in general.
template <template<class...> class T, class A>
constexpr bool is_template = false;

//! @brief True if second parameter is of the form T<A...>.
template <template<class...> class T, class... A>
constexpr bool is_template<T, T<A...>> = true;
//@}


/**
 * @brief Helper class for implicit conversion to a template type.
 */
template <template<class...> class T, class A>
struct to_template {
    /**
     * @brief A if it is an instance of T, T<A> otherwise.
     */
    typedef typename std::conditional<is_template<T, A>, A, T<A>>::type type;
};


}

#endif  // FCPP_UTILS_TRAITS_H_
