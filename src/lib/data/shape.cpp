// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "lib/data/shape.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief String representation of a shape.
std::string to_string(shape s) {
    switch (s) {
        case shape::tetrahedron:
            return "tetrahedron";

        case shape::cube:
            return "cube";

        case shape::octahedron:
            return "octahedron";

        case shape::icosahedron:
            return "icosahedron";

        case shape::sphere:
            return "sphere";

        case shape::star:
            return "star";

        default:
            return "shape";
    }
}

}
