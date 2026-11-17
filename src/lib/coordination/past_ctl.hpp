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
GEN_EXPORT(T) bounded_past_ctl_t = export_list<bool,T>;

//! @brief Namespace containing logical operators and formulas.
namespace logic {

//! @brief Support function for bounded past operator
template <typename T>
T mix(bool f1, bool f2, T ol) {
  if (f2)
    return 0;
  else {
    if (f1)
      return ol;
    else
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
GEN(T) bool S(ARGS, T k, field<T> metric, bool f1, bool f2) { CODE
    return nbr(CALL, std::numeric_limits<T>::max(), [&](field<T> l) {
        T value = self(CALL, l);
        T unit = self(CALL, metric);
        // Check to handle overflow
        if (value < std::numeric_limits<T>::max() - unit)
          value += unit;
        else
          value = std::numeric_limits<T>::max();
        return mix<T>(f1, f2, value);
        }) < k;
}

//! @brief f1 holds since f2 held in all devices.
FUN bool AS(ARGS, bool f1, bool f2) { CODE
    return nbr(CALL, false, [&](field<bool> n) -> bool {
        return f2 | (f1 & all_hood(CALL, n));
    });
}

//! @brief f1 holds since f2 held in any device.
FUN bool ES(ARGS, bool f1, bool f2) { CODE
    return nbr(CALL, false, [&](field<bool> n) -> bool {
        return f2 | (f1 & any_hood(CALL, n));
    });
}

//! @brief Previously in the same device.
FUN bool P(ARGS, bool f) { CODE
    return old(CALL, false, [&](bool o) -> bool {
        return f | o;
    });
}

//! @brief Previously in all devices.
FUN bool AP(ARGS, bool f) { CODE
    return nbr(CALL, false, [&](field<bool> n) -> bool {
        return f | all_hood(CALL, n);
    });
}

//! @brief Previously in any device.
FUN bool EP(ARGS, bool f) { CODE
    return nbr(CALL, false, [&](field<bool> n) -> bool {
        return f | any_hood(CALL, n);
    });
}

//! @brief Historically in the same device.
FUN bool H(ARGS, bool f) { CODE
    return old(CALL, true, [&](bool o) -> bool {
        return f & o;
    });
}

//! @brief Historically in all devices.
FUN bool AH(ARGS, bool f) { CODE
    return nbr(CALL, true, [&](field<bool> n) -> bool {
        return f & all_hood(CALL, n);
    });
}

//! @brief Historically in any device.
FUN bool EH(ARGS, bool f) { CODE
    return nbr(CALL, true, [&](field<bool> n) -> bool {
        return f & any_hood(CALL, n);
    });
}

} // logic

} // coordination

} // fcpp

#endif // FCPP_COORDINATION_PAST_CTL_H_
