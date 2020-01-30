// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file algorithm.hpp
 * @brief Implementation of common algorithms useful in the library.
 */

#ifndef FCPP_COMMON_ALGORITHM_H_
#define FCPP_COMMON_ALGORITHM_H_

#include <algorithm>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Sort a sequence just enough to fix a set of particular positions.
 * Assumes that the index list is sorted in increasing order, and all indexes are within range 0 .. (elast-efirst-1).
 *
 * @param efirst Random-access iterator to the first element.
 * @param elast  Random-access iterator after the last element.
 * @param ifirst Random-access iterator to the first index.
 * @param ilast  Random-access iterator after the last index.
 * @param offs   An offset to be applied to all indexes.
 */
template <typename E, typename I>
void nth_elements(E efirst, E elast, I ifirst, I ilast, size_t offs = 0) {
    if (ifirst == ilast) return;
    I imid = ifirst + (ilast-ifirst)/2;
    E emid = efirst + (*imid-offs);
    nth_element (efirst, emid,  elast);
    nth_elements(efirst, emid,  ifirst, imid,  offs);
    nth_elements(emid+1, elast, imid+1, ilast, *imid+1);
}


/**
 * @brief Sort a sequence just enough to fix a set of particular positions using a predicate for comparison.
 * Assumes that the index list is sorted in increasing order, and all indexes are within range 0 .. (elast-efirst-1).
 *
 * @param efirst Random-access iterator to the first element.
 * @param elast  Random-access iterator after the last element.
 * @param ifirst Random-access iterator to the first index.
 * @param ilast  Random-access iterator after the last index.
 * @param comp   A comparison functor.
 * @param offs   An offset to be applied to all indexes.
 */
template <typename E, typename I, typename C>
void nth_elements(E efirst, E elast, I ifirst, I ilast, C comp, size_t offs = 0) {
    if (ifirst == ilast) return;
    I imid = ifirst + (ilast-ifirst)/2;
    E emid = efirst + (*imid-offs);
    nth_element (efirst, emid,  elast,  comp);
    nth_elements(efirst, emid,  ifirst, imid,  comp, offs);
    nth_elements(emid+1, elast, imid+1, ilast, comp, *imid+1);
}


}

#endif // FCPP_COMMON_ALGORITHM_H_
