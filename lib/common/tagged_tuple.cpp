// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <string>
#include <unordered_set>

#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @cond INTERNAL
namespace details {
    //! @brief Set of type names to be skipped.
    std::unordered_set<std::string> skip_tag_set;

    //! @brief Separator between tags and corresponding values.
    std::string tag_val_sep = ":";

    //! @brief Separator between a value and the following tag.
    std::string val_tag_sep = ", ";

    //! @brief Accesses the set of type names to be skipped.
    std::unordered_set<std::string>& get_skip_tags() {
        return skip_tag_set;
    }

    //! @brief Accesses the separator between tags and corresponding values.
    const std::string& get_tag_val_sep() {
        return tag_val_sep;
    }

    //! @brief Accesses the separator between a value and the following tag.
    const std::string& get_val_tag_sep() {
        return val_tag_sep;
    }
}
//! @endcond


//! @brief Stream manipulator for representing tagged tuples in dictionary format (default).
std::ostream& operator<<(std::ostream& o, const dictionary_tuple_t&) {
    details::tag_val_sep = ":";
    details::val_tag_sep = ", ";
    return o;
}

//! @brief Stream manipulator for representing tagged tuples in assignment-list format.
std::ostream& operator<<(std::ostream& o, const assignment_tuple_t&) {
    details::tag_val_sep = " = ";
    details::val_tag_sep = ", ";
    return o;
}

//! @brief Stream manipulator for representing tagged tuples in compact underscore format.
std::ostream& operator<<(std::ostream& o, const underscore_tuple_t&) {
    details::tag_val_sep = "-";
    details::val_tag_sep = "_";
    return o;
}


}
