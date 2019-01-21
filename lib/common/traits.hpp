// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

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
constexpr bool some_true = !all_false<v...>;

//! @brief Checks if some argument is `false`.
template <bool... v>
constexpr bool some_false = !all_true<v...>;
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


/**
 * @brief Deletes occurrences of the first (template) parameter within the second (type) parameter, which is built through arrays and tuples.
 */
// If no occurrences of the template are present.
template <template<class> class T, class A>
struct del_template {
    //! @brief The result of the conversion.
    typedef A type;
};
//! @cond INTERNAL
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
//! @endcond

    
//! @brief Converts the second (type) parameter into a specialization of the first (template) parameter.
template <template<class> class T, class A>
struct add_template {
    //! @brief The result of the conversion.
    typedef T<typename del_template<T, A>::type> type;
};


}

#endif  // FCPP_COMMON_TRAITS_H_
