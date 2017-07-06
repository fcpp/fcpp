// Copyright © 2017 Giorgio Audrito. All Rights Reserved.

#ifndef FCPP_UTIL_TRAITS_H_
#define FCPP_UTIL_TRAITS_H_

#include <cstddef>


namespace fcpp {


template <typename A, typename B, typename... Ts>
constexpr size_t type_index = type_index<A, Ts...> + 1;

template <typename A, typename... Ts>
constexpr size_t type_index<A, A, Ts...> = 0;


template <typename A, typename... Ts>
constexpr bool type_contains = false;

template <typename A, typename B, typename... Ts>
constexpr bool type_contains<A, B, Ts...> = type_contains<A, Ts...>;

template <typename A, typename... Ts>
constexpr bool type_contains<A, A, Ts...> = true;


template <typename A, typename... Ts>
constexpr bool type_repeated = type_contains<A, Ts...> or type_repeated<Ts...>;

template <typename A>
constexpr bool type_repeated<A> = false;


template <template<class...> class T, class A>
constexpr bool is_template = false;

template <template<class...> class T, class... A>
constexpr bool is_template<T, T<A...>> = true;


}

#endif  // FCPP_UTILS_TRAITS_H_
