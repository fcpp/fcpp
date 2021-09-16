// Copyright © 2021 Gianmarco Rampulla. All Rights Reserved.

/**
 * @file simulated_map.hpp
 * @brief Implementation of the `simulated_map` component handling collision and collision avoidance.
 */

#ifndef FCPP_SIMULATED_MAP_H_
#define FCPP_SIMULATED_MAP_H_

#include "lib/component/base.hpp"
#include "lib/data/vec.hpp"

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {

    //! @brief Declaration tag associating to the dimensionality of the space.
    template <intmax_t n>
    struct dimension;

    //! @brief Declaration tag associating to the bounding coordinates of the grid area.
    template <intmax_t xmin, intmax_t ymin, intmax_t xmax, intmax_t ymax>
    struct area;

    //! @brief Net initialisation tag associating to the minimum coordinates of the grid area.
    struct area_min;

    //! @brief Net initialisation tag associating to the maximum coordinates of the grid area.
    struct area_max;

}

//! @cond INTERNAL
namespace details {

    //! @brief Converts a number sequence to a vec (general form).
    template <typename T>
    struct numseq_to_vec_map;

    //! @brief Converts a number sequence to a vec (active form).
    template <intmax_t... xs>
    struct numseq_to_vec_map<common::number_sequence<xs...>> {
        constexpr static auto value = make_vec(xs...);
    };

}
//! @endcond

/**
 * @brief Component handling node collision and collision avoidance.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::dimension defines the dimensionality of the space (defaults to 2).
 *
 */
template <class... Ts>
struct simulated_map {

    //! @brief The dimensionality of the space.
    constexpr static intmax_t dimension = common::option_num<tags::dimension, 2, Ts...>;

    //! @brief Bounding coordinates of the grid area.
    using area = common::option_nums<tags::area, Ts...>;

    static_assert(area::size == 4 or area::size == 0, "the bounding coordinates must be 4 integers");

    //! @brief Vector of minimum coordinate of the grid area.
    constexpr static auto area_min = details::numseq_to_vec_map<typename area::template slice<0, area::size/2>>::value;

    //! @brief Vector of maximum coordinate of the grid area.
    constexpr static auto area_max = details::numseq_to_vec_map<typename area::template slice<area::size/2>>::value;

    /**
     * @brief The actual component.
     *
     * Component functionalities are added to those of the parent by inheritance at multiple levels: the whole component class inherits tag for static checks of correct composition, while `node` and `net` sub-classes inherit actual behaviour.
     * Further parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     * @param P The parent component to inherit from.
     */
    template <typename F, typename P>
    struct component : public P {

        //! @cond INTERNAL
        DECLARE_COMPONENT(simulated_map);
        //! @endcond

        //! @brief The local part of the component.
        using node = typename P::node;

        //! @brief The global part of the component.
        class net : public P::net {

          public: // visible by node objects and the main program

            //! @brief Type for representing a position.
            using position_type = vec<dimension>;

            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(common::tagged_tuple<S,T> const& t) : P::net(t) {}

            //! @brief Returns the position of the closest empty space starting from node_position
            position_type closest_space(position_type node_position) {}

            //! @brief Returns the position of the closest obstacle starting from node_position
            position_type closest_obstacle(position_type node_position) {}

          private: // implementation details

            //! @brief Type for representing a bitmap index.
            using index_type = std::array<size_t, 2>;

            //! @brief Bitmap representation
            std::vector<std::vector<bool>> m_bitmap;

            //! @brief Matrix containing data to implements closest_space() and closest_obstacle()
            std::vector<std::vector<index_type>> m_closest;

            //! @brief Converts a node position to an equivalent bitmap index
            index_type position_to_index(position_type position) {}

            //! @brief Converts a bitmap index to an equivalent node position
            position_type index_to_position(index_type index, position_type starting_position) {}

        };
    };

};

}

}

#endif // FCPP_SIMULATED_MAP_H_
