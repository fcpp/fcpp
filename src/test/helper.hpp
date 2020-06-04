// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file helper.hpp
 * @brief Bundle including helper functions for testing.
 */

#ifndef FCPP_HELPER_H_
#define FCPP_HELPER_H_

#include <iostream>

#include "gtest/gtest.h"

#include "lib/beautify.hpp"
#include "lib/common/ostream.hpp"
#include "lib/common/traits.hpp"


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
        EXPECT_TRUE(b) << "\033[4A" << file << ":" << line << ": Failure\n"
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

#endif // FCPP_HELPER_H_
