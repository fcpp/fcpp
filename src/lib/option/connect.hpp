// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file connect.hpp
 * @brief Connection predicates to be used with a \ref fcpp::component::simulated_connector "simulated_connector".
 */

#ifndef FCPP_OPTION_CONNECT_H_
#define FCPP_OPTION_CONNECT_H_

#include <random>

#include "lib/settings.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/data/vec.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {

//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Net initialisation tag associating to communication radius.
    struct radius;

    //! @brief Net initialisation tag associating to 50%-likely communication radius.
    struct half_radius;
}

}


//! @brief Namespace containing connection predicates.
namespace connect {


/**
 * Connection predicate which is true between any pair of devices.
 *
 * @param n   Dimensionality of the space (defaults to 2).
 */
template <size_t n = 2>
class clique {
  public:
    //! @brief Type for representing a position.
    using position_type = vec<n>;

    //! @brief The node data type.
    struct data_type {};

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    clique(G&&, const common::tagged_tuple<S,T>&) {}

    //! @brief The maximum radius of connection.
    real_t maximum_radius() const {
        return INF;
    }

    //! @brief Checks if connection is possible.
    template <typename G>
    bool operator()(G&&, const data_type&, const position_type&, const data_type&, const position_type&) const {
        return true;
    }
};


/**
 * Connection predicate which is true within a fixed radius (can be set through tag `radius`).
 *
 * @param num The numerator of the default value for the radius (defaults to 1).
 * @param den The denominator of the default value for the radius (defaults to 1).
 * @param n   Dimensionality of the space (defaults to 2).
 */
template <intmax_t num = 1, intmax_t den = 1, size_t n = 2>
class fixed {
  public:
    //! @brief Type for representing a position.
    using position_type = vec<n>;

    //! @brief The node data type.
    struct data_type {};

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    fixed(G&&, const common::tagged_tuple<S,T>& t) {
        m_radius = common::get_or<component::tags::radius>(t, ((real_t)num)/den);
    }

    //! @brief The maximum radius of connection.
    real_t maximum_radius() const {
        return m_radius;
    }

    //! @brief Checks if connection is possible.
    template <typename G>
    bool operator()(G&&, const data_type&, const position_type& position1, const data_type&, const position_type& position2) const {
        return norm(position1 - position2) <= m_radius;
    }

  private:
    //! @brief The connection radius.
    real_t m_radius;
};


/**
 * Connection predicate with likelyhood depending on radius (with tag `radius` setting maximum radius and `half_radius` setting half radius).
 *
 * @param r99 The numerator of the default value for the maximum radius (99% communication failure).
 * @param r50 The numerator of the default value for the half radius (50% communication failure).
 * @param den The denominator of the default values (defaults to 1).
 * @param n   Dimensionality of the space (defaults to 2).
 */
template <intmax_t r99, intmax_t r50, intmax_t den = 1, size_t n = 2>
class radial {
  public:
    //! @brief Type for representing a position.
    using position_type = vec<n>;

    //! @brief The node data type.
    struct data_type {};

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    radial(G&&, const common::tagged_tuple<S,T>& t) {
        m_r99 = common::get_or<component::tags::radius>(t, ((real_t)r99)/den);
        m_r50 = common::get_or<component::tags::half_radius>(t, ((real_t)r50)/den);
        m_k = log(6792093.0/29701) / (m_r99-m_r50);
    }

    //! @brief The maximum radius of connection.
    real_t maximum_radius() const {
        return m_r99;
    }

    //! @brief Checks if connection is possible.
    template <typename G>
    bool operator()(G&& gen, const data_type&, const position_type& position1, const data_type&, const position_type& position2) const {
        std::uniform_real_distribution<real_t> dist;
        real_t r = dist(gen);
        return r*r*r > 1/(7 * exp((m_r50 - norm(position1 - position2)) * m_k) + 1);
    }

  private:
    //! @brief The connection radius.
    real_t m_r99, m_r50, m_k;

    //! @brief The random number distribution.
    std::uniform_real_distribution<real_t> dist;
};


/**
 * Connection predicate which is true within a maximum radius (can be set through tag `radius`)
 * depending on power data of involved devices. Power is a real number from 0 to 1, and
 * connection is possible within `radius * node1_power * node2_power`.
 *
 * @param num The numerator of the default value for the radius (defaults to 1).
 * @param den The denominator of the default value for the radius (defaults to 1).
 * @param n   Dimensionality of the space (defaults to 2).
 */
template <intmax_t num = 1, intmax_t den = 1, size_t n = 2>
class powered {
  public:
    //! @brief Type for representing a position.
    using position_type = vec<n>;

    //! @brief The node data type.
    using data_type = real_t;

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    powered(G&&, const common::tagged_tuple<S,T>& t) {
        m_radius = common::get_or<component::tags::radius>(t, ((real_t)num)/den);
    }

    //! @brief The maximum radius of connection.
    real_t maximum_radius() const {
        return m_radius;
    }

    //! @brief Checks if connection is possible.
    template <typename G>
    bool operator()(G&&, const data_type& power1, const position_type& position1, const data_type& power2, const position_type& position2) const {
        return norm(position1 - position2) <= m_radius * power1 * power2;
    }

  private:
    //! @brief The connection radius.
    real_t m_radius;
};


}


}

#endif // FCPP_OPTION_CONNECT_H_
