// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "lib/data/bloom.hpp"

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

double bloom_error(size_t m, size_t bits, size_t elements) {
    return pow(1 - pow(1 - 1.0/bits, m*elements), m);
}

} // fcpp
