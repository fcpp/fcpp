// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/fcpp.hpp"

#include "test/test_net.hpp"
#include "test/general/collection_compare.hpp"

using namespace fcpp;
using namespace coordination::tags;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    program<coordination::main>,
    round_schedule<sequence::list<distribution::constant_n<times_t, 100>>>,
    log_schedule<sequence::list<distribution::constant_n<times_t, 100>>>,
    exports<coordination::main_t>,
//        device_t, real_t, field<real_t>, vec<2>,
//        tuple<real_t,device_t>, tuple<real_t,int>, tuple<real_t,real_t>
//    >,
    tuple_store<
        algorithm,  int,
        spc_sum,    real_t,
        mpc_sum,    real_t,
        wmpc_sum,   real_t,
        ideal_sum,  real_t,
        spc_max,    real_t,
        mpc_max,    real_t,
        wmpc_max,   real_t,
        ideal_max,  real_t
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
    test_net<combo<O>, std::tuple<real_t>()> n{
        [&](auto& node){
            node.round_main(0);
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
