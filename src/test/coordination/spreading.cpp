// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/spreading.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


constexpr hops_t X = std::numeric_limits<hops_t>::max();

template <int O>
DECLARE_OPTIONS(options,
    exports<
        coordination::abf_hops_t,
        coordination::abf_distance_t,
        coordination::bis_distance_t,
        coordination::flex_distance_t,
        coordination::broadcast_t<int, int>,
        coordination::broadcast_t<hops_t, hops_t>
    >,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

template <class...>
struct lagdist {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;

            field<real_t> nbr_dist() {
                return {1};
            }

            field<times_t> nbr_lag() {
                return {1};
            }
        };
        using net = typename P::net;
    };
};
DECLARE_COMBINE(calc_dist, lagdist, component::calculus);

template <int O>
using combo = calc_dist<options<O>>;


field<real_t> nbr_one() {
    return 1;
}


MULTI_TEST(SpreadingTest, ABFH, O, 3) {
    test_net<combo<O>, std::tuple<hops_t>(bool)> n{
        [&](auto& node, bool source){
            return std::make_tuple(
                coordination::abf_hops(node, 0, source)
            );
        }
    };
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     X,      X});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      X});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
}

MULTI_TEST(SpreadingTest, ABFD, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(bool)> n{
        [&](auto& node, bool source){
            return std::make_tuple(
                coordination::abf_distance(node, 0, source)
            );
        }
    };
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     INF,    INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
}

MULTI_TEST(SpreadingTest, ABFM, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(bool)> n{
        [&](auto& node, bool source){
            return std::make_tuple(
                coordination::abf_distance(node, 0, source, nbr_one)
            );
        }
    };
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     INF,    INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
}

MULTI_TEST(SpreadingTest, BISD, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(bool)> n{
        [&](auto& node, bool source){
            return std::make_tuple(
                coordination::bis_distance(node, 0, source, 0, 0)
            );
        }
    };
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     INF,    INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
}

MULTI_TEST(SpreadingTest, BISM, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(bool)> n{
        [&](auto& node, bool source){
            return std::make_tuple(
                coordination::bis_distance(node, 0, source, 0, 0, nbr_one)
            );
        }
    };
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     INF,    INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
}

MULTI_TEST(SpreadingTest, FLEXD, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(bool)> n{
        [&](auto& node, bool source){
            return std::make_tuple(
                coordination::flex_distance(node, 0, source, 0, 1, 0, 0)
            );
        }
    };
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     INF,    INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
}

MULTI_TEST(SpreadingTest, FLEXM, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(bool)> n{
        [&](auto& node, bool source){
            return std::make_tuple(
                coordination::flex_distance(node, 0, source, 0, 1, 0, 0, nbr_one)
            );
        }
    };
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     INF,    INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      INF});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
    EXPECT_ROUND(n, {true,  false,  false},
                    {0,     1,      2});
}

MULTI_TEST(SpreadingTest, Broadcast, O, 3) {
    test_net<combo<O>, std::tuple<int>(int,int)> n{
        [&](auto& node, int dist, int value){
            return std::make_tuple(
                coordination::broadcast(node, 0, dist, value)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2},
                    {0, 1, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2},
                    {0, 0, 1});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2},
                    {0, 0, 0});
}

MULTI_TEST(SpreadingTest, BroadcastSource, O, 3) {
    test_net<combo<O>, std::tuple<int>(hops_t,int)> n{
        [&](auto& node, hops_t dist, hops_t value){
            return std::make_tuple(
                coordination::broadcast(node, 0, dist, value, dist==0, X)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2},
                    {0, X, X});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2},
                    {0, 0, X});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2},
                    {0, 0, 0});
}
