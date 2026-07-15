// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file helper.hpp
 * @brief Bundle including helper functions for testing.
 */

#ifndef FCPP_HELPER_H_
#define FCPP_HELPER_H_

#include <iostream>

#include "gtest/gtest.h"

#include "lib/common/ostream.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @cond INTERNAL
namespace details {
    //! @brief Checks the equality between two types (as a function).
    template <typename T, typename U>
    void expect_same(std::string file, int line) {
        bool b = std::is_same<T, U>::value;
        EXPECT_TRUE(b) << "\u001b[4A" << file << ":" << line << ": Failure\n"
                       << "      Expected: " << common::type_name<T>() << "\n"
                       << "To be equal to: " << common::type_name<U>();
    }
}
//! @endcond

}

//! @brief Checks the equality between two types (macro).
#define EXPECT_SAME(...) fcpp::details::expect_same<__VA_ARGS__>(__FILE__, __LINE__)

//! @brief Escapes a macro argument containing commas.
#define ESCAPE(...) __VA_ARGS__

//! @brief Prints the value of a variable for debugging purposes.
#define PRINT_VAR(x)                                std::cerr << __FILE__ << ":" << __LINE__ << ": " \
                                                              << common::type_name<decltype(x)>() << " " << #x \
                                                              << " = " << x << std::endl

//! @brief Prints the values of multiple variables for debugging purposes.
#define PRINT_VARS(...)                             MACRO_MAPPER(PRINT_VAR, __VA_ARGS__)

//! @cond INTERNAL
#define MULTI_TEST_0(suite, test, prefix)        \
        TEST(suite, test ## prefix) {            \
            suite ## test<0b ## prefix>();       \
        }
#define MULTI_TEST_1(suite, test, prefix)        \
        MULTI_TEST_0(suite, test, 0 ## prefix)   \
        MULTI_TEST_0(suite, test, 1 ## prefix)
#define MULTI_TEST_2(suite, test, prefix)        \
        MULTI_TEST_1(suite, test, 0 ## prefix)   \
        MULTI_TEST_1(suite, test, 1 ## prefix)
#define MULTI_TEST_3(suite, test, prefix)        \
        MULTI_TEST_2(suite, test, 0 ## prefix)   \
        MULTI_TEST_2(suite, test, 1 ## prefix)
#define MULTI_TEST_4(suite, test, prefix)        \
        MULTI_TEST_3(suite, test, 0 ## prefix)   \
        MULTI_TEST_3(suite, test, 1 ## prefix)
#define MULTI_TEST_5(suite, test, prefix)        \
        MULTI_TEST_4(suite, test, 0 ## prefix)   \
        MULTI_TEST_4(suite, test, 1 ## prefix)
#define MULTI_TEST_6(suite, test, prefix)        \
        MULTI_TEST_5(suite, test, 0 ## prefix)   \
        MULTI_TEST_5(suite, test, 1 ## prefix)
#define MULTI_TEST_7(suite, test, prefix)        \
        MULTI_TEST_6(suite, test, 0 ## prefix)   \
        MULTI_TEST_6(suite, test, 1 ## prefix)
#define MULTI_TEST_8(suite, test, prefix)        \
        MULTI_TEST_7(suite, test, 0 ## prefix)   \
        MULTI_TEST_7(suite, test, 1 ## prefix)
#define MULTI_TEST_9(suite, test, prefix)        \
        MULTI_TEST_8(suite, test, 0 ## prefix)   \
        MULTI_TEST_8(suite, test, 1 ## prefix)
//! @endcond

/**
 * @brief Generates multiple google tests.
 * @param suite The test suite name.
 * @param test The test name.
 * @param param The template parameter to vary across test instances.
 * @param length The maximum length in bits of the parameter considered.
 */
#define MULTI_TEST(suite, test, param, length)      \
        template <int param> void suite ## test();  \
        MULTI_TEST_ ## length(suite, test,)         \
        template <int param> void suite ## test()

/**
 * @brief Generates multiple google tests with (non-parametric) fixture.
 * @param suite The test suite name.
 * @param test The test name.
 * @param param The template parameter to vary across test instances.
 * @param length The maximum length in bits of the parameter considered.
 */
#define MULTI_TEST_F(suite, test, param, length)            \
        template <int param>                                \
        struct suite ## test ## C : public suite {   \
            using suite::SetUp;                      \
            virtual void TestBody();                        \
        };                                                  \
        template <int param> void suite ## test() {         \
            suite ## test ## C<param> t;                    \
            t.SetUp();                                      \
            t.TestBody();                                   \
        }                                                   \
        MULTI_TEST_ ## length(suite, test,)                 \
        template <int param>                                \
        void suite ## test ## C<param>::TestBody()

#endif // FCPP_HELPER_H_
