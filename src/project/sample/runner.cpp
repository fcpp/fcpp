// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "lib/fcpp.hpp"

#include "project/sample/slowdistance.hpp"

using namespace fcpp;
using namespace coordination::tags;

#define DEVICE_NUM 10000
#define ROUND_NUM  50
#define AREA_SIZE  30

using spawn_s = random::sequence_multiple<random::constant_distribution<times_t, 0>, DEVICE_NUM>;

using round_s = random::sequence_periodic<random::constant_distribution<times_t, 1>, random::constant_distribution<times_t, 2>, random::constant_distribution<times_t, 2*ROUND_NUM>>;

using export_s = random::sequence_periodic<random::constant_distribution<times_t, 2>, random::constant_distribution<times_t, 2>, random::constant_distribution<times_t, 2*ROUND_NUM+1>>;

using rectangle_d = random::array_distribution<random::interval_d<double, 0, AREA_SIZE>, random::interval_d<double, 0, AREA_SIZE>>;

using combo = component::combine<
    coordination::slowdistance,
    coordination::spreading,
    component::calculus<metric::once, double>,

    component::physical_connector<random::constant_distribution<times_t, 1, 4>, connector::fixed<1>>,
    component::physical_position<>,

    component::exporter<false,export_s,fasterr,aggregator::stats<double>,slowerr,aggregator::stats<double>>,
    component::storage<idealdist, double, fastdist, double, slowdist, double, fasterr, double, slowerr, double>,

    component::scheduler<round_s>,
    component::spawner<spawn_s, component::tags::x, rectangle_d>,
    component::identifier<true>,
    component::randomizer<>
>;

int main() {
    combo::net network{common::make_tagged_tuple<>()};
    network.run();
    return 0;
}
