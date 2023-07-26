// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {

std::string strip_namespaces(std::string s) {
    size_t tstart = s.find("<");
    if (tstart == std::string::npos) return details::strip_namespaces_type(s);
    std::string q = details::strip_namespaces_type(s.substr(0,tstart+1));
    size_t tend = s.rfind(">");
    size_t tcomma = tstart;
    while (tcomma < tend) {
        size_t next = std::min(s.find(",", tcomma+1), tend);
        q += strip_namespaces(s.substr(tcomma+1, next-tcomma));
        tcomma = next;
    }
    q += s.substr(tend+1);
    return q;
}

} // namespace common


} // namespace fcpp
