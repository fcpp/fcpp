// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file beautify.hpp
 * @brief Header defining macros for cleaning up FCPP code.
 */

#ifndef FCPP_BEAUTIFY_H_
#define FCPP_BEAUTIFY_H_


//! @cond INTERNAL
#define MACRO_MAPPER0(M)
#define MACRO_MAPPER1(M,A)                          M(A)
#define MACRO_MAPPER2(M,A,B)                        MACRO_MAPPER1(M,A), MACRO_MAPPER1(M,B)
#define MACRO_MAPPER3(M,A,B,C)                      MACRO_MAPPER1(M,A), MACRO_MAPPER2(M,B,C)
#define MACRO_MAPPER4(M,A,B,C,D)                    MACRO_MAPPER1(M,A), MACRO_MAPPER3(M,B,C,D)
#define MACRO_MAPPER5(M,A,B,C,D,E)                  MACRO_MAPPER1(M,A), MACRO_MAPPER4(M,B,C,D,E)
#define MACRO_MAPPER6(M,A,B,C,D,E,F)                MACRO_MAPPER1(M,A), MACRO_MAPPER5(M,B,C,D,E,F)
#define MACRO_MAPPER7(M,A,B,C,D,E,F,G)              MACRO_MAPPER1(M,A), MACRO_MAPPER6(M,B,C,D,E,F,G)
#define MACRO_MAPPER8(M,A,B,C,D,E,F,G,H)            MACRO_MAPPER1(M,A), MACRO_MAPPER7(M,B,C,D,E,F,G,H)
#define MACRO_MAPPER9(M,A,B,C,D,E,F,G,H,I)          MACRO_MAPPER1(M,A), MACRO_MAPPER8(M,B,C,D,E,F,G,H,I)
#define MACRO_MAPPERX(M,A,B,C,D,E,F,G,H,I,X,...)    X
//! @endcond

//! @brief Maps a macro to a variable number of arguments (up to 9), comma separating the calls.
#define MACRO_MAPPER(...)                           MACRO_MAPPERX(__VA_ARGS__, MACRO_MAPPER9, \
                                                        MACRO_MAPPER8, MACRO_MAPPER7, MACRO_MAPPER6, \
                                                        MACRO_MAPPER5, MACRO_MAPPER4, MACRO_MAPPER3, \
                                                        MACRO_MAPPER2, MACRO_MAPPER1, MACRO_MAPPER0, *)(__VA_ARGS__)

//! @brief Internal macro producing a typename declaration for a type variable.
#define __TYPE_ARG__(T) typename T

//! @brief Macro defining the type arguments of a function.
#define FUN(...)        template <MACRO_MAPPER(__TYPE_ARG__, node_t __VA_OPT__(,) __VA_ARGS__)>

//! @brief Macro inserting the default arguments.
#define ARGS            node_t& node, trace_t call_point

//! @brief Macro inserting the default arguments at function call.
#define CALL            node, __COUNTER__

//! @brief Macro inserting the default code at function start.
#define CODE            internal::trace_call trace_caller(node.stack_trace, call_point);

/**
 * @brief Macro for defining a main class to be used in the calculus component.
 *
 * The function to be called by the main class is given as the first argument `f`.
 * The second argument `t` is a variable name for the `times_t` time of function call
 * (leave empty if not used).
 * Further arguments to be provided to `f` follow (possibly using `t`).
 *
 */
#define MAIN(f, t, ...)                         \
struct main {                                   \
    template <typename node_t>                  \
    void operator()(node_t& node, times_t t) {  \
        f(node, 0 __VA_OPT__(,) __VA_ARGS__);   \
    }                                           \
}


#endif // FCPP_BEAUTIFY_H_
