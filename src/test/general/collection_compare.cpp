// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/fcpp.hpp"

#include "test/test_net.hpp"
#include "test/general/collection_compare.hpp"

using namespace fcpp;
using namespace coordination::tags;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    program<main>,
    round_schedule<sequence::list<distribution::constant<times_t, 100>>>,
    log_schedule<sequence::list<distribution::constant<times_t, 100>>>,
    exports<
        device_t, double, field<double>, vec<2>,
        tuple<double,device_t>, tuple<double,int>, tuple<double,double>
    >,
    tuple_store<
        algorithm,  int,
        spc_sum,    double,
        mpc_sum,    double,
        wmpc_sum,   double,
        ideal_sum,  double,
        spc_max,    double,
        mpc_max,    double,
        wmpc_max,   double,
        ideal_max,  double
    >,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>,
    parallel<(O & 8) == 8>,
    synchronised<(O & 16) == 16>
);
template <int O>
using combo = component::batch_simulator<options<O>>;


MULTI_TEST(CollectionCompareTest, ShortLine, O, 5) {
    test_net<combo<O>, std::tuple<double>()> n{
        [&](auto& node){
            node.round_main(0.0);
            return std::make_tuple(
                node.storage(ideal_sum{})
            );
        }
    };
    EXPECT_ROUND(n, {1, 1, 1});
    EXPECT_ROUND(n, {1, 1, 1});
    EXPECT_ROUND(n, {1, 1, 1});
    EXPECT_ROUND(n, {1, 1, 1});
    EXPECT_ROUND(n, {1, 1, 1});
}
