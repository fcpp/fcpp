// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"
#include "lib/simulation/simulated_positioner.hpp"

#include "test/test_net.hpp"
#include "test/general/slow_distance.hpp"

using namespace fcpp;
using namespace coordination::tags;
using namespace component::tags;


struct mytimer {
    template <typename F, typename P>
    struct component : public P {
        DECLARE_COMPONENT(timer);
        struct node : public P::node {
            using P::node::node;
            field<times_t> const& nbr_lag() const {
                return m_nl;
            }
            field<times_t> m_nl = field<times_t>(1);
        };
        using net  = typename P::net;
    };
};

template <int O>
using combo = component::combine_spec<
    component::simulated_positioner<>,
    mytimer,
    component::storage<tuple_store<idealdist, real_t, fastdist, real_t, slowdist, real_t, fasterr, real_t, slowerr, real_t>>,
    component::identifier<parallel<(O & 8) == 8>, synchronised<(O & 16) == 16>>,
    component::calculus<
        program<main>,
        exports<coordination::distance_compare_t, coordination::connection_t>,
        export_pointer<(O & 1) == 1>,
        export_split<(O & 2) == 2>,
        online_drop<(O & 4) == 4>
    >,
    component::base<parallel<(O & 8) == 8>>
>;


MULTI_TEST(SlowdistanceTest, ShortLine, O, 5) {
    test_net<combo<O>, std::tuple<real_t,real_t,real_t>()> n{
        [&](auto& node){
            node.round_main(0);
            return std::make_tuple(
                node.storage(idealdist{}),
                node.storage( fastdist{}),
                node.storage( slowdist{})
            );
        }
    };
    EXPECT_ROUND(n,
        {0,   1,   1.5f},
        {0,   INF, INF},
        {0,   INF, INF}
    );
    EXPECT_ROUND(n,
        {0,   1,   1.5f},
        {0,   1,   INF},
        {0,   INF, INF},
    );
    EXPECT_ROUND(n,
        {0,   1,   1.5f},
        {0,   1,   1.5f},
        {0,   1,   INF},
    );
    EXPECT_ROUND(n,
        {0,   1,   1.5f},
        {0,   1,   1.5f},
        {0,   1,   INF},
    );
    EXPECT_ROUND(n,
        {0,   1,   1.5f},
        {0,   1,   1.5f},
        {0,   1,   1.5f},
    );
}

TEST(SlowdistanceTest, Connection) {
    test_net<combo<0>, std::tuple<int>()> n{
        [&](auto& node){
            return std::make_tuple(
                coordination::sum_hood(node, 0, coordination::connection(node, 0))
            );
        }
    };
    EXPECT_ROUND(n, {1, 1,  1});
    EXPECT_ROUND(n, {3, 4,  3});
    EXPECT_ROUND(n, {5, 7,  5});
    EXPECT_ROUND(n, {7, 10, 7});
    EXPECT_ROUND(n, {9, 13, 9});
}
