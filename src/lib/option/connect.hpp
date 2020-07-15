// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file connect.hpp
 * @brief Connection predicates to be used with a \ref physical_connector.
 */

#ifndef FCPP_OPTION_CONNECT_H_
#define FCPP_OPTION_CONNECT_H_

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
    using position_type = vec<2>;

    //! @brief The node data type.
    struct data_type {};

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    clique(G&&, const common::tagged_tuple<S,T>&) {}

    //! @brief The maximum radius of connection.
    double maximum_radius() const {
        return INF;
    }

    //! @brief Checks if connection is possible.
    bool operator()(const data_type&, const position_type&, const data_type&, const position_type&) const {
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
    using position_type = vec<2>;

    //! @brief The node data type.
    struct data_type {};

    //! @brief Generator and tagged tuple constructor.
    template <typename G, typename S, typename T>
    fixed(G&&, const common::tagged_tuple<S,T>& t) {
        m_radius = common::get_or<component::tags::radius>(t, ((double)num)/den);
    }

    //! @brief The maximum radius of connection.
    double maximum_radius() const {
        return m_radius;
    }

    //! @brief Checks if connection is possible.
    bool operator()(const data_type&, const position_type& position1, const data_type&, const position_type& position2) const {
        return norm(position1 - position2) <= m_radius;
    }

  private:
    //! @brief The connection radius.
    double m_radius;
};


}


}

#endif // FCPP_OPTION_CONNECT_H_
