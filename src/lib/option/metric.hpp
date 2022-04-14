// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file metric.hpp
 * @brief Classes realising metric predicates between messages.
 */

#ifndef FCPP_OPTION_METRIC_H_
#define FCPP_OPTION_METRIC_H_

#include <limits>

#include "lib/settings.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for metrics between messages.
namespace metric {


/**
 * @brief Metric predicate which clears out everything every round.
 */
struct once {
    //! @brief The data type.
    using result_type = char;

    //! @brief Default threshold.
    result_type build() const {
        return 1;
    }

    //! @brief Measures an incoming message.
    template <typename N, typename S, typename T>
    result_type build(N const& n, times_t, device_t d, common::tagged_tuple<S,T> const&) const {
        return d == n.uid ? 0 : 1;
    }

    //! @brief Updates an existing measure.
    template <typename N>
    result_type update(result_type const& r, N const&) const {
        return r == 0 ? 0 : 2;
    }
};

/**
 * @brief Metric predicate which clears out values after a retain time.
 *
 * Requires nodes to have a `next_time()` and `current_time()` interface (as per the `timer` component).
 *
 * @param period The period of time after which values are discarded.
 * @param scale A scale by which `period` is divided.
 */
template <intmax_t period = 1, intmax_t scale = 1>
struct retain {
    //! @brief The data type.
    using result_type = times_t;

    //! @brief Default threshold.
    result_type build() const {
        return period / times_t(scale);
    }

    //! @brief Measures an incoming message.
    template <typename N, typename S, typename T>
    result_type build(N const& n, times_t t, device_t d, common::tagged_tuple<S,T> const&) const {
        return d == n.uid ? 0 : n.next_time() - t;
    }

    //! @brief Updates an existing measure.
    template <typename N>
    result_type update(result_type const& r, N const& n) const {
        return r == 0 ? 0 : r + n.next_time() - n.current_time();
    }
};

/**
 * @brief Metric predicate which clears out values based on space-time distance.
 *
 * The metric is tuned to equiparate a temporal distance of `period` with a spatial distance of `radius`.
 * Requires nodes to have a `next_time()`, `current_time()` and `position(t)` interface
 * (as per the `timer` and `physical_position` components).
 *
 * @param position_tag The tag storing position data in messages.
 * @param radius The maximum communication radius.
 * @param period The maximum temporal interval between rounds.
 * @param scale A scale by which `radius` and `period` are divided.
 *
 */
template <typename position_tag, intmax_t radius = 1, intmax_t period = 1, intmax_t scale = 1>
struct minkowski {
    //! @brief The data type.
    using result_type = real_t;

    //! @brief Default threshold.
    result_type build() const {
        return 2 * radius / real_t(scale);
    }

    //! @brief Measures an incoming message.
    template <typename N, typename S, typename T>
    result_type build(N const& n, times_t t, device_t d, common::tagged_tuple<S,T> const& m) const {
        return d == n.uid ? 0 : norm(common::get<position_tag>(m) - n.position(t)) + (n.next_time() - t) * radius / real_t(period);
    }

    //! @brief Updates an existing measure.
    template <typename N>
    result_type update(result_type const& r, N const& n) const {
        return r == 0 ? 0 : r + (n.next_time() - n.current_time()) * radius / real_t(period);
    }
};


}


}

#endif // FCPP_OPTION_METRIC_H_
