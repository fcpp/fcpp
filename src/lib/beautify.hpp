// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file beautify.hpp
 * @brief Header defining macros for cleaning up FCPP code.
 */

#ifndef FCPP_BEAUTIFY_H_
#define FCPP_BEAUTIFY_H_


//! @cond INTERNAL
#define __TYPE_ARG__(T) typename T

#define __MAPPER0__(M)
#define __MAPPER1__(M,A)                          M(A)
#define __MAPPER2__(M,A,B)                        __MAPPER1__(M,A), __MAPPER1__(M,B)
#define __MAPPER3__(M,A,B,C)                      __MAPPER1__(M,A), __MAPPER2__(M,B,C)
#define __MAPPER4__(M,A,B,C,D)                    __MAPPER1__(M,A), __MAPPER3__(M,B,C,D)
#define __MAPPER5__(M,A,B,C,D,E)                  __MAPPER1__(M,A), __MAPPER4__(M,B,C,D,E)
#define __MAPPER6__(M,A,B,C,D,E,F)                __MAPPER1__(M,A), __MAPPER5__(M,B,C,D,E,F)
#define __MAPPER7__(M,A,B,C,D,E,F,G)              __MAPPER1__(M,A), __MAPPER6__(M,B,C,D,E,F,G)
#define __MAPPER8__(M,A,B,C,D,E,F,G,H)            __MAPPER1__(M,A), __MAPPER7__(M,B,C,D,E,F,G,H)
#define __MAPPER9__(M,A,B,C,D,E,F,G,H,I)          __MAPPER1__(M,A), __MAPPER8__(M,B,C,D,E,F,G,H,I)
#define __MAPPERX__(M,A,B,C,D,E,F,G,H,I,X,...)    X
//! @endcond

//! @brief Maps a macro to a variable number of arguments (up to 9), comma separating the calls.
#define MACRO_MAPPER(...)                         __MAPPERX__(__VA_ARGS__, __MAPPER9__, __MAPPER8__, \
                                                              __MAPPER7__, __MAPPER6__, __MAPPER5__, \
                                                              __MAPPER4__, __MAPPER3__, __MAPPER2__, \
                                                              __MAPPER1__, __MAPPER0__)(__VA_ARGS__)

//! @brief Macro defining a non-generic aggregate function.
#define FUN             template <typename node_t>

//! @brief Macro defining the type arguments of an aggregate function.
#define GEN(...)        template <MACRO_MAPPER(__TYPE_ARG__, node_t, __VA_ARGS__)>

//! @brief Bounds generic function type F to comply with signature T.
#define BOUND(F, T)     = common::type_unwrap<void(common::type_sequence<common::if_signature<F, T>>)>

//! @brief Macro inserting the default arguments.
#define ARGS            node_t& node, trace_t call_point

//! @brief Macro inserting the default arguments at function call.
#define CALL            node, __COUNTER__

//! @brief Macro inserting the default code at function start.
#define CODE            internal::trace_call trace_caller(node.stack_trace, call_point);

//! @brief Macro defining a non-generic aggregate function.
#define FUN_EXPORT      using

//! @brief Macro defining the type arguments of an aggregate function.
#define GEN_EXPORT(...) template <MACRO_MAPPER(__TYPE_ARG__, __VA_ARGS__)> using

//! @brief Macro defining the index of an aggregate for loop.
#define LOOP(v, s)      internal::trace_cycle v{node.stack_trace, trace_t(s)}

/**
 * @brief Macro for defining a main class to be used in the calculus component. Usage:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * MAIN() {
 *   ...
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * The code of the main function has access to the `node` object.
 */
#define MAIN()                                  \
struct main {                                   \
    template <typename node_t>                  \
    void operator()(node_t&, times_t);          \
};                                              \
template <typename node_t>                      \
void main::operator()(node_t& node, times_t)


#endif // FCPP_BEAUTIFY_H_
