// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "lib/fcpp.hpp"
#include "lib/simulation/displayer.hpp"

#define EXAMPLE_3D

//! @brief Minimum number whose square is at least n.
constexpr size_t discrete_sqrt(size_t n) {
    size_t lo = 0, hi = n, mid = 0;
    while (lo < hi) {
        mid = (lo + hi)/2;
        if (mid*mid < n) lo = mid+1;
        else hi = mid;
    }
    return lo;
}

constexpr size_t devices = 1000;
constexpr size_t comm = 100;
constexpr size_t side = discrete_sqrt(devices * 3000);
constexpr size_t height = 100;

constexpr float hue_scale = 360.0f/(side+height);


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

namespace tags {
    //! @brief Distance of the current node.
    struct my_distance {};
    //! @brief Diameter of the network (in the source).
    struct source_diameter {};
    //! @brief Diameter of the network (in every node).
    struct diameter {};
    //! @brief Color representing the distance of the current node.
    struct distance_c {};
    //! @brief Color representing the diameter of the network (in the source).
    struct source_diameter_c {};
    //! @brief Color representing the diameter of the network (in every node).
    struct diameter_c {};
    //! @brief Size of the current node.
    struct size {};
}

//! @brief Main function.
MAIN() {
#ifdef EXAMPLE_3D
    rectangle_walk(CALL, make_vec(0,0,0), make_vec(side,side,height), comm/3, 1);
#else
    rectangle_walk(CALL, make_vec(0,0), make_vec(side,side), comm/3, 1);
#endif
    device_t source_id = ((int)node.current_time()) / 50;
    bool is_source = node.uid == source_id;
    node.storage(tags::size{}) = is_source ? 20 : 10;
    double dist = abf_distance(CALL, is_source);
    double sdiam = mp_collection(CALL, dist, dist, 0.0, [](double x, double y){
        return max(x, y);
    }, [](double x, int){
        return x;
    });
    double diam = broadcast(CALL, dist, sdiam);
    node.storage(tags::my_distance{}) = dist;
    node.storage(tags::source_diameter{}) = sdiam;
    node.storage(tags::diameter{}) = diam;
    node.storage(tags::distance_c{}) = color::hsva(dist*hue_scale, 1, 1);
    node.storage(tags::source_diameter_c{}) = color::hsva(sdiam*hue_scale, 1, 1);
    node.storage(tags::diameter_c{}) = color::hsva(diam*hue_scale, 1, 1);
}

}


//! @brief Namespace for all FCPP components.
namespace component {

/**
 * @brief Combination of components for interactive simulations.
 *
 * It can be instantiated as `interactive_simulator<options...>::net`.
 */
DECLARE_COMBINE(interactive_simulator, displayer, calculus, simulated_connector, simulated_positioner, timer, scheduler, logger, storage, spawner, identifier, randomizer);

}

}

using namespace fcpp;
using namespace component::tags;
using namespace coordination::tags;

using round_s = sequence::periodic<
    distribution::interval_n<times_t, 0, 1>,
    distribution::weibull_n<times_t, 10, 1, 10>
>;

#ifdef EXAMPLE_3D
using rectangle_d = distribution::rect_n<1, 0, 0, 0, side, side, height>;
constexpr size_t dim = 3;
#else
using rectangle_d = distribution::rect_n<1, 0, 0, side, side>;
constexpr size_t dim = 2;
#endif

DECLARE_OPTIONS(opt,
    parallel<true>,
    synchronised<false>,
    program<coordination::main>,
    round_schedule<round_s>,
    dimension<dim>,
    exports<vec<dim>, double>,
    log_schedule<sequence::periodic_n<1, 0, 1>>,
    tuple_store<
        my_distance,        double,
        source_diameter,    double,
        diameter,           double,
        distance_c,         color,
        source_diameter_c,  color,
        diameter_c,         color,
        size,               double
    >,
    spawn_schedule<sequence::multiple_n<devices, 0>>,
    init<x, rectangle_d>,
    connector<connect::fixed<comm, 1, dim>>,
    size_tag<size>,
    color_tag<distance_c,source_diameter_c,diameter_c>
);

int main() {
    component::interactive_simulator<opt>::net network{common::make_tagged_tuple<epsilon>(0.1)};
    network.run();
    return 0;
}
