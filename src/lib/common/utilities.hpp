// Copyright © 2026 Giorgio Audrito. All Rights Reserved.

/**
 * @file utilities.hpp
 * @brief Miscellaneous utility functions.
 */

#ifndef FCPP_COMMON_UTILITIES_H_
#define FCPP_COMMON_UTILITIES_H_

#include <fstream>
#include <memory>
#include <string>

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


//! @brief Makes an istream reference from a `std::string` path.
std::shared_ptr<std::istream> make_istream(std::string const& s);

//! @brief Makes an istream reference from a `char const*` path.
std::shared_ptr<std::istream> make_istream(char const* s);

//! @brief Makes an istream reference from a stream pointer.
std::shared_ptr<std::istream> make_istream(std::istream* i);


} // namespace common


} // namespace fcpp

#endif  // FCPP_COMMON_UTILITIES_H_
