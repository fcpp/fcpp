// Copyright © 2024 Giorgio Audrito. All Rights Reserved.

/**
 * @file past_ctl.hpp
 * @brief Implementation of Past-CTL temporal logic operators.
 */

#ifndef FCPP_COORDINATION_PAST_CTL_H_
#define FCPP_COORDINATION_PAST_CTL_H_

#include <limits>
#include "lib/beautify.hpp"
#include "lib/coordination/utils.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Exports for Past-CTL logic formulas.
using past_ctl_t = export_list<bool>;

//! @brief Namespace containing logical operators and formulas.
namespace logic {

namespace details {

//! @brief Support function for bounded past operator
template <typename T> T mix(bool f1, bool f2, T ol) {
    if (f2) return 0;
    if (f1) return ol;
    return std::numeric_limits<T>::max();
}

}

//! @brief Yesterday in the same device.
FUN bool Y(ARGS, bool f) { CODE
    return old(CALL, false, f);
}

//! @brief Yesterday in all devices.
FUN bool AY(ARGS, bool f) { CODE
    return all_hood(CALL, nbr(CALL, true, f));
}

//! @brief Yesterday in some device.
FUN bool EY(ARGS, bool f) { CODE
    return any_hood(CALL, nbr(CALL, false, f));
}

//! @brief f1 holds since f2 held in the same device.
FUN bool S(ARGS, bool f1, bool f2) { CODE
    return old(CALL, false, [&](bool o) -> bool {
        return f2 | (f1 & o);
    });
}

//! @brief f1 holds since f2 held in the same device up to k metric steps in the past.
FUN bool S(ARGS, int k, int metric, bool f1, bool f2) { CODE
    return old(CALL, std::numeric_limits<int>::max(), [&](int value) -> int {
               value += (value < std::numeric_limits<int>::max()) ? metric : 0;
               return details::mix(f1, f2, value);
           }) < k;
}

FUN bool S(ARGS, times_t k, times_t metric, bool f1, bool f2) { CODE
    return old(CALL, std::numeric_limits<times_t>::max(), [&](times_t value) -> times_t {
               value += (value < std::numeric_limits<times_t>::max()) ? metric : 0;
               return details::mix(f1, f2, value);
           }) < k;
}

//! @brief f1 holds since f2 held in all devices.
FUN bool AS(ARGS, bool f1, bool f2) { CODE
    return nbr(CALL, false, [&](field<bool> n) -> bool {
        return f2 | (f1 & all_hood(CALL, n));
    });
}

//! @brief f1 holds since f2 held in all devices up to k metric steps in the past.
FUN bool AS(ARGS, int k, field<int> const& metric, bool f1, bool f2) { CODE
    return nbr(CALL, std::numeric_limits<int>::max(), [&](field<int> l) -> int {
               field<int> update = map_hood(
                   [&](int value, int unit) {
                       value += (value < std::numeric_limits<int>::max()) ? unit : 0;
                       return value;
                   }, l, metric);
               return details::mix(f1, f2, max_hood(CALL, update));
           }) < k;
}

FUN bool AS(ARGS, times_t k, field<times_t> const& metric, bool f1, bool f2) { CODE
    return nbr(CALL, std::numeric_limits<times_t>::max(), [&](field<times_t> l) -> times_t {
               field<times_t> update = map_hood(
                   [&](times_t value, times_t unit) {
                       value += (value < std::numeric_limits<times_t>::max()) ? unit : 0;
                       return value;
                   }, l, metric);
               return details::mix(f1, f2, max_hood(CALL, update));
           }) < k;
}
//! @brief f1 holds since f2 held in any device.
FUN bool ES(ARGS, bool f1, bool f2) { CODE
    return nbr(CALL, false, [&](field<bool> n) -> bool {
        return f2 | (f1 & any_hood(CALL, n));
    });
}

//! @brief f1 holds since f2 held in at least one device up to k metric steps in the past.
FUN bool ES(ARGS, int k, field<int> const& metric, bool f1, bool f2) { CODE
    return nbr(CALL, std::numeric_limits<int>::max(), [&](field<int> l) -> int {
               field<int> update = map_hood(
                   [&](int value, int unit) {
                       value += (value < std::numeric_limits<int>::max()) ? unit : 0;
                       return value;
                   }, l, metric);
               return details::mix(f1, f2, min_hood(CALL, update));
           }) < k;
}
FUN bool ES(ARGS, times_t k, field<times_t> const& metric, bool f1, bool f2) { CODE
    return nbr(CALL, std::numeric_limits<times_t>::max(), [&](field<times_t> l) -> times_t {
               field<times_t> update = map_hood(
                   [&](times_t value, times_t unit) {
                       value += (value < std::numeric_limits<times_t>::max()) ? unit : 0;
                       return value;
                   }, l, metric);
               return details::mix(f1, f2, min_hood(CALL, update));
           }) < k;
}

//! @brief Previously in the same device.
FUN bool P(ARGS, bool f) { CODE
    return old(CALL, false, [&](bool o) -> bool {
        return f | o;
    });
}

FUN bool P(ARGS, int k, int metric, bool f) { CODE
  return S(CALL, k, metric, true, f);
}

FUN bool P(ARGS, times_t k, times_t metric, bool f) { CODE
  return S(CALL, k, metric, true, f);
}

//! @brief Previously in all devices.
FUN bool AP(ARGS, bool f) { CODE
    return nbr(CALL, false, [&](field<bool> n) -> bool {
        return f | all_hood(CALL, n);
    });
}

FUN bool AP(ARGS, int k, field<int> const& metric, bool f) { CODE
  return AS(CALL, k, metric, true, f);
}

FUN bool AP(ARGS, times_t k, field<times_t> const& metric, bool f) { CODE
  return AS(CALL, k, metric, true, f);
}

//! @brief Previously in any device.
FUN bool EP(ARGS, bool f) { CODE
    return nbr(CALL, false, [&](field<bool> n) -> bool {
        return f | any_hood(CALL, n);
    });
}

FUN bool EP(ARGS, int k, field<int> const& metric, bool f) { CODE
  return ES(CALL, k, metric, true, f);
}

FUN bool EP(ARGS, times_t k, field<times_t> const& metric, bool f) { CODE
  return ES(CALL, k, metric, true, f);
}

//! @brief Historically in the same device.
FUN bool H(ARGS, bool f) { CODE
    return old(CALL, true, [&](bool o) -> bool {
        return f & o;
    });
}

FUN bool H(ARGS, int k, int metric, bool f) { CODE
    return not P(CALL, k, metric, not f);
}

FUN bool H(ARGS, times_t k, times_t metric, bool f) { CODE
    return not P(CALL, k, metric, not f);
}

//! @brief Historically in all devices.
FUN bool AH(ARGS, bool f) { CODE
    return nbr(CALL, true, [&](field<bool> n) -> bool {
        return f & all_hood(CALL, n);
    });
}

FUN bool AH(ARGS, int k, field<int> const& metric, bool f) { CODE
    return not EP(CALL, k, metric, not f);
}
FUN bool AH(ARGS, times_t k, field<times_t> const& metric, bool f) { CODE
    return not EP(CALL, k, metric, not f);
}

//! @brief Historically in any device.
FUN bool EH(ARGS, bool f) { CODE
    return nbr(CALL, true, [&](field<bool> n) -> bool {
        return f & any_hood(CALL, n);
    });
}

FUN bool EH(ARGS, int k, field<int> const& metric, bool f) { CODE
    return not AP(CALL, k, metric, not f);
}

FUN bool EH(ARGS, times_t k, field<times_t> const& metric, bool f) { CODE
    return not AP(CALL, k, metric, not f);
}

} // logic

} // coordination

} // fcpp

#endif // FCPP_COORDINATION_PAST_CTL_H_
