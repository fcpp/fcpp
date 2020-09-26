// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/component/randomizer.hpp"
#include "lib/coordination/geometry.hpp"
#include "lib/simulation/simulated_positioner.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<vec<2>>,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

DECLARE_COMBINE(calc_pos, component::calculus, component::simulated_positioner, component::randomizer);

template <int O>
using combo = calc_pos<options<O>>;


TEST(GeometryTest, Target) {
    test_net<combo<0>> n;
    n.d(2).position() = make_vec(1,1);
    vec<2> p{0,0};
    for (int i=0; i<10000; ++i) {
        vec<2> q = coordination::random_rectangle_target(n.d(0), 0, make_vec(1,2), make_vec(3,8));
        EXPECT_TRUE(1 <= q[0] and q[0] <= 3);
        EXPECT_TRUE(2 <= q[1] and q[1] <= 8);
        p += q;
    }
    p /= 10000;
    EXPECT_LT(distance(p, make_vec(2,5)), 0.1);
    p = {0,0};
    for (int i=0; i<10000; ++i) {
        vec<2> q = coordination::random_rectangle_target(n.d(2), 0, make_vec(1,2), make_vec(3,8), 2);
        EXPECT_TRUE(1 <= q[0] and q[0] <= 3);
        EXPECT_TRUE(2 <= q[1] and q[1] <= 3);
        p += q;
    }
    p /= 10000;
    EXPECT_LT(distance(p, make_vec(2,2.5)), 0.1);
}

MULTI_TEST(GeometryTest, Follow, O, 3) {
    {
        test_net<combo<O>, std::tuple<double>(double), 1> n{
            [&](auto& node, double val){
                return std::make_tuple(
                    coordination::follow_target(node, 0, make_vec(val, 0), 3, 1)
                );
            }
        };
        EXPECT_ROUND(n, {10}, {10});
        EXPECT_ROUND(n, {10}, {7});
        EXPECT_ROUND(n, {10}, {4});
        EXPECT_ROUND(n, {10}, {1});
        EXPECT_ROUND(n, {10}, {0});
        EXPECT_ROUND(n, {10}, {0});
    }
    vec<2> target;
    test_net<combo<O>, std::tuple<double>(), 1> n{
        [&](auto& node){
            return std::make_tuple(
                coordination::follow_target(node, 0, target, 3, 1, 1)
            );
        }
    };
    for (int i=0; i<1000; ++i) {
        target = coordination::random_rectangle_target(n.d(0), 0, -make_vec(4,3), make_vec(4,3));
        int c = 0;
        for (double d = INF; d > 1; d = get<0>(n.full_round({}))[0]) ++c;
        EXPECT_LT(c, 16);
    }
}

MULTI_TEST(GeometryTest, Walk, O, 3) {
    test_net<combo<O>, std::tuple<bool, bool, bool, bool>(), 1> n{
        [&](auto& node){
            vec<2> t = coordination::rectangle_walk(node, 0, make_vec(0,0), make_vec(3,3), 1, 0.5, 1);
            vec<2> p = node.position();
            return std::make_tuple(
                0 <= t[0] and t[0] <= 3 and 0 <= t[1] and t[1] <= 3,
                0 <= p[0] and p[0] <= 3 and 0 <= p[1] and p[1] <= 3,
                norm(node.velocity()) <= 0.5000001,
                node.propulsion() == make_vec(0,0) and node.friction() == 0.0
            );
        }
    };
    for (int i=0; i<10000; ++i)
        EXPECT_ROUND(n, {true}, {true}, {true}, {true});
}
