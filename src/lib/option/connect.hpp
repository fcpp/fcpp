// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

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


// Namespace for all FCPP components.
namespace component {

// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Net initialisation tag associating to communication radius (required for \ref connect::fixed and \ref connect::powered).
    struct radius {};

    //! @brief Net initialisation tag associating to 50%-likely communication radius (required for \ref connect::radial).
    struct half_radius {};

    //! @brief Node initialisation tag associating to a network rank (required for \ref connect::hierarchical).
    struct network_rank {};

    //! @brief Node initialisation tag associating to the ratio to full power (required for \ref connect::powered).
    struct power_ratio {};
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
    using data_type = common::tagged_tuple_t<>;

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    clique(G&&, common::tagged_tuple<S,T> const&) {}

    //! @brief The maximum radius of connection.
    real_t maximum_radius() const {
        return INF;
    }

    //! @brief The maximum radius of connection.
    template <typename T>
    real_t relative_radius(T const&, T const&) const {
        return INF;
    }

    //! @brief Checks if connection is possible.
    template <typename G>
    bool operator()(G&&, data_type const&, position_type const&, data_type const&, position_type const&) const {
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
class fixed : public clique<n> {
    //! @brief Shortcut to the parent fixed connector.
    using C = clique<n>;

    //! brief Shortcut to the radius tag.
    using radius = component::tags::radius;

  public:
    //! @brief Type for representing a position.
    using typename C::position_type;

    //! @brief The node data type.
    using typename C::data_type;

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    fixed(G&& g, common::tagged_tuple<S,T> const& t) : C(std::forward<G>(g), t) {
        m_radius = common::get_or<radius>(t, ((real_t)num)/den);
    }

    //! @brief The maximum radius of connection.
    real_t maximum_radius() const {
        return m_radius;
    }

    //! @brief The maximum radius of connection.
    template <typename T>
    real_t relative_radius(T const&, T const&) const {
        return m_radius;
    }

    //! @brief Checks if connection is possible.
    template <typename G, typename T>
    bool operator()(G&&, T const& data1, position_type const& pos1, T const& data2, position_type const& pos2) const {
        return norm(pos1 - pos2) <= relative_radius(data1, data2);
    }

  private:
    //! @brief The connection radius.
    real_t m_radius;
};


/**
 * Connection predicate which is true within a maximum radius (can be set through tag `radius`)
 * depending on \ref component::tags::power_ratio data of involved devices. Power is a real number
 * from 0 to 1, and connection is possible within `radius * node1_power * node2_power`.
 *
 * @param num The numerator of the default value for the radius (defaults to 1).
 * @param den The denominator of the default value for the radius (defaults to 1).
 * @param n   Dimensionality of the space (defaults to 2).
 */
template <intmax_t num = 1, intmax_t den = 1, size_t n = 2>
class powered : public fixed<num,den,n> {
    //! @brief Shortcut to the parent fixed connector.
    using C = fixed<num,den,n>;

    //! brief Shortcut to the power_ratio tag.
    using power_ratio = component::tags::power_ratio;

  public:
    //! @brief Type for representing a position.
    using typename C::position_type;

    //! @brief The node data type.
    using data_type = typename C::data_type::template push_back<power_ratio, real_t>;

    //! @brief Inheriting constructor.
    using C::C;

    //! @brief The maximum radius of connection.
    template <typename T>
    real_t relative_radius(T const& data1, T const& data2) const {
        return C::relative_radius(data1, data2) * common::get<power_ratio>(data1) * common::get<power_ratio>(data2);
    }

    //! @brief Checks if connection is possible.
    template <typename G, typename T>
    bool operator()(G&&, T const& data1, position_type const& pos1, T const& data2, position_type const& pos2) const {
        return norm(pos1 - pos2) <= relative_radius(data1, data2);
    }
};


/**
 * @brief Connection predicate modifying a base connector with a likelyhood depending on radius (with tag `radius` setting maximum radius and `half_radius` setting half radius).
 *
 * The half radius (50% communication failure) is given as a percentile (1-99) over the maximum connection radius.
 * The maximum radius is set at the point \f$ r99 \f$ where 99% of communication would fail in the theoretical
 * distribution, which has empirically chosen density:
 * \f[
 * p(r) = \left( 7 e^\frac{\textstyle (\frac{r50}{100} - \frac{r}{r99})\log(\frac{6792093}{29701})}{\textstyle 1 - \frac{r50}{100}} + 1 \right)^{-\frac{\textstyle 1}{\textstyle 3}}
 * \f]
 *
 * @param r50 The default value for the half radius.
 * @param C   The basic connector to modify.
 */
template <intmax_t r50, typename C>
class radial : public C {
    static_assert(1 <= r50 and r50 <= 99, "\033[1m\033[4mhalf radius out of bounds\033[0m");

    //! brief Shortcut to the power_ratio tag.
    using half_radius = component::tags::half_radius;

  public:
    //! @brief Type for representing a position.
    using typename C::position_type;

    //! @brief The node data type.
    using typename C::data_type;

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    radial(G&& g, common::tagged_tuple<S,T> const& t) : C(std::forward<G>(g), t) {
        m_r50 = common::get_or<half_radius>(t, (real_t)r50)/100;
        m_k = log(6792093.0/29701) / (1-m_r50);
    }

    //! @brief Checks if connection is possible.
    template <typename G, typename T>
    bool operator()(G&& g, T const& data1, position_type const& pos1, T const& data2, position_type const& pos2) const {
        if (not C::operator()(std::forward<G>(g), data1, pos1, data2, pos2)) return false;
        std::uniform_real_distribution<real_t> dist;
        real_t r = dist(g);
        real_t r99 = C::relative_radius(data1, data2);
        return r*r*r > 1/(7 * exp((m_r50 - norm(pos1 - pos2)/r99) * m_k) + 1);
    }

  private:
    //! @brief The half radius and distribution scaling factor.
    real_t m_r50, m_k;
};


/**
 * Connection predicate adding a hierarchical condition on connectivity: devices are
 * only allowed to connect to others exactly one step lower (on on the same step
 * if zero or negative priority) in their \ref component::tags::network_rank value.
 *
 * @param C   The basic connector to modify.
 */
template <typename C>
class hierarchical : public C {
    //! brief Shortcut to the network_rank tag.
    using network_rank = component::tags::network_rank;

  public:
    //! @brief Type for representing a position.
    using typename C::position_type;

    //! @brief The node data type.
    using data_type = typename C::data_type::template push_back<network_rank, int>;

    //! @brief Inheriting constructor.
    using C::C;

    //! @brief Checks if connection is possible.
    template <typename G, typename T>
    bool operator()(G&& g, T const& data1, position_type const& pos1, T const& data2, position_type const& pos2) const {
        int delta = abs(common::get<network_rank>(data1) - common::get<network_rank>(data2));
        return (delta == 1 or (delta == 0 and common::get<network_rank>(data1) <= 0)) and C::operator()(std::forward<G>(g), data1, pos1, data2, pos2);
    }
};


}


}

#endif // FCPP_OPTION_CONNECT_H_
