// Copyright © 2026 Giorgio Audrito. All Rights Reserved.

#include "lib/cloud/graph_connector.hpp"

namespace fcpp {

namespace component {

//! @brief Converts a request_kind to its string representation.
std::string to_string(request_kind r) {
    switch (r) {
        case request_kind::CONNECT:
            return "CONNECT";
        case request_kind::NONE:
            return "NONE";
        case request_kind::DISCONNECT:
            return "DISCONNECT";
        case request_kind::BIDISCONNECT:
            return "BIDISCONNECT";
    }
    return "?";
}


} // namespace component

} // namespace fcpp
