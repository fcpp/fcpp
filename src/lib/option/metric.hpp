// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file metric.hpp
 * @brief Classes realising metric predicates between messages.
 */

#ifndef FCPP_OPTION_METRIC_H_
#define FCPP_OPTION_METRIC_H_

#include <limits>

#include "lib/settings.hpp"
#include "lib/common/option.hpp"
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

    //! @brief Constructor.
    template <typename S, typename T>
    once(common::tagged_tuple<S,T> const&) {}

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
 * @param period_tag An initialisation tag from which a period can be read.
 */
template <intmax_t period = 1, intmax_t scale = 1, typename period_tag = void>
struct retain {
    //! @brief Whether a tag has been provided.
    constexpr static bool has_tag = not std::is_same<period_tag, void>::value;

  public:
    //! @brief The data type.
    using result_type = times_t;

    //! @brief Constructor.
    template <typename S, typename T>
    retain(common::tagged_tuple<S,T> const& t) : m_period(set_period(t, common::number_sequence<has_tag>{})) {}

    //! @brief Default threshold.
    result_type build() const {
        return get_period(common::number_sequence<has_tag>{});
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

  private:
    //! @brief Initialises the period from tagged tuple values (empty overload).
    template <typename S, typename T>
    inline static common::option<result_type, has_tag> set_period(common::tagged_tuple<S,T> const& t, common::number_sequence<false>) {
        return {};
    }

    //! @brief Initialises the period from tagged tuple values (active overload).
    template <typename S, typename T>
    inline static result_type set_period(common::tagged_tuple<S,T> const& t, common::number_sequence<true>) {
        return common::get<period_tag>(t);
    }

    //! @brief Reads the period from template parameters.
    inline result_type get_period(common::number_sequence<false>) const {
        return period / result_type(scale);
    }

    //! @brief Reads the period from class data.
    inline result_type get_period(common::number_sequence<true>) const {
        return m_period;
    }

    //! @brief The period of time after which values are discarded.
    common::option<result_type, has_tag> m_period;
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

    //! @brief Constructor.
    template <typename S, typename T>
    minkowski(common::tagged_tuple<S,T> const&) {}

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
