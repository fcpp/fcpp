// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "lib/cloud/graph_spawner.hpp"

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {

//! @cond INTERNAL
namespace details {
    //! @brief Makes an istream reference from a `std::string` path.
    std::shared_ptr<std::istream> make_istream(std::string const& s) {
        return std::shared_ptr<std::istream>(new std::ifstream(s));
    }
    //! @brief Makes an istream reference from a `const char*` path.
    std::shared_ptr<std::istream> make_istream(const char* s) {
        return make_istream(std::string(s));
    }
    //! @brief Makes an istream reference from a stream pointer.
    std::shared_ptr<std::istream> make_istream(std::istream* i) {
        return std::shared_ptr<std::istream>(i, [] (void*) {});
    }
}

}
}
