// Copyright © 2024 Giorgio Audrito. All Rights Reserved.

/**
 * @file slcs.hpp
 * @brief Implementation of SLCS spatial logic operators.
 */

#ifndef FCPP_COORDINATION_SLCS_H_
#define FCPP_COORDINATION_SLCS_H_

#include "lib/beautify.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/coordination/spreading.hpp"

#ifndef FCPP_DIAMETER
//! @brief Setting defining an upper bound to the diameter of the network.
#define FCPP_DIAMETER 20
#endif


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
    //! @brief Upper bound to the node diameter.
    struct diameter {};
}

//! @brief Exports for SLCS logic formulas.
using slcs_t = export_list<bool, abf_hops_t>;

//! @brief Namespace containing logical operators and formulas.
namespace logic {

//! @brief Interior of a region.
FUN bool I(ARGS, bool f) { CODE
    return all_hood(CALL, nbr(CALL, true, f));
}

//! @brief Closure of a region.
FUN bool C(ARGS, bool f) { CODE
    return any_hood(CALL, nbr(CALL, false, f));
}

//! @brief Boundary of a region.
FUN bool B(ARGS, bool f) { CODE
    return C(CALL, f) & !I(CALL, f);
}

//! @brief Interior boundary of a region.
FUN bool IB(ARGS, bool f) { CODE
    return f & !I(CALL, f);
}

//! @brief Closure boundary of a region.
FUN bool CB(ARGS, bool f) { CODE
    return C(CALL, f) & !f;
}

//! @brief Finally/somewhere operator.
FUN bool F(ARGS, bool f) { CODE
    return abf_hops(CALL, f) < common::get_or<tags::diameter>(node.storage_tuple(), FCPP_DIAMETER);
}

//! @brief Globally/everywhere operator.
FUN bool G(ARGS, bool f) { CODE
    return !F(CALL, !f);
}

//! @brief Reaches operator.
FUN bool R(ARGS, bool f1, bool f2) { CODE
    return f1 ? F(CALL, f2) : false;
}

//! @brief Touches operator.
FUN bool T(ARGS, bool f1, bool f2) { CODE
    return R(CALL, f1, C(CALL, f2));
}

//! @brief Until/surrounding operator.
FUN bool U(ARGS, bool f1, bool f2) { CODE
    return f1 & I(CALL, !R(CALL, !f2, !f1));
}

} // logic

} // coordination

} // fcpp

#endif // FCPP_COORDINATION_SLCS_H_
