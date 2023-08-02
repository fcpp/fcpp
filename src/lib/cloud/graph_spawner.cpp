// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "lib/cloud/graph_spawner.hpp"

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {

//! @cond INTERNAL
namespace details {
    //! @brief Makes an istream reference from a `std::string` path.
    std::shared_ptr<std::istream> make_istream(std::string const& s) {
        return std::shared_ptr<std::istream>(new std::ifstream(s));
    }
    //! @brief Makes an istream reference from a `char const*` path.
    std::shared_ptr<std::istream> make_istream(char const* s) {
        return make_istream(std::string(s));
    }
    //! @brief Makes an istream reference from a stream pointer.
    std::shared_ptr<std::istream> make_istream(std::istream* i) {
        return std::shared_ptr<std::istream>(i, [] (void*) {});
    }
}

}
}
