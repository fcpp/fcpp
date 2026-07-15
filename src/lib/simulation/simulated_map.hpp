// Copyright © 2023 Gianmarco Rampulla. All Rights Reserved.

/**
 * @file simulated_map.hpp
 * @brief Implementation of the `simulated_map` component handling obstacle collision control.
 */

#ifndef FCPP_SIMULATED_MAP_H_
#define FCPP_SIMULATED_MAP_H_


#include <cmath>
#include <cstring>
#include <stack>
#include <queue>
#include <set>
#include <limits>
#include <map>
#include <cassert>

#include <stb_image.h>
#include <stb_image_write.h>

#include "lib/common/traits.hpp"
#include "lib/component/base.hpp"
#include "lib/data/color.hpp"
#include "lib/data/vec.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

// Namespace for all FCPP components.
namespace component {

// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to the dimensionality of the space (defaults to 2).
    template <intmax_t n>
    struct dimension;

    //! @brief Declaration tag associating to the bounding coordinates of the grid area (defaults to the minimal area covering initial nodes)
    template <intmax_t xmin, intmax_t ymin, intmax_t xmax, intmax_t ymax, intmax_t den>
    struct area;

    //! @brief Net initialisation tag associating to the minimum coordinates of the grid area (defaults to the value in \ref tags::area).
    struct area_min;

    //! @brief Net initialisation tag associating to the maximum coordinates of the grid area (defaults to the value in \ref tags::area).
    struct area_max;

    //! @brief Net initialisation tag associating to the map navigator object.
    struct navigator {};
}

//! @cond INTERNAL
namespace details {
    //! @brief Converts a number sequence to a vec (general form).
    template<typename T>
    struct numseq_to_vec_map;
    //! @brief Converts a number sequence to a vec (empty form).
    template<>
    struct numseq_to_vec_map<common::number_sequence<>> {
        constexpr static vec<0> min{};
        constexpr static vec<0> max{};
    };
    //! @brief Converts a number sequence to a vec (active form).
    template<intmax_t xmin, intmax_t ymin, intmax_t xmax, intmax_t ymax, intmax_t den>
    struct numseq_to_vec_map<common::number_sequence<xmin, ymin, xmax, ymax, den>> {
        constexpr static vec<2> min{xmin * 1.0 / den, ymin * 1.0 / den};
        constexpr static vec<2> max{xmax * 1.0 / den, ymax * 1.0 / den};
    };

}
}
//! @endcond

struct map_navigator {

public:
    using index_type = std::array<size_t, 2>;

    map_navigator(const std::string& path = "", color obstacle_color = color(BLACK), real_t obstacles_color_threshold = 0.5) {

        load_bitmap(path,obstacle_color,obstacles_color_threshold);

        /*
        //load image to draw results (debug)
        int bitmap_width, bitmap_height, channels_per_pixel, row_index, col_index = 0;
        std::string real_path;
#if _WIN32
        real_path = std::string(".\\textures\\").append(path);
#else
        //real_path = std::string("./textures/").append(path);
        real_path = path;
#endif
        unsigned char *p = stbi_load(real_path.c_str(), &bitmap_width, &bitmap_height,&channels_per_pixel, 0);
*/

        m_map_rooms = std::vector<std::vector<std::vector<int>>>(m_bitmap.size(),std::vector<std::vector<int>>(m_bitmap[0].size(),std::vector<int>(2, 0)));
        if (is_empty()) return;
        auto rooms_function = std::function<bool(int, int, int, int, std::vector<std::vector<bool>> &)>(
                [&](int x, int y, int bitmap, int obstacle,
                    std::vector<std::vector<bool>> &visited_matrix) {
                    if (obstacle == 1)
                        return bitmap > 0 || (bitmap == 0 && m_bitmap[y][x]);
                    else if (obstacle == 0)
                        return (bitmap < 0 || (bitmap == 0 && !m_bitmap[y][x]));
                    return false;
                });

        auto rooms_function_no_obstacle = std::function<bool(int, int, int, int, std::vector<std::vector<bool>> &)>(
                [&](int x, int y, int bitmap, int obstacle,
                    std::vector<std::vector<bool>> &visited_matrix) {
                    if (m_bitmap[y][x]) visited_matrix[y][x] = true;
                    if (obstacle == 1) return bitmap > 0;
                    else if (obstacle == 0)
                        return (bitmap < 0 || (bitmap == 0 && !m_bitmap[y][x]));
                    return false;
                });

        fill_closest(m_closest, m_bitmap,
                     std::function<bool(int, int, bool, int, std::vector<std::vector<bool>> &)>(
                             [](int x, int y, bool bitmap, int obstacle,
                                std::vector<std::vector<bool>> &visited_matrix) {
                                 return bitmap == obstacle;
                             }));

        start_room_subdivision(m_closest, m_bitmap, std::function<bool(int, int, bool, int)>([](int x, int y, bool element, int arg) { return element; }),path);

        int red_point = 0;
        m_room_iteration_count = 0;
        do {
            fill_closest_rooms(m_rooms_closest, m_map_rooms, rooms_function);
            red_point = start_room_subdivision_rooms(m_rooms_closest, m_map_rooms,std::function<bool(int, int, int, int)>([&](int x, int y, int element, int arg) {return element > 0 || (element == 0 &&m_bitmap[y][x]);}),path,false);
            m_room_iteration_count++;
        } while (red_point > 0);

        fill_closest_rooms(m_rooms_closest, m_map_rooms, rooms_function_no_obstacle);
        fill_empty_spaces(path);


        /* debug
        for(int r = 0; r < m_bitmap.size(); r++) {
            for(int c = 0; c < m_bitmap[0].size(); c++) {
                if (m_map_rooms[r][c][0] > 0) {
                    write_point_on_image(p, r, c, bitmap_width, bitmap_height, channels_per_pixel,color::hsva(30 * ((m_map_rooms[r][c][0] / 4) % 12),0.5 + 0.5 * ((m_map_rooms[r][c][0] / 2) % 2),0.5 + 0.5 * (m_map_rooms[r][c][0] % 2)), false);
                }
            }
        }


        for(index_type t : red_point) {
            write_point_on_image(p, t[1], t[0], bitmap_width, bitmap_height, channels_per_pixel,color(RED), false);
        }


        if(!path.empty())
            stbi_write_png("debugPoints.png", bitmap_width, bitmap_height, channels_per_pixel, p, bitmap_width * channels_per_pixel);
        */

        calculate_waypoints(path);

        //draw waypoints (debug)
        /*
        for(waypoints p2 : m_waypoints_list) {
            write_point_on_image(p, p2.second[1], p2.second[0], bitmap_width, bitmap_height, channels_per_pixel, color(RED), false);
        }

        if(!path.empty())
            stbi_write_png("debugPoints.png", bitmap_width, bitmap_height, channels_per_pixel, p, bitmap_width * channels_per_pixel);

        stbi_image_free(p);
        */

        for(int i = 0; i < m_waypoints_list.size(); i++) {
            assert((m_waypoints_list[i].second[0] < m_bitmap[0].size() && m_waypoints_list[i].second[0] >= 0) && (m_waypoints_list[i].second[1] < m_bitmap.size() && m_waypoints_list[i].second[1] >= 0));
        }

        nav_floyd_warshall();

        /*
        for(int i = 0; i < m_floyd_matrix.size(); i++) {
            for(int j = 0; j < m_floyd_matrix[0].size(); j++) {
                assert(m_floyd_matrix[i][j] < std::numeric_limits<real_t>::max());
            }
        }
         */
    }

    index_type get_closest_from(index_type index) {
        assert(0 <= index[0] and index[0] < m_bitmap[0].size());
        assert(0 <= index[1] and index[1] < m_bitmap.size());
        return m_closest[index[1]][index[0]];
    }

    bool is_obstacle_at(index_type index) {
        assert(0 <= index[0] and index[0] < m_bitmap[0].size());
        assert(0 <= index[1] and index[1] < m_bitmap.size());
        return m_bitmap[index[1]][index[0]];
    }

    index_type get_bitmap_size() {
        return {m_bitmap[0].size(), m_bitmap.size()};
    }

    std::pair<index_type,real_t> path_to(index_type source, index_type dest, bool debug = false) {
        assert(not is_obstacle_at(source));
        assert(not is_obstacle_at(dest));
        real_t best_distance = std::numeric_limits<real_t>::infinity();
        index_type best_waypoint = source;
        constexpr real_t range = 0.2;

        if (debug) {
            std::cout<<"Starting pos: ["<<source[0]<<","<<source[1]<<"]"<<std::endl;
            std::cout<<"Target pos: ["<<dest[0]<<","<<dest[1]<<"]"<<std::endl;
        }

        for (int p : get_rooms_at(source)) if (p > 0) {
            for (int q : get_rooms_at(dest)) if (q > 0) {
                // same room, go straight
                if (p == q)
                    return {dest, 0};
                for (index_type first_waypoint: get_waypoints_for(p)) {
                    if (debug) {
                        std::cout<<"first_waypoint considered: ["<<first_waypoint[0]<<","<<first_waypoint[1]<<"]"<<std::endl;
                    }

                    if (first_waypoint[0] - source[0] <= range and first_waypoint[1] - source[1] <= range) {
                        std::cout << m_floyd_matrix[m_waypoint_index[source]][m_waypoint_index[dest]] << std::endl; // DEBUG
                        continue;
                    }
                    for (index_type last_waypoint: get_waypoints_for(q)) {
                        real_t distance = get_distance(source, first_waypoint, last_waypoint, dest);
                        if (distance <= best_distance) {
                            best_distance = distance;
                            best_waypoint = first_waypoint;
                        }
                        if (debug) {
                            std::cout<<"rooms considered: ["<<m_map_rooms[first_waypoint[1]][first_waypoint[0]][0]<<","<<m_map_rooms[last_waypoint[1]][last_waypoint[0]][0]<<"]"<<std::endl;
                            std::cout<<"waypoints considered: ["<<first_waypoint[0]<<","<<first_waypoint[1]<<"]"<<"   ["<<last_waypoint[0]<<","<<last_waypoint[1]<<"], distance: "<<get_eu_distance(first_waypoint[0],last_waypoint[0],first_waypoint[1],last_waypoint[1])<<std::endl;
                        }
                    }
                }
            }
        }

        if (debug) {
            std::cout<<"Starting pos: ["<<source[0]<<","<<source[1]<<"]"<<std::endl;
            std::cout<<"Target pos: ["<<dest[0]<<","<<dest[1]<<"]"<<std::endl;
            std::cout<<"best_waypoint: ["<<best_waypoint[0]<<","<<best_waypoint[1]<<"]"<<std::endl;
            std::cout<<"best_waypoint distance: "<<best_distance - get_eu_distance(source[0],dest[0],source[1],dest[1])<<std::endl;
        }

        return {best_waypoint, best_distance - get_eu_distance(source[0],dest[0],source[1],dest[1])};
    }

    bool is_empty() {
        return empty;
    }

private:

    //! @brief Type for representing a bfs queue pair of point and its source point from which was generated.
    using matrix_pair_type = std::pair<index_type, index_type>;

    std::vector<int> get_rooms_at(index_type index) {
        return m_map_rooms[index[1]][index[0]];
    }

    std::vector<index_type> get_waypoints_for(int room) {
        return m_waypoints_per_rooms[room];
    }

    real_t get_distance(index_type index_source, index_type first_waypoint, index_type last_waypoint, index_type index_dest) {
        return get_eu_distance(index_source[0], first_waypoint[0], index_source[1], first_waypoint[1]) +
               m_floyd_matrix[m_waypoint_index[first_waypoint]][m_waypoint_index[last_waypoint]] +
               get_eu_distance(last_waypoint[0], index_dest[0], last_waypoint[1], index_dest[1]);
    }

    void write_point_on_image(unsigned char* pointer, int row, int col, int bitmap_width, int bitmap_height, int channels, color clr, bool visit = true) {
        if(col >= 0 && col < m_bitmap[0].size() && row >= 0 && row < m_bitmap.size()) {
            for (int j = 0; j < channels; j++) {
                (pointer + (channels * ((bitmap_height-1 - row) * bitmap_width + col)))[j] = clr.rgba[j] * 255;
            }
        }
    }

    //! @brief Fills m_bitmap by reading and parsing a stored bitmap given the bitmap file path
    void load_bitmap(std::string const &path, color const &color, real_t threshold) {
        if (path.empty()) {
            m_bitmap = {{false}};
            empty = true;
            return;
        }
        int bitmap_width, bitmap_height, channels_per_pixel, row_index, col_index = 0;
        std::string real_path;
        threshold *= 255;
#if _WIN32
        real_path = std::string(".\\textures\\").append(path);
#else
        real_path = std::string("./textures/").append(path);
        //real_path = path;
#endif
        unsigned char *bitmap_data = stbi_load(real_path.c_str(), &bitmap_width, &bitmap_height,&channels_per_pixel, 0);
        unsigned char *copy_ptr = bitmap_data;
        if (bitmap_data == nullptr) throw std::runtime_error("Error in image loading");

        row_index = bitmap_height-1;
        m_bitmap = std::vector<std::vector<bool>>(bitmap_height,std::vector<bool>(bitmap_width, true));

        while (row_index >= 0) {
            unsigned char *pixel_ptr = (bitmap_data + (channels_per_pixel * ((bitmap_height-1 - row_index) * bitmap_width + col_index)));
            for (int j = 0; j < channels_per_pixel && m_bitmap[row_index][col_index]; j++)
                m_bitmap[row_index][col_index] = m_bitmap[row_index][col_index] && std::abs(color.rgba[j] * 255 - pixel_ptr[j]) < threshold;
            col_index++;
            if (col_index == bitmap_width) {
                row_index--;
                col_index = 0;
            }
        }
    }

    template<typename obstacle_type, typename obstacle_arg>
    int start_room_subdivision(std::vector<std::vector<index_type>> &closest_map,std::vector<std::vector<obstacle_type>> &obstacles_map,std::function<bool(int, int, obstacle_type, obstacle_arg)> predicate,std::string const &path, bool noroom = false) {

        constexpr static std::array<std::array<int, 3>, 8> deltas = {{{-1, 0, 2}, {1, 0, 2}, {0, 1, 2}, {0, -1, 2}, {1, 1, 3}, {-1, 1, 3}, {1, -1, 3},{-1, -1, 3}}};
        int row_index, col_index;
        int point_number = 0;

        std::vector<std::vector<bool>> visited(m_bitmap.size(),std::vector<bool>(m_bitmap[0].size()));
        std::vector<std::vector<std::pair<index_type, color>>> draw_buffers;
        std::vector<std::pair<index_type, int>> point_values;

        //prepare 2 buffers
        draw_buffers.emplace_back();
        draw_buffers.emplace_back();

        row_index = m_bitmap.size() - 1;
        col_index = 0;

        while (row_index >= 0) {

            if (!predicate(col_index, row_index, obstacles_map[row_index][col_index], 0)) {

                if (col_index >= 0 && col_index < obstacles_map[0].size() && row_index >= 0 && row_index < obstacles_map.size() && !predicate(col_index, row_index, obstacles_map[row_index][col_index], 0)) {
                    int distance = get_distance(closest_map[row_index][col_index], col_index,row_index);
                    int n_distance;
                    bool max = true;
                    for (std::array<int, 3> const &d: deltas) {
                        size_t n_x = col_index + d[0];
                        size_t n_y = row_index + d[1];
                        if (n_x >= 0 && n_x < obstacles_map[0].size() && n_y >= 0 &&
                            n_y < obstacles_map.size() &&
                            !predicate(n_x, n_y, obstacles_map[n_y][n_x], 0)) {
                            n_distance = get_distance(closest_map[n_y][n_x], n_x, n_y);
                            max &= distance >= n_distance;
                        }
                    }

                    if (max && distance > 16) {
                        index_type t;
                        t[0] = col_index;
                        t[1] = row_index;
                        draw_buffers[0].emplace_back(t, color(RED));
                        point_number++;

                        if (!visited[row_index][col_index]) {
                            std::array<int, 2> b = start_it_dfs(closest_map, obstacles_map, predicate,visited, row_index, col_index);
                            if (b[0] >= 0 && b[1] >= 0) {
                                index_type b2;
                                b2[0] = b[0];
                                b2[1] = b[1];
                                draw_buffers[1].emplace_back(b2, color(BLUE));
                                m_centroids.emplace(get_distance(b2, closest_map[b2[1]][b2[0]][0],closest_map[b2[1]][b2[0]][1]), b);
                            }
                        }
                    }
                }
            }
            col_index++;
            if (col_index == m_bitmap[0].size()) {
                row_index--;
                col_index = 0;
            }
        }

        if (!noroom) create_rooms(predicate);

        return point_number;

    }

    template<typename obstacle_type, typename obstacle_arg>
    int start_room_subdivision_rooms(std::vector<std::vector<index_type>> &closest_map,std::vector<std::vector<std::vector<obstacle_type>>> &obstacles_map,std::function<bool(int, int, obstacle_type,obstacle_arg)> predicate,std::string const &path, bool noroom = false) {
        constexpr static std::array<std::array<int, 3>, 8> deltas = {{{-1, 0, 2}, {1, 0, 2}, {0, 1, 2}, {0, -1, 2}, {1, 1, 3}, {-1, 1, 3}, {1, -1, 3},{-1, -1, 3}}};
        int row_index, col_index;
        std::string real_path;
        int point_number = 0;


        std::vector<std::vector<bool>> visited(m_bitmap.size(),std::vector<bool>(m_bitmap[0].size()));
        std::vector<std::vector<std::pair<index_type, color>>> draw_buffers;
        std::vector<index_type> red_points;
        std::vector<std::pair<index_type, int>> point_values;

        //prepare 2 buffers
        draw_buffers.emplace_back();
        draw_buffers.emplace_back();

        row_index = m_bitmap.size() - 1;
        col_index = 0;

        while (row_index >= 0) {

            if (!predicate(col_index, row_index, obstacles_map[row_index][col_index][0], 0)) {

                if (col_index >= 0 && col_index < obstacles_map[0].size() && row_index >= 0 && row_index < obstacles_map.size() &&!predicate(col_index, row_index, obstacles_map[row_index][col_index][0], 0)) {

                    int distance = get_distance(closest_map[row_index][col_index], col_index,row_index);
                    int n_distance;
                    bool max = true;
                    for (std::array<int, 3> const &d: deltas) {
                        size_t n_x = col_index + d[0];
                        size_t n_y = row_index + d[1];
                        if (n_x >= 0 && n_x < obstacles_map[0].size() && n_y >= 0 &&
                            n_y < obstacles_map.size() &&
                            !predicate(n_x, n_y, obstacles_map[n_y][n_x][0], 0)) {
                            n_distance = get_distance(closest_map[n_y][n_x], n_x, n_y);
                            max &= distance >= n_distance;
                        }
                    }

                    if (max && distance > 16) {
                        index_type t;
                        t[0] = col_index;
                        t[1] = row_index;
                        red_points.emplace_back(t);
                        point_number++;

                        if (!visited[row_index][col_index]) {
                            std::array<int, 2> b = start_it_dfs_rooms(closest_map, obstacles_map,predicate,visited, row_index,col_index);
                            if (b[0] >= 0 && b[1] >= 0) {
                                index_type b2;
                                b2[0] = b[0];
                                b2[1] = b[1];
                                draw_buffers[1].emplace_back(b2, color(BLUE));
                                m_centroids.emplace(get_distance(b2, closest_map[b2[1]][b2[0]][0], closest_map[b2[1]][b2[0]][1]), b);
                            }
                        }
                    }
                }
            }
            col_index++;
            if (col_index == m_bitmap[0].size()) {
                row_index--;
                col_index = 0;
            }
        }

        if (!noroom) create_rooms(predicate);

        return point_number;

    }


    int calculate_waypoints(std::string const &path) {
        constexpr static std::array<std::array<int, 3>, 8> deltas = {{{-1, 0, 2}, {1, 0, 2}, {0, 1, 2}, {0, -1, 2}, {1, 1, 3}, {-1, 1, 3}, {1, -1, 3},{-1, -1, 3}}};
        std::vector<std::vector<bool>> visited(m_bitmap.size(),std::vector<bool>(m_bitmap[0].size()));
        int point_number = 0;

        for (int r = 0; r < m_map_rooms.size(); r++) {
            for (int c = 0; c < m_map_rooms[0].size(); c++) {
                if (!m_bitmap[r][c]) {
                    int current_cell_id = m_map_rooms[r][c][0];
                    for (auto &d: deltas) {
                        int n_x = c + d[0];
                        int n_y = r + d[1];
                        if (n_x >= 0 && n_x < m_map_rooms[0].size() && n_y >= 0 &&
                            n_y < m_map_rooms.size() && !m_bitmap[n_y][n_x]) {
                            int neighbour_id = m_map_rooms[n_y][n_x][0];
                            if (current_cell_id != neighbour_id) {m_waypoint_map.insert(std::pair<std::pair<int, int>, std::vector<std::pair<int, int>>>(std::pair<int, int>(current_cell_id, neighbour_id),std::vector<std::pair<int, int>>()));
                                m_waypoint_map.at({current_cell_id, neighbour_id}).emplace_back(n_x,n_y);
                            }
                        }
                    }
                }
            }
        }

        std::set<std::pair<int, int>> already_visited;
        //m_navigation_graph = std::vector<std::vector<std::pair<int,std::pair<int,int>>>>(m_rooms_count);

        using border_indexes = std::pair<int, int>;
        using waypoint = std::pair<int, int>;

        std::vector<std::pair<border_indexes, waypoint>> waypoints_to_connect;

        for (auto &iter: m_waypoint_map) {
            std::vector<std::pair<int, int>> curr_vec = iter.second;
            if (already_visited.find(iter.first) == already_visited.end()) {
                already_visited.emplace(iter.first.first, iter.first.second);
                int sumX = 0;
                int sumY = 0;
                int count = 0;
                for (std::pair<int, int> &c: curr_vec) {
                    if(!m_bitmap[c.second][c.first]) {
                        sumX += c.first;
                        sumY += c.second;
                        m_map_rooms[c.second][c.first][0] = iter.first.first;
                        m_map_rooms[c.second][c.first][1] = iter.first.second;
                        for (int i = 1; i < m_room_iteration_count + 1; i++) {
                            if (c.first + i < m_map_rooms[0].size() && !m_bitmap[c.second][c.first + i]) {
                                m_map_rooms[c.second][c.first + i][0] = iter.first.first;
                                m_map_rooms[c.second][c.first + i][1] = iter.first.second;
                            }
                            if (c.first - i >= 0 && !m_bitmap[c.second][c.first - i]) {
                                m_map_rooms[c.second][c.first - i][0] = iter.first.first;
                                m_map_rooms[c.second][c.first - i][1] = iter.first.second;
                            }
                            if (c.second + i < m_map_rooms.size() && !m_bitmap[c.second + i][c.first]) {
                                m_map_rooms[c.second + i][c.first][0] = iter.first.first;
                                m_map_rooms[c.second + i][c.first][1] = iter.first.second;
                            }
                            if (c.second - i >= 0 && !m_bitmap[c.second - i][c.first]) {
                                m_map_rooms[c.second - i][c.first][0] = iter.first.first;
                                m_map_rooms[c.second - i][c.first][1] = iter.first.second;
                            }
                        }
                        count++;
                    }
                }
                try {
                    std::vector<std::pair<int, int>> opposite_vec = m_waypoint_map.at({iter.first.second, iter.first.first});
                    already_visited.emplace(iter.first.second, iter.first.first);
                    for (std::pair<int, int> &c: opposite_vec) {
                        if(!m_bitmap[c.second][c.first]) {
                            sumX += c.first;
                            sumY += c.second;
                            m_map_rooms[c.second][c.first][0] = iter.first.first;
                            m_map_rooms[c.second][c.first][1] = iter.first.second;
                            for (int i = 1; i < m_room_iteration_count + 1; i++) {
                                if (c.first + i < m_map_rooms[0].size() && !m_bitmap[c.second][c.first + i]) {
                                    m_map_rooms[c.second][c.first + i][0] = iter.first.first;
                                    m_map_rooms[c.second][c.first + i][1] = iter.first.second;
                                }
                                if (c.first - i >= 0 && !m_bitmap[c.second][c.first - i]) {
                                    m_map_rooms[c.second][c.first - i][0] = iter.first.first;
                                    m_map_rooms[c.second][c.first - i][1] = iter.first.second;
                                }
                                if (c.second + i < m_map_rooms.size() && !m_bitmap[c.second + i][c.first]) {
                                    m_map_rooms[c.second + i][c.first][0] = iter.first.first;
                                    m_map_rooms[c.second + i][c.first][1] = iter.first.second;
                                }
                                if (c.second - i >= 0 && !m_bitmap[c.second - i][c.first]) {
                                    m_map_rooms[c.second - i][c.first][0] = iter.first.first;
                                    m_map_rooms[c.second - i][c.first][1] = iter.first.second;
                                }
                            }
                            count++;
                        }
                    }
                }
                catch (const std::exception &e) {}

                int centroidX = sumX / count;
                int centroidY = sumY / count;

                waypoints_to_connect.emplace_back(border_indexes(iter.first.first, iter.first.second),waypoint(centroidX, centroidY));

                index_type n;
                n[0] = centroidX;
                n[1] = centroidY;
                m_waypoints_list.emplace_back(std::pair<int, int>(iter.first.first, iter.first.second), n);

                m_waypoints_per_rooms[iter.first.first].emplace_back(n);
                m_waypoints_per_rooms[iter.first.second].emplace_back(n);
                m_map_rooms[centroidY][centroidX][0] = iter.first.first;
                m_map_rooms[centroidY][centroidX][1] = iter.first.second;

            }
        }

        m_navigation_graph = std::vector<std::vector<real_t>>(m_waypoints_list.size(),std::vector<real_t>(m_waypoints_list.size(),std::numeric_limits<real_t>::max()));

        for (int i = 0; i < m_waypoints_list.size(); i++) {
            int room_index1 = m_waypoints_list[i].first.first;
            int room_index2 = m_waypoints_list[i].first.second;
            m_waypoint_index[m_waypoints_list[i].second] = i;
            for (int j = 0; j < m_waypoints_list.size(); j++) {
                int room_index1p = m_waypoints_list[j].first.first;
                int room_index2p = m_waypoints_list[j].first.second;
                if (room_index1 == room_index1p || room_index1 == room_index2p ||
                    room_index2 == room_index1p || room_index2 == room_index2p) {
                    real_t distance = get_eu_distance(m_waypoints_list[i].second[0],m_waypoints_list[j].second[0],m_waypoints_list[i].second[1],m_waypoints_list[j].second[1]);
                    m_navigation_graph[i][j] = distance;
                }
            }
        }
        return point_number;
    }

    int get_distance(index_type const &closest, int x, int y) {

        int deltaX = std::abs((int) closest[0] - x);
        int deltaY = std::abs((int) closest[1] - y);

        return std::min(deltaX, deltaY) + (2 * std::max(deltaX, deltaY));

    }

    inline real_t get_eu_distance(real_t px, real_t qx, real_t py, real_t qy) {
        return std::sqrt((qx - px) * (qx - px) + (qy - py) * (qy - py));
    }

    inline bool obstacle_blocking(std::array<int, 2> const &B, int Ox, int Oy, int Px, int Py, int k) {

        int OBx = B[0] - Ox;
        int OBy = B[1] - Oy;
        int OPx = Px - Ox;
        int OPy = Py - Oy;
        int result = OPx * OBx + OPy * OBy;

        return result <= k * std::sqrt(OBx * OBx + OBy * OBy);
    }

    bool check_obstacles(std::vector<std::pair<real_t, real_t>> const &obstacles,std::array<int, 2> const &B,int x, int y, int k) {
        for (std::pair<real_t, real_t> o: obstacles) {
            if (obstacle_blocking(B, o.first, o.second, x, y, k)) {
                return false;
            }
        }
        return true;
    }


    void fill_empty_spaces(std::string const &path) {
        std::vector<std::vector<bool>> visited(m_bitmap.size(),std::vector<bool>(m_bitmap[0].size()));
        for (int r = 0; r < m_map_rooms.size(); r++) {
            for (int c = 0; c < m_map_rooms[0].size(); c++) {
                if (m_map_rooms[r][c][0] < 0 || (m_map_rooms[r][c][0] == 0 && !m_bitmap[r][c])) {
                    index_type t = m_rooms_closest[r][c];
                    m_map_rooms[r][c] = m_map_rooms[t[1]][t[0]];
                }
            }
        }
    }

    //! @brief create rooms for navigations
    template<typename obstacle_type, typename obstacle_arg>
    void create_rooms(std::function<bool(int, int, obstacle_type, obstacle_arg)> predicate) {

        constexpr static std::array<std::array<int, 3>, 8> deltas = {{{-1, 0, 2}, {1, 0, 2}, {0, 1, 2}, {0, -1, 2}, {1, 1, 3}, {-1, 1, 3}, {1, -1, 3},{-1, -1, 3}}};
        //int rooms_count = 0;

        //dynamic queues vector with source queue
        std::priority_queue<std::pair<real_t, std::array<int, 2>>> queue;
        std::vector<std::pair<real_t, real_t>> obstacles;

        std::vector<std::array<int, 2>> filtered_centroids;

        if (m_centroids.empty()) {
            std::cerr<<"image map too small or too noisy to create rooms"<<std::endl;
            //m_bitmap = {{false}};
            return;
        }

        int max_distance = m_centroids.top().first;

        for (int i = 0; i < m_centroids.size(); i++) {
            if (m_centroids.top().first >= max_distance / 2) {
                filtered_centroids.emplace_back(std::array<int, 2>({m_centroids.top().second[0], m_centroids.top().second[1]}));
            }
            m_centroids.pop();
        }

        //start bfs
        for (std::array<int, 2> centroid: filtered_centroids) {

            queue.push({0, centroid});
            m_rooms.emplace_back();
            m_rooms_count++;
            std::vector<std::vector<bool>> visited(m_map_rooms.size(),std::vector<bool>(m_map_rooms[0].size()));

            //start bfs
            while (!queue.empty()) {

                std::pair<real_t, std::array<int, 2>> elem = queue.top();
                queue.pop();
                std::array<int, 2> const &point = elem.second;
                real_t eu_distance = elem.first;

                if (m_map_rooms[point[1]][point[0]][0] > 0) continue;
                //distances[point[1]][point[0]] = eu_distance;
                m_map_rooms[point[1]][point[0]][0] = m_rooms_count;
                //visited[point[1]][point[0]] = true;
                for (std::array<int, 3> const &d: deltas) {
                    //add queues to add nodes at distance i+2 and i+3
                    int n_x = point[0] + d[0];
                    int n_y = point[1] + d[1];

                    if (n_x >= 0 && n_x < m_bitmap[0].size() && n_y >= 0 && n_y < m_bitmap.size()) {
                        if (m_map_rooms[n_y][n_x][0] <= 0 && !m_bitmap[n_y][n_x])
                            m_map_rooms[n_y][n_x][0] = -m_rooms_count;
                        if (m_bitmap[n_y][n_x] ||
                            (m_map_rooms[n_y][n_x][0] > 0 &&
                             m_map_rooms[n_y][n_x][0] != m_rooms_count)) {
                            if (check_obstacles(obstacles, centroid, n_x, n_y, 1))
                                obstacles.emplace_back(n_x, n_y);
                        } else {
                            real_t n_eu_distance = get_eu_distance(centroid[0], n_x, centroid[1],
                                                                   n_y);
                            if (eu_distance < n_eu_distance &&
                                check_obstacles(obstacles, centroid, n_x, n_y, -1))
                                queue.push({-n_eu_distance, {n_x, n_y}});
                        }
                    }
                }
            }
            //if(rooms_count == 1) break;
        }
    }


    //! @brief Fills m_closest by parsing the bitmap with two sequential bfs
    template<typename obstacle_type, typename obstacle_arg>
    void fill_closest(std::vector<std::vector<index_type>> &closest_map,std::vector<std::vector<obstacle_type>> &obstacles_map,std::function<bool(int, int, obstacle_type, obstacle_arg,std::vector<std::vector<bool>> &)> predicate) {
        constexpr static std::array<std::array<int, 3>, 8> deltas = {{{-1, 0, 2}, {1, 0, 2}, {0, 1, 2}, {0, -1, 2}, {1, 1, 3}, {-1, 1, 3}, {1, -1, 3},{-1, -1, 3}}};
        closest_map = std::vector<std::vector<index_type>>(obstacles_map.size(),std::vector<index_type>(obstacles_map[0].size()));

        for (int obstacle = 1; obstacle >= 0; obstacle--) {
            //dynamic queues vector with source queue
            std::vector<std::vector<matrix_pair_type>> queues(1);
            //new visited matrix
            std::vector<std::vector<bool>> visited(obstacles_map.size(),std::vector<bool>(obstacles_map[0].size()));
            //load source points
            for (size_t r = 0; r < obstacles_map.size(); r++) {
                for (size_t c = 0; c < obstacles_map[0].size(); c++) {
                    if (predicate(c, r, obstacles_map[r][c], obstacle, visited)) {
                        queues[0].emplace_back(std::pair<index_type, index_type>({c, r}, {c, r}));
                    }
                }
            }
            //start bfs
            for (size_t i = 0; i < queues.size(); ++i) {
                for (matrix_pair_type const &elem: queues[i]) {
                    index_type const &point = elem.first;
                    if (!visited[point[1]][point[0]]) {
                        if (i > 0) closest_map[point[1]][point[0]] = elem.second;
                        visited[point[1]][point[0]] = true;
                        for (std::array<int, 3> const &d: deltas) {
                            //add queues to add nodes at distance i+2 and i+3
                            while (i + d[2] >= queues.size()) queues.emplace_back();
                            size_t n_x = point[0] + d[0];
                            size_t n_y = point[1] + d[1];
                            if (n_x >= 0 && n_x < obstacles_map[0].size() && n_y >= 0 && n_y < obstacles_map.size())
                                queues[i + d[2]].push_back({{n_x, n_y}, elem.second});
                        }
                    }
                }
                queues[i].clear();
            }
            //end bfs
        }
    }


    //! @brief Fills m_closest by parsing the bitmap with two sequential bfs
    template<typename obstacle_type, typename obstacle_arg>
    void fill_closest_rooms(std::vector<std::vector<index_type>> &closest_map,std::vector<std::vector<std::vector<obstacle_type>>> &obstacles_map,std::function<bool(int, int, obstacle_type, obstacle_arg,std::vector<std::vector<bool>> &)> predicate) {
        constexpr static std::array<std::array<int, 3>, 8> deltas = {{{-1, 0, 2}, {1, 0, 2}, {0, 1, 2}, {0, -1, 2}, {1, 1, 3}, {-1, 1, 3}, {1, -1, 3},{-1, -1, 3}}};
        closest_map = std::vector<std::vector<index_type>>(obstacles_map.size(),std::vector<index_type>(obstacles_map[0].size()));

        for (int obstacle = 1; obstacle >= 0; obstacle--) {
            //dynamic queues vector with source queue
            std::vector<std::vector<matrix_pair_type>> queues(1);
            //new visited matrix
            std::vector<std::vector<bool>> visited(obstacles_map.size(),std::vector<bool>(obstacles_map[0].size()));
            //load source points
            for (size_t r = 0; r < obstacles_map.size(); r++) {
                for (size_t c = 0; c < obstacles_map[0].size(); c++) {
                    if (predicate(c, r, obstacles_map[r][c][0], obstacle, visited)) {
                        queues[0].emplace_back(std::pair<index_type, index_type>({c, r}, {c, r}));
                    }
                }
            }
            //start bfs
            for (size_t i = 0; i < queues.size(); ++i) {
                for (matrix_pair_type const &elem: queues[i]) {
                    index_type const &point = elem.first;
                    if (!visited[point[1]][point[0]]) {
                        if (i > 0) closest_map[point[1]][point[0]] = elem.second;
                        visited[point[1]][point[0]] = true;
                        for (std::array<int, 3> const &d: deltas) {
                            //add queues to add nodes at distance i+2 and i+3
                            while (i + d[2] >= queues.size()) queues.emplace_back();
                            size_t n_x = point[0] + d[0];
                            size_t n_y = point[1] + d[1];
                            if (n_x >= 0 && n_x < obstacles_map[0].size() && n_y >= 0 && n_y < obstacles_map.size())
                                queues[i + d[2]].push_back({{n_x, n_y}, elem.second});
                        }
                    }
                }
                queues[i].clear();
            }
            //end bfs
        }
    }

    // Implementing floyd warshall algorithm
    void nav_floyd_warshall() {
        int nV = m_navigation_graph.size();
        int i, j, k;

        m_floyd_matrix = std::vector<std::vector<real_t>>(nV, std::vector<real_t>(nV,std::numeric_limits<real_t>::max()));

        for (i = 0; i < nV; i++)
            for (j = 0; j < nV; j++)
                m_floyd_matrix[i][j] = m_navigation_graph[i][j];

        for (k = 0; k < nV; k++) {
            for (i = 0; i < nV; i++) {
                for (j = 0; j < nV; j++) {
                    if (m_floyd_matrix[i][k] + m_floyd_matrix[k][j] < m_floyd_matrix[i][j])
                        m_floyd_matrix[i][j] = m_floyd_matrix[i][k] + m_floyd_matrix[k][j];
                }
            }
        }
    }


    template<typename obstacle_type, typename obstacle_arg>
    std::array<int, 2> start_it_dfs(std::vector<std::vector<index_type>> &closest_map,std::vector<std::vector<obstacle_type>> &obstacles_map,std::function<bool(int, int, obstacle_type,obstacle_arg)> predicate,std::vector<std::vector<bool>> &visited, int row_index,int column_index) {

        constexpr static std::array<std::array<int, 3>, 8> deltas = {{{-1, 0, 2}, {1, 0, 2}, {0, 1, 2}, {0, -1, 2}, {1, 1, 3}, {-1, 1, 3}, {1, -1, 3},{-1, -1, 3}}};
        std::stack<std::pair<std::pair<size_t, size_t>, int>> stack;
        int tollerance = 5;
        int x_sum = 0, y_sum = 0, count = 0;

        stack.push({{column_index, row_index}, tollerance});

        while (!stack.empty()) {

            std::pair<std::pair<size_t, size_t>, int> p = stack.top();
            stack.pop();

            visited[p.first.second][p.first.first] = true;

            if (p.second == 0) continue;

            if (p.second == tollerance) {
                count++;
                x_sum += p.first.first;
                y_sum += p.first.second;
            }

            for (std::array<int, 3> const &d: deltas) {

                size_t n_x = p.first.first + d[0];
                size_t n_y = p.first.second + d[1];

                if (n_x >= 0 && n_x < obstacles_map[0].size() && n_y >= 0 &&
                    n_y < obstacles_map.size() &&
                    !predicate(n_x, n_y, obstacles_map[n_y][n_x], 0) && !visited[n_y][n_x]) {

                    int distance_n = get_distance(closest_map[n_y][n_x], n_x, n_y);
                    bool max = true;
                    for (std::array<int, 3> const &d: deltas) {
                        size_t n_x2 = n_x + d[0];
                        size_t n_y2 = n_y + d[1];
                        if (n_x2 >= 0 && n_x2 < obstacles_map[0].size() && n_y2 >= 0 &&
                            n_y2 < obstacles_map.size() &&
                            !predicate(n_x, n_y, obstacles_map[n_y][n_x], 0)) {
                            int n_distance = get_distance(closest_map[n_y2][n_x2], n_x2, n_y2);
                            max &= distance_n >= n_distance;
                        }
                    }

                    if (max) {
                        stack.push({{n_x, n_y}, tollerance});
                    } else if (p.second > 0) {
                        stack.push({{n_x, n_y}, p.second - 1});
                    }
                }
            }
        }
        std::array<int, 2> barycentre{};
        barycentre[0] = x_sum / count;
        barycentre[1] = y_sum / count;
        return barycentre;
    }

    template<typename obstacle_type, typename obstacle_arg>
    std::array<int, 2> start_it_dfs_rooms(std::vector<std::vector<index_type>> &closest_map,std::vector<std::vector<std::vector<obstacle_type>>> &obstacles_map,std::function<bool(int, int, obstacle_type,obstacle_arg)> predicate,std::vector<std::vector<bool>> &visited, int row_index,int column_index) {

        constexpr static std::array<std::array<int, 3>, 8> deltas = {{{-1, 0, 2}, {1, 0, 2}, {0, 1, 2}, {0, -1, 2}, {1, 1, 3}, {-1, 1, 3}, {1, -1, 3},{-1, -1, 3}}};
        std::stack<std::pair<std::pair<size_t, size_t>, int>> stack;
        int tollerance = 5;
        int x_sum = 0, y_sum = 0, count = 0;

        stack.push({{column_index, row_index}, tollerance});

        while (!stack.empty()) {

            std::pair<std::pair<size_t, size_t>, int> p = stack.top();
            stack.pop();

            visited[p.first.second][p.first.first] = true;

            if (p.second == 0) continue;

            if (p.second == tollerance) {
                count++;
                x_sum += p.first.first;
                y_sum += p.first.second;
            }

            for (std::array<int, 3> const &d: deltas) {

                size_t n_x = p.first.first + d[0];
                size_t n_y = p.first.second + d[1];

                if (n_x >= 0 && n_x < obstacles_map[0].size() && n_y >= 0 &&
                    n_y < obstacles_map.size() &&
                    !predicate(n_x, n_y, obstacles_map[n_y][n_x][0], 0) && !visited[n_y][n_x]) {

                    int distance_n = get_distance(closest_map[n_y][n_x], n_x, n_y);
                    bool max = true;
                    for (std::array<int, 3> const &d: deltas) {
                        size_t n_x2 = n_x + d[0];
                        size_t n_y2 = n_y + d[1];
                        if (n_x2 >= 0 && n_x2 < obstacles_map[0].size() && n_y2 >= 0 &&
                            n_y2 < obstacles_map.size() &&
                            !predicate(n_x, n_y, obstacles_map[n_y][n_x][0], 0)) {
                            int n_distance = get_distance(closest_map[n_y2][n_x2], n_x2, n_y2);
                            max &= distance_n >= n_distance;
                        }
                    }

                    if (max) {
                        stack.push({{n_x, n_y}, tollerance});
                    } else if (p.second > 0) {
                        stack.push({{n_x, n_y}, p.second - 1});
                    }
                }
            }
        }
        std::array<int, 2> barycentre{};
        barycentre[0] = x_sum / count;
        barycentre[1] = y_sum / count;
        return barycentre;
    }

    int m_rooms_count = 0;

    std::vector<std::vector<real_t>> m_floyd_matrix;

    std::vector<std::vector<index_type>> m_closest;

    std::vector<std::vector<index_type>> m_rooms_closest;

    //! @brief Matrix containing each point of a specific room
    std::vector<std::vector<std::pair<index_type, color>>> m_rooms;

    //! @brief Array containing calculated centroids
    std::priority_queue<std::pair<int, std::array<int, 2>>> m_centroids;

    std::map<std::pair<int, int>, std::vector<std::pair<int, int>>> m_waypoint_map;

    using waypoints = std::pair<std::pair<int, int>, index_type>;

    std::vector<waypoints> m_waypoints_list;

    std::vector<std::vector<real_t>> m_navigation_graph;

    /**
     * @brief Bitmap representation
     *
     * a true value means there is an obstacle otherwise false
     */
    std::vector<std::vector<bool>> m_bitmap;

    std::map<int, std::vector<index_type>> m_waypoints_per_rooms;

    //! @brief Array containing calculated barycenters
    std::vector<std::vector<std::vector<int>>> m_map_rooms;

    std::map<index_type, int> m_waypoint_index;

    int m_room_iteration_count;

    bool empty = false;


};

/**
 * @brief Component handling node collision and collision avoidance.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::dimension defines the dimensionality of the space (defaults to 2).
 * - \ref tags::area defines the bounding coordinates of the grid area (defaults to the minimal area covering initial nodes).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::area_min associates to the the minimum coordinates of the grid area (defaults to the value in \ref tags::area).
 * - \ref tags::area_max associates to the the maximum coordinates of the grid area (defaults to the value in \ref tags::area).
 * - \ref tags::navigator associates to the map navigator object (defaults to the empty map).
 */
namespace component {

template<class... Ts>
struct simulated_map {
    //! @brief The dimensionality of the space.
    constexpr static intmax_t dimension = common::option_num<tags::dimension, 2, Ts...>;

    //! @brief Bounding coordinates of the grid area.
    using area = common::option_nums<tags::area, Ts...>;

    static_assert(area::size == 5 or area::size == 0, "the bounding coordinates must be 4 integers");

    /**
     * @brief The actual component.
     *
     * Component functionalities are added to those of the parent by inheritance at multiple levels: the whole component class inherits tag for static checks of correct composition, while `node` and `net` sub-classes inherit actual behaviour.
     * Further parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     * @param P The parent component to inherit from.
     */
    template<typename F, typename P>
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
            template<typename S, typename T>
            net(common::tagged_tuple<S, T> const &t) : P::net(t) {
                static_assert(area::size != 5 or S::template intersect<tags::area_min, tags::area_max>::size != 2, "no option area defined and no area_min and area_max defined either");
                m_map = common::get_or<tags::navigator>(t, m_map);
                if (m_map.is_empty()) {
                    m_viewport_min = to_pos_type(make_vec(0, 0));
                    m_viewport_max = to_pos_type(make_vec(1, 1));
                } else {
                    // variables to avoid linking issues
                    constexpr auto max = details::numseq_to_vec_map<area>::max;
                    constexpr auto min = details::numseq_to_vec_map<area>::min;
                    m_viewport_min = to_pos_type(fcpp::common::get_or<tags::area_min>(t, min));
                    m_viewport_max = to_pos_type(common::get_or<tags::area_max>(t, max));
                }
                position_type viewport_size = m_viewport_max - m_viewport_min;
                m_index_scales = {m_map.get_bitmap_size()[0] / viewport_size[0], m_map.get_bitmap_size()[1] / viewport_size[1]};
                m_index_factors = {viewport_size[0] / m_map.get_bitmap_size()[0], viewport_size[1] / m_map.get_bitmap_size()[1]};
                m_distance_factor = std::sqrt(m_index_factors[0] * m_index_factors[1]);
            }

            //! @brief Returns the position of the empty space closer to the given position.
            position_type closest_space(position_type position) {
                if (m_map.is_empty()) return position;
                if (not is_in_area(position)) crop_to_area(position);
                if (not is_obstacle(position)) return position;
                index_type index = position_to_index(position);
                return index_to_position(m_map.get_closest_from(index), position);
            }

            //! @brief Returns the position of the obstacle closer to the given position.
            position_type closest_obstacle(position_type position) {
                if (m_map.is_empty()) return to_pos_type(make_vec(std::numeric_limits<real_t>::infinity()));
                if (is_obstacle(position)) return position;
                index_type index = position_to_index(position);
                return index_to_position(m_map.get_closest_from(index), position);
            }

            //! @brief Checks whether a given position is an obstacle.
            bool is_obstacle(position_type position) {
                if (m_map.is_empty()) return false;
                if (not is_in_area(position)) return true;
                index_type index = position_to_index(position);
                return m_map.is_obstacle_at(index);
            }

            //! @brief Returns the next waypoint and remaining length after it of a short path from a given source and destination avoiding obstacles.
            std::pair<position_type, real_t> path_to(position_type source, position_type dest, bool debug = false) {
                index_type index_source = position_to_index(source);
                index_type index_dest = position_to_index(dest);
                std::pair<index_type, real_t> next_waypoint = m_map.path_to(index_source, index_dest, debug);
                return {index_to_position(next_waypoint.first, source), next_waypoint.second * m_distance_factor};
            }

          private: // implementation details
            //! @brief Type for representing a bitmap index.
            using index_type = map_navigator::index_type;

            //! @brief Infinitesimal quantity to avoid borders.
            constexpr static real_t epsilon = 1e-6;

            //! @brief Converts a position to the corresponding bitmap index.
            inline index_type position_to_index(position_type const& position) {
                index_type index_to_return;
                // linear scaling
                for (int i = 0; i < 2; i++)
                    index_to_return[i] = static_cast<size_t>(std::min(std::max(std::floor(m_index_scales[i] * (position[i] - m_viewport_min[i])),real_t(0)), m_viewport_max[i] - 1));
                return index_to_return;
            }

            //! @brief Converts a bitmap index to the position within it closer to a given position.
            position_type index_to_position(index_type const& index, position_type position) {
                // linear scaling inverse formula
                for (int i = 0; i < 2; i++) {
                    real_t x = index[i] * m_index_factors[i] + m_viewport_min[i];
                    position[i] = std::min(std::max(position[i], x + epsilon), x + m_index_factors[i] - epsilon);
                }
                return position;
            }

            //! @brief Checks whether a given position is within the map area.
            bool is_in_area(position_type position) {
                for (int i = 0; i < 2; i++)
                    if (position[i] <= m_viewport_min[i] or position[i] >= m_viewport_max[i])
                        return false;
                return true;
            }

            //! @brief Substitutes a given position with the closer one in the map area.
            void crop_to_area(position_type& position) {
                for (int i = 0; i < 2; i++)
                    position[i] = std::min(std::max(position[i], m_viewport_min[i] + epsilon), m_viewport_max[i] - epsilon);
            }

            //! @brief Converts a vector of possibly smaller dimension to a space position.
            template <size_t n>
            position_type to_pos_type(vec<n> const& vec) {
                position_type t;
                for (size_t i = 0; i < vec.dimension; ++i) t[i] = vec[i];
                return t;
            }

            //! @brief Vector of maximum coordinate of the grid area.
            position_type m_viewport_max;
            //! @brief Vector of minimum coordinate of the grid area.
            position_type m_viewport_min;
            //! @brief Array containing cached values of m_index_size / m_viewport_size.
            std::array<real_t, 2> m_index_scales;
            //! @brief Array containing cached values of m_index_size * m_viewport_size.
            std::array<real_t, 2> m_index_factors;
            //! @brief The scale factor for distances.
            real_t m_distance_factor;
            //! @brief The map navigator object.
            map_navigator m_map;
        };
    };
};


}


}

#endif // FCPP_SIMULATED_MAP_H_
