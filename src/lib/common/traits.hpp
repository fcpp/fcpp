// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

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

#include <cassert>
#include <functional>
#include <memory>
#include <ostream>
#include <string>
#include <type_traits>
#include <vector>

#include "lib/common/number_sequence.hpp"
#include "lib/common/type_sequence.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


// GENERAL METAPROGRAMMING SUPPORT


//! @brief Unit type not holding any data.
struct unit {};

/**
 *  @brief Helper function ignoring its arguments.
 *
 *  Useful to allow parameter pack expansion of arbitrary expressions. Sample usage:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * common::ignore_args((<expr>,0)...);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <typename... Ts>
inline void ignore_args(Ts&&...) {}

/**
 *  @brief Helper function returning its argument.
 *
 *  Useful to allow parameter pack expansion of an expression that does not depend
 *  on a type parameter pack, according to the pack. Sample usage:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * f(common::type_pack_wrapper<Ts>(<expr>)...);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <typename, typename U>
inline U&& type_pack_wrapper(U&& x) {
    return std::forward<U>(x);
}

/**
 *  @brief Helper function returning its argument.
 *
 *  Useful to allow parameter pack expansion of an expression that does not depend
 *  on a integer parameter pack, according to the pack. Sample usage:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * f(common::type_pack_wrapper<xs>(<expr>)...);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
template <intmax_t, typename U>
inline U&& number_pack_wrapper(U&& x) {
    return std::forward<U>(x);
}


// TYPE PREDICATES

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
constexpr bool has_template<T, U<A...>> = number_some_true<has_template<T, A>...>;

//! @brief Recurse on array-like type arguments.
template <template<class...> class T, template<class,size_t> class U, class A, size_t N>
constexpr bool has_template<T, U<A, N>> = has_template<T, A>;
//! @}

//! @brief Enables if type is instance of a class template.
template <template<class...> class T, class A, class B = void>
using if_template = std::enable_if_t<has_template<T, A>, B>;

//! @brief Enables if type is not instance of a class template.
template <template<class...> class T, class A, class B = void>
using ifn_template = std::enable_if_t<not has_template<T, A>, B>;


/**
 * @brief Enables if type is within a list of types.
 *
 * The last argument is the return type if the function is enabled, and cannot be omitted.
 */
template <typename T, typename... Ts>
using if_among = std::enable_if_t<type_sequence<Ts...>::pop_back::template count<T>, typename type_sequence<Ts...>::back>;

/**
 * @brief Enables if type is within a list of types.
 *
 * The last argument is the return type if the function is enabled, and cannot be omitted.
 */
template <typename T, typename... Ts>
using ifn_among = std::enable_if_t<not type_sequence<Ts...>::pop_back::template count<T>, typename type_sequence<Ts...>::back>;


/**
 * @brief Declares that the type @tparam T is usable as an output stream.
 *
 * Enabled by default for all subtypes of std::ostream.
 */
template <typename T>
struct is_ostream : public std::is_base_of<std::ostream, T> {};

//! @brief Corresponds to T only if A is an output stream according to \ref fcpp::common::is_ostream.
template <typename A, typename T = void>
using if_ostream = std::enable_if_t<fcpp::common::is_ostream<A>::value, T>;

//! @brief Corresponds to T only if A is not an output stream according to \ref fcpp::common::is_ostream.
template <typename A, typename T = void>
using ifn_ostream = std::enable_if_t<not fcpp::common::is_ostream<A>::value, T>;


//! @brief Enables template resolution if a callable class @tparam G complies to a given signature @tparam F.
template <typename G, typename F, typename T = void>
using if_signature = std::enable_if_t<std::is_convertible<G,std::function<F>>::value, T>;


// TYPE TRANSFORMATIONS


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

    //! @brief general case.
    template <typename T>
    struct type_unwrap;

    //! @brief defined case.
    template <typename T>
    struct type_unwrap<void(type_sequence<T>)> {
        using type = T;
    };

    //! @brief Applies templates to arguments modelled as type sequences (base case).
    template <typename S, template<class...> class... T>
    struct apply_templates {
        using type = S;
    };

    //! @brief Applies templates to arguments modelled as type sequences (recursive form).
    template <typename... Ss, template<class...> class T, template<class...> class... Ts>
    struct apply_templates<type_sequence<Ss...>, T, Ts...> {
        using type = T<typename apply_templates<Ss, Ts...>::type...>;
    };
}
//! @endcond


//! @brief The type that should be returned by a function forwarding an argument of type T.
template <typename T>
using partial_decay = typename details::partial_decay<T>::type;


//! @brief Allows passing a type with commas as macro argument.
template <typename T>
using type_unwrap = typename details::type_unwrap<T>::type;


/**
 * @brief Instantiates (possibly nested) templates with types wrapped in (possibly nested) type sequences.
 *
 * @tparam S The arguments as (possibly nested) type sequence.
 * @tparam Ts Sequence of templates, to be applied in nested levels.
 */
template <typename S, template<class...> class... Ts>
using apply_templates = typename details::apply_templates<S,Ts...>::type;


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

//! @brief Returns the class arguments of a template types as a type sequence, propagating value type.
template <class A>
using template_args = typename details::template_args<A>::type;


// GENERIC OPTION PASSING


//! @cond INTERNAL
namespace details {
    //! @brief Identity function on type sequences.
    template <typename... Ts>
    type_sequence<Ts...> type_sequence_identity(type_sequence<Ts...>) {
        return {};
    }

    //! @brief Decays a type into a type sequence (type not convertible to type sequence).
    template <typename T, typename = void>
    struct type_sequence_decay_impl {
        using type = type_sequence<>;
    };

    //! @brief Decays a type into a type sequence (type is convertible to type sequence).
    template <typename T>
    struct type_sequence_decay_impl<T(), decltype(void(type_sequence_identity(std::declval<T>())))> {
        using type = decltype(type_sequence_identity(std::declval<T>()));
    };

    //! @brief Decays a type into a type sequence.
    template <typename T>
    using type_sequence_decay = typename type_sequence_decay_impl<T()>::type;

    //! @brief Extracts a boolean option (no arguments).
    template <template<bool> class T, bool d, typename... Ss>
    struct option_flag : public std::integral_constant<bool, d> {};

    //! @brief Extracts a boolean option (option in first place).
    template <template<bool> class T, bool d, bool b, typename... Ss>
    struct option_flag<T,d,T<b>,Ss...> : public std::integral_constant<bool, b> {};

    //! @brief Extracts a boolean option (type sequence in first place).
    template <template<bool> class T, bool d, typename... Ts, typename... Ss>
    struct option_flag<T,d,type_sequence<Ts...>,Ss...> : public option_flag<T,d,Ts...,Ss...> {};

    //! @brief Extracts a boolean option (something else in first place).
    template <template<bool> class T, bool d, typename S, typename... Ss>
    struct option_flag<T,d,S,Ss...> : public option_flag<T,d,type_sequence_decay<S>,Ss...> {};

    //! @brief Extracts a numeric option (no arguments).
    template <template<intmax_t> class T, intmax_t d, typename... Ss>
    struct option_num : public std::integral_constant<intmax_t, d> {};

    //! @brief Extracts a numeric option (option in first place).
    template <template<intmax_t> class T, intmax_t d, intmax_t i, typename... Ss>
    struct option_num<T,d,T<i>,Ss...> : public std::integral_constant<intmax_t, i> {};

    //! @brief Extracts a numeric option (type sequence in first place).
    template <template<intmax_t> class T, intmax_t d, typename... Ts, typename... Ss>
    struct option_num<T,d,type_sequence<Ts...>,Ss...> : public option_num<T,d,Ts...,Ss...> {};

    //! @brief Extracts a numeric option (something else in first place).
    template <template<intmax_t> class T, intmax_t d, typename S, typename... Ss>
    struct option_num<T,d,S,Ss...> : public option_num<T,d,type_sequence_decay<S>,Ss...> {};

    //! @brief Prepends indexes to an index sequence (general form).
    template <typename T, intmax_t... is>
    struct nums_prepend;

    //! @brief Prepends indexes to an index sequence (active form).
    template <intmax_t... js, intmax_t... is>
    struct nums_prepend<number_sequence<js...>, is...> {
        using type = number_sequence<is..., js...>;
    };

    //! @brief Extracts a multinumeric option (no arguments).
    template <template<intmax_t...> class T, typename... Ss>
    struct option_nums {
        using type = number_sequence<>;
    };

    //! @brief Extracts a multinumeric option (option in first place).
    template <template<intmax_t...> class T, intmax_t... is, typename... Ss>
    struct option_nums<T, T<is...>, Ss...> : public nums_prepend<typename option_nums<T, Ss...>::type, is...> {};

    //! @brief Extracts a multinumeric option (type sequence in first place).
    template <template<intmax_t...> class T, typename... Ts, typename... Ss>
    struct option_nums<T, type_sequence<Ts...>, Ss...> : public option_nums<T, Ts..., Ss...> {};

    //! @brief Extracts a multinumeric option (something else in first place).
    template <template<intmax_t...> class T, typename S, typename... Ss>
    struct option_nums<T, S, Ss...> : public option_nums<T, type_sequence_decay<S>, Ss...> {};

    //! @brief Extracts a floating-point numeric option (no arguments).
    template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, typename... Ss>
    struct option_float {
        static constexpr double value = dnum / (double)dden;
    };

    //! @brief Extracts a floating-point numeric option (option in first place).
    template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, intmax_t i, intmax_t j, typename... Ss>
    struct option_float<T,dnum,dden,T<i,j>,Ss...> {
        static constexpr double value = i / (double)j;
    };

    //! @brief Extracts a floating-point numeric option (type sequence in first place).
    template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, typename... Ts, typename... Ss>
    struct option_float<T,dnum,dden,type_sequence<Ts...>,Ss...> : public option_float<T,dnum,dden,Ts...,Ss...> {};

    //! @brief Extracts a floating-point numeric option (something else in first place).
    template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, typename S, typename... Ss>
    struct option_float<T,dnum,dden,S,Ss...> : public option_float<T,dnum,dden,type_sequence_decay<S>,Ss...> {};

    //! @brief Extracts a type option (no arguments).
    template <template<class> class T, typename D, typename... Ss>
    struct option_type {
        using type = D;
    };

    //! @brief Extracts a type option (option in first place).
    template <template<class> class T, typename D, typename S, typename... Ss>
    struct option_type<T,D,T<S>,Ss...> {
        using type = S;
    };

    //! @brief Extracts a type option (type sequence in first place).
    template <template<class> class T, typename D, typename... Ts, typename... Ss>
    struct option_type<T,D,type_sequence<Ts...>,Ss...> : public option_type<T,D,Ts...,Ss...> {};

    //! @brief Extracts a type option (something else in first place).
    template <template<class> class T, typename D, typename S, typename... Ss>
    struct option_type<T,D,S,Ss...> : public option_type<T,D,type_sequence_decay<S>,Ss...> {};

    //! @brief Extracts a multitype option (no arguments).
    template <template<class...> class T, typename... Ss>
    struct option_types {
        using type = type_sequence<>;
    };

    //! @brief Extracts a multitype option (option in first place).
    template <template<class...> class T, typename... Ts, typename... Ss>
    struct option_types<T, T<Ts...>, Ss...> {
        using type = typename option_types<T, Ss...>::type::template push_front<Ts...>;
    };

    //! @brief Extracts a multitype option (type sequence in first place).
    template <template<class...> class T, typename... Ts, typename... Ss>
    struct option_types<T, type_sequence<Ts...>, Ss...> : public option_types<T, Ts..., Ss...> {};

    //! @brief Extracts a multitype option (something else in first place).
    template <template<class...> class T, typename S, typename... Ss>
    struct option_types<T, S, Ss...> : public option_types<T, type_sequence_decay<S>, Ss...> {};

    //! @brief Extracts a multitype option (no arguments).
    template <template<class...> class T, typename... Ss>
    struct option_multitypes {
        using type = type_sequence<>;
    };

    //! @brief Extracts a multitype option (option in first place).
    template <template<class...> class T, typename... Ts, typename... Ss>
    struct option_multitypes<T, T<Ts...>, Ss...> {
        using type = typename option_multitypes<T, Ss...>::type::template push_front<common::type_sequence<Ts...>>;
    };

    //! @brief Extracts a multitype option (type sequence in first place).
    template <template<class...> class T, typename... Ts, typename... Ss>
    struct option_multitypes<T, type_sequence<Ts...>, Ss...> : public option_multitypes<T, Ts..., Ss...> {};

    //! @brief Extracts a multitype option (something else in first place).
    template <template<class...> class T, typename S, typename... Ss>
    struct option_multitypes<T, S, Ss...> : public option_multitypes<T, type_sequence_decay<S>, Ss...> {};
}
//! @endcond

/**
 * @brief Extracts a boolean option from a sequence of options.
 *
 * @tparam T Flag option name.
 * @tparam d Default value if the option is missing.
 * @tparam Ss Sequence of options.
 */
template <template<bool> class T, bool d, typename... Ss>
constexpr bool option_flag = details::option_flag<T,d,Ss...>::value;

/**
 * @brief Extracts a numeric option from a sequence of options.
 *
 * @tparam T Numeric option name.
 * @tparam d Default value if the option is missing.
 * @tparam Ss Sequence of options.
 */
template <template<intmax_t> class T, intmax_t d, typename... Ss>
constexpr intmax_t option_num = details::option_num<T,d,Ss...>::value;

/**
 * @brief Extracts a numeric or multi-numeric option from a sequence of options as an index sequence.
 *
 * @tparam T Multi-numeric option name.
 * @tparam Ss Sequence of options.
 */
template <template<intmax_t...> class T, typename... Ss>
using option_nums = typename details::option_nums<T, Ss...>::type;

/**
 * @brief Extracts a floating-point numeric option from a sequence of options.
 *
 * @tparam T Floating-point numeric option name.
 * @tparam dnum Default numerator of the value if the option is missing.
 * @tparam dden Default denumerator of the value if the option is missing.
 * @tparam Ss Sequence of options.
 */
template <template<intmax_t,intmax_t> class T, intmax_t dnum, intmax_t dden, typename... Ss>
constexpr double option_float = details::option_float<T,dnum,dden,Ss...>::value;

/**
 * @brief Extracts a type option from a sequence of options.
 *
 * @tparam T Type option name.
 * @tparam D Default type if the option is missing.
 * @tparam Ss Sequence of options.
 */
template <template<class> class T, typename D, typename... Ss>
using option_type = typename details::option_type<T,D,Ss...>::type;

/**
 * @brief Extracts a type or multi-type option from a sequence of options as a type sequence.
 *
 * @tparam T Multi-type option name.
 * @tparam Ss Sequence of options.
 */
template <template<class...> class T, typename... Ss>
using option_types = typename details::option_types<T, Ss...>::type;

/**
 * @brief Extracts a multi-type option from a sequence of options as a type sequence of type sequences.
 *
 * @tparam T Multi-type option name.
 * @tparam Ss Sequence of options.
 */
template <template<class...> class T, typename... Ss>
using option_multitypes = typename details::option_multitypes<T, Ss...>::type;


//! @cond INTERNAL
namespace details {
    //! @brief Decays a function call type to its return type (general form).
    template <typename T>
    struct call_decay;

    //! @brief Decays a function call type to its return type.
    template <typename T>
    struct call_decay<T()> {
        using type = T;
    };

    //! @brief Decays a type into a type sequence (type not convertible to type sequence).
    template <typename T, typename = void>
    struct type_sequence_if_possible_impl {
        using type = typename call_decay<T>::type;
    };

    //! @brief Decays a type into a type sequence (type is convertible to type sequence).
    template <typename T>
    struct type_sequence_if_possible_impl<T(), decltype(void(type_sequence_identity(std::declval<T>())))> {
        using type = decltype(type_sequence_identity(std::declval<T>()));
    };

    //! @brief Decays a type into a type sequence if possible.
    template <typename T>
    using type_sequence_if_possible = typename type_sequence_if_possible_impl<T()>::type;

    //! @brief General form.
    template <typename... Ts>
    struct export_list {
        using type = type_sequence<>;
    };
    //! @brief Type argument.
    template <typename T, typename... Ts>
    struct export_list<T,Ts...> : public type_unite<common::type_sequence<T>, typename export_list<Ts...>::type> {};
    //! @brief Type sequence argument.
    template <typename... Ts, typename... Ss>
    struct export_list<type_sequence<Ts...>,Ss...> : public export_list<type_sequence_if_possible<Ts>...,Ss...> {};
}
//! @endcond

//! @brief Merges export lists and types into a single type sequence.
template <typename... Ts>
using export_list = typename details::export_list<details::type_sequence_if_possible<Ts>...>::type;


//! @cond INTERNAL
namespace details {
    //! @brief General form.
    template <typename... Ts>
    struct storage_list {
        using type = type_sequence<>;
    };
    //! @brief Type argument.
    template <typename S, typename T, typename... Ts>
    struct storage_list<S,T,Ts...> {
        using tmp = typename storage_list<Ts...>::type;
        using type = std::conditional_t<tmp::template slice<0, -1, 2>::template count<S> == 0, typename tmp::template push_front<S,T>, tmp>;
    };
    //! @brief Single type sequence argument.
    template <typename... Ts>
    struct storage_list<type_sequence<Ts...>> : public storage_list<type_sequence_if_possible<Ts>...> {};
    //! @brief Type sequence argument.
    template <typename... Ts, typename S, typename... Ss>
    struct storage_list<type_sequence<Ts...>,S,Ss...> : public storage_list<type_sequence_if_possible<Ts>...,S,Ss...> {};
}
//! @endcond

//! @brief Merges storage lists and types into a single type sequence.
template <typename... Ts>
using storage_list = typename details::storage_list<details::type_sequence_if_possible<Ts>...>::type;


// TYPE REPRESENTATION


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


//! @cond INTERNAL
namespace details {
    //! @brief Strips the namespaces from a non-templated type.
    inline std::string strip_namespaces_type(std::string s) {
        size_t pos = s.rfind("::");
        if (pos == std::string::npos) return s;
        return s.substr(pos+2);
    }
}
//! @endcond

//! @brief Removes the namespaces from a type representation.
std::string strip_namespaces(std::string s);


//! @brief Escapes a value for clear printing.
//! @{

//! @brief Escapes a boolean value for clear printing.
inline std::string escape(bool x) {
    return x ? "true" : "false";
}
//! @brief Escapes a char value for clear printing.
inline std::string escape(char x) {
    if (x < 32 or x > 126) {
        int y = x < 0 ? x + 256 : x;
        return "'\\" + std::to_string(y/64) + std::to_string((y/8)%8) + std::to_string(y%8) + "'";
    }
    if (x == '\'') return "'\\''";
    return std::string("'") + x + "'";
}
//! @brief Escapes a signed byte value for clear printing.
inline int escape(int8_t x) {
    return x;
}
//! @brief Escapes an unsigned byte value for clear printing.
inline int escape(uint8_t x) {
    return x;
}
//! @brief Escapes a string value for clear printing.
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
//! @brief Escapes a C-string value for clear printing.
inline std::string escape(char const* x) {
    return escape(std::string(x));
}
//! @brief Escapes a empty value for clear printing.
template <typename T, typename = std::enable_if_t<std::is_empty<T>::value>>
inline std::string escape(T) {
    return type_name<T>();
}
//! @brief Escapes any other value for clear printing.
template <typename T, typename = std::enable_if_t<type_count<T, bool, char, int8_t, uint8_t, std::string, char const*> == 0 and not std::is_empty<T>::value>>
inline T const& escape(T const& x) {
    return x;
}
//! @}


// COMPILER ERROR HANDLING


/**
 *  @brief Struct wrapping a false value.
 *
 *  Useful to deprecate templated function, in order to produce compile errors with
 *  `static_assert` whenever they are instantiated.
 */
template <typename... T>
struct always_false : public std::false_type {};

/**
 * @brief Returns a reference to a given type.
 *
 * Useful paired with `static_assert`, to suppress as many spurious error messages as possible.
 * Produces runtime errors if actually used.
 */
template <typename T>
T& declare_reference() {
    assert(false);
    return *reinterpret_cast<std::decay_t<T>*>(42);
}

/**
 * @brief Wildcard struct allowing arbitrary conversions.
 *
 * Useful paired with `static_assert`, to suppress as many spurious error messages as possible.
 * Produces runtime errors if actually used.
 */
struct wildcard {
    //! @brief Generic constructor.
    template <typename T>
    wildcard(T&&) {
        assert(false);
    }

    //! @brief Generic assignment.
    template <typename T>
    wildcard& operator=(T&&) {
        assert(false);
        return *this;
    }

    //! @brief Generic conversion.
    template <typename T>
    operator T() const {
        return declare_reference<T>();
    }
};


} // namespace common


//! @brief Allows usage of export_list in main namespace.
using common::export_list;

//! @brief Allows usage of storage_list in main namespace.
using common::storage_list;


} // namespace fcpp

#endif  // FCPP_COMMON_TRAITS_H_
