// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "lib/coordination/basics.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief String representation of a status.
std::string to_string(status s) {
    switch (s) {
        case status::terminated:
            return "terminated";

        case status::external_deprecated:
            return "external";

        case status::border:
            return "border";

        case status::internal:
            return "internal";

        case status::terminated_output:
            return "terminated_output";

        case status::external_output_deprecated:
            return "external_output";

        case status::border_output:
            return "border_output";

        case status::internal_output:
            return "internal_output";

        case status::output:
            return "output";

        default:
            return "status";
    }
}

} // fcpp
