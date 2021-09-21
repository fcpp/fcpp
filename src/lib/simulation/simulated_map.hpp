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
    template <intmax_t xmin, intmax_t ymin, intmax_t xmax, intmax_t ymax, intmax_t den>
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

    //! @brief Converts a number sequence to a vec (empty form).
    template <>
    struct numseq_to_vec_map<common::number_sequence<>> {
        constexpr static auto min = make_vec();
        constexpr static auto max = make_vec();
    };

    //! @brief Converts a number sequence to a vec (active form).
    template <intmax_t xmin, intmax_t ymin, intmax_t xmax, intmax_t ymax, intmax_t den>
    struct numseq_to_vec_map<common::number_sequence<xmin,ymin,xmax,ymax,den>> {
        constexpr static auto min = make_vec(xmin/den,ymin/den);
        constexpr static auto max = make_vec(xmax/den,ymax/den);
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

    static_assert(area::size == 5 or area::size == 0, "the bounding coordinates must be 4 integers");

    //! @brief Vector of minimum coordinate of the grid area.
    constexpr static auto area_min = details::numseq_to_vec_map<area>::min;

    //! @brief Vector of maximum coordinate of the grid area.
    constexpr static auto area_max = details::numseq_to_vec_map<area>::max;

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
            net(common::tagged_tuple<S,T> const& t) : P::net(t) {


            }

            //! @brief Returns the position of the closest empty space starting from node_position
            position_type closest_space(position_type node_position) {

                if(!is_obstacle(node_position)) return node_position;

                index_type index = position_to_index(node_position);

                return index_to_position(m_closest[index[0]][index[1]],node_position);

            }

            //! @brief Returns the position of the closest obstacle starting from node_position
            position_type closest_obstacle(position_type node_position) {

                if(is_obstacle(node_position)) return node_position;

                if(!is_in_area(node_position)){

                    auto nearest_in_area_position = get_nearest_edge_position(node_position);

                    return (node_position - nearest_in_area_position) + (nearest_in_area_position - closest_obstacle(nearest_in_area_position));

                }

                index_type index = position_to_index(node_position);

                return index_to_position(m_closest[index[0]][index[1]],node_position);

            }

            //! @brief Returns true if a specific position is a obstacle otherwise false
            bool is_obstacle(position_type position) {

                if(!is_in_area(position)) return false;

                index_type index = position_to_index(position);

                return m_bitmap[index[0]][index[1]];

            }

          private: // implementation details

            //! @brief Type for representing a bitmap index.
            using index_type = std::array<size_t, 2>;

            //! @brief Bitmap representation, a true value means there is an obstacle otherwise false
            std::vector<std::vector<bool>> m_bitmap;

            //! @brief Matrix containing data to implements closest_space() and closest_obstacle(), if a indexed position is empty it contains the nearest obstacle otherwise the nearest empty space position
            std::vector<std::vector<index_type>> m_closest;

            //! @brief Converts a node position to an equivalent bitmap index
            index_type position_to_index(position_type position) {

                return {std::round(position[0]), std::round(position[1])};

            }

            //! @brief Converts a bitmap index to an equivalent node position
            vec<2> index_to_position(index_type index, vec<2> position) {

                return make_vec(index[0], index[1]);

            }

            //! @brief Converts a bitmap index to an equivalent node position
            vec<3> index_to_position(index_type index, vec<3> starting_position) {

                return make_vec(index[0], index[1], starting_position[2]);

            }


            //! @brief Checks if a position is contained in the predefined area
            bool is_in_area(position_type position){

                return position >= area_min && position <= area_max;

            }

            //! @brief convert a vector to a 2d one (first overload)
            vec<2> get_2d_vec(vec<2> vector) {

                return vector;

            }

            //! @brief convert a vector to a 2d one (second overload)
            vec<2> get_2d_vec(vec<3> vector){

                return make_vec(vector[0], vector[1]);

            }

            //! @brief calculates the nearest in area position (edge position) starting from a generic node position
            vec<2> get_nearest_edge_position(position_type node_position){

                auto min_distance = 0.0;
                auto min_pos = node_position;
                auto potential_min_pos = node_position;
                auto bidimensional_pos = get_2d_vec(node_position);
                auto i = 0.0;
                auto y_starting_point = 0.0;
                auto x_starting_point = 0.0;

                //generates all points of the lower edge
                for(i = area_min[0]; i <= area_max[0]; i++){

                    potential_min_pos = make_vec(i, area_min[1]);

                    if(min_distance >= distance(bidimensional_pos, potential_min_pos)){

                        min_distance = distance(bidimensional_pos, potential_min_pos);
                        min_pos = potential_min_pos;

                    }

                }

                x_starting_point = i;

                //generates all points of the left edge
                for(i = area_min[1]; i <= area_max[1]; i++){

                    potential_min_pos = make_vec(area_min[0], i);

                    if(min_distance >= distance(bidimensional_pos, potential_min_pos)){

                        min_distance = distance(bidimensional_pos, potential_min_pos);
                        min_pos = potential_min_pos;

                    }

                }

                y_starting_point = i;

                //generates all points of the upper edge
                for(i = area_min[0]; i <= area_max[0]; i++){

                    potential_min_pos = make_vec(i, y_starting_point);

                    if(min_distance >= distance(bidimensional_pos, potential_min_pos)){

                        min_distance = distance(bidimensional_pos, potential_min_pos);
                        min_pos = potential_min_pos;

                    }

                }

                //generates all points of the right edge
                for(i = area_min[0]; i <= area_max[1]; i++){

                    potential_min_pos = make_vec(x_starting_point, i);

                    if(min_distance >= distance(bidimensional_pos, potential_min_pos)){

                        min_distance = distance(bidimensional_pos, potential_min_pos);
                        min_pos = potential_min_pos;

                    }

                }

                return min_pos;

            }


        };
    };

};

}

}

#endif // FCPP_SIMULATED_MAP_H_
