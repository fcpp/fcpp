// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "lib/fcpp.hpp"

#include "project/sample/collection_compare.hpp"

using namespace fcpp;
using namespace component::tags;
using namespace coordination::tags;

#define ALGO        1
#define DEVICE_NUM  1000
#define END_TIME    500
#define MAXX        2000
#define MAXY        200

using spawn_s = random::sequence_multiple<random::constant_distribution<times_t, 0>, DEVICE_NUM>;

using round_s = random::sequence_periodic<random::interval_d<times_t, 0, 1>, random::weibull_d<times_t, 100, 25, 100>, random::constant_distribution<times_t, END_TIME>>;

using export_s = random::sequence_periodic<random::constant_distribution<times_t, 0>, random::constant_distribution<times_t, 10>, random::constant_distribution<times_t, END_TIME>>;

using rectangle_d = random::array_distribution<random::interval_d<double, 0, MAXX>, random::interval_d<double, 0, MAXY>>;

using combo = component::combine<
    component::calculus<program<main>, exports<
        device_t, double, field<double>, std::array<double, 2>,
        tuple<double,device_t>, tuple<double,int>, tuple<double,double>
    >>,
    component::exporter<value_push<false>, log_schedule<export_s>, aggregators<
        spc_sum,    aggregator::sum<double>,
        mpc_sum,    aggregator::sum<double>,
        wmpc_sum,   aggregator::sum<double>,
        ideal_sum,  aggregator::sum<double>,
        spc_max,    aggregator::max<double>,
        mpc_max,    aggregator::max<double>,
        wmpc_max,   aggregator::max<double>,
        ideal_max,  aggregator::max<double>
    >>,
    component::storage<
        algorithm,  int,
        spc_sum,    double,
        mpc_sum,    double,
        wmpc_sum,   double,
        ideal_sum,  double,
        spc_max,    double,
        mpc_max,    double,
        wmpc_max,   double,
        ideal_max,  double>,
    component::spawner<spawn_s,
        x,          rectangle_d,
        algorithm,  random::constant_distribution<int, ALGO>>,

    component::physical_connector<connector::fixed<100>>,
    component::physical_position<>,
    component::timer,
    component::scheduler<round_s>,
    component::identifier<synchronised<false>>,
    component::randomizer<>
>;

int main() {
    combo::net network{common::make_tagged_tuple<>()};
    network.run();
    return 0;
}
