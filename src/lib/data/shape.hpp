// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file shape.hpp
 * @brief Implementation of the `shape` enum and helper functions.
 */

#ifndef FCPP_DATA_SHAPE_H_
#define FCPP_DATA_SHAPE_H_

#include <string>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Supported shapes for representing nodes.
enum class shape : char { tetrahedron, cube, octahedron, icosahedron, sphere, star, SIZE };

//! @brief String representation of a shape.
std::string to_string(shape);

//! @brief Printing shapes.
template <typename O>
O& operator<<(O& o, shape s) {
    o << to_string(s);
    return o;
}


}

#endif // FCPP_DATA_SHAPE_H_
