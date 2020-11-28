// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/component/timer.hpp"
#include "lib/coordination/time.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<int, real_t, times_t>,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

DECLARE_COMBINE(calc_only, component::calculus, component::timer);

template <int O>
using combo = calc_only<options<O>>;


MULTI_TEST(TimeTest, Counter, O, 3) {
    test_net<combo<O>, std::tuple<int>()> n{
        [&](auto& node){
            return std::make_tuple(
                coordination::counter(node, 0)
            );
        }
    };
    EXPECT_ROUND(n, {1, 1, 1});
    EXPECT_ROUND(n, {2, 2, 2});
    EXPECT_ROUND(n, {3, 3, 3});
    EXPECT_ROUND(n, {4, 4, 4});
}

MULTI_TEST(TimeTest, ExponentialFilter, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(real_t)> n{
        [&](auto& node, real_t value){
            return std::make_tuple(
                coordination::exponential_filter(node, 0, value, 0.5f)
            );
        }
    };
    EXPECT_ROUND(n, {0,   1,   2},
                    {0,   1,   2});
    EXPECT_ROUND(n, {2,   5,   8},
                    {1,   3,   5});
    EXPECT_ROUND(n, {1,   4,   3},
                    {1,   3.5f,4});
}

MULTI_TEST(TimeTest, SharedFilter, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(real_t)> n{
        [&](auto& node, real_t value){
            return std::make_tuple(
                coordination::shared_filter(node, 0, value, 0.5f)
            );
        }
    };
    EXPECT_ROUND(n, {0,    1,    2},
                    {0,    1,    2});
    EXPECT_ROUND(n, {2,    7,    0},
                    {1.5f, 6,    0.5f});
    EXPECT_ROUND(n, {1.5f, 3,    2.5f},
                    {0,    3.5f, 0.5f});
}

MULTI_TEST(TimeTest, SharedDecay, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(real_t)> n{
        [&](auto& node, real_t value){
            return std::make_tuple(
                coordination::shared_decay(node, 0, 0, value, 0.5f)
            );
        }
    };
    EXPECT_ROUND(n, {2,      4,   6},
                    {1,      2,   3});
    EXPECT_ROUND(n, {2,      4,   6},
                    {1.25f,  3,   4.75f});
    EXPECT_ROUND(n, {2,      4,   6},
                    {1.5625f,3.5f,5.4375f});
}

MULTI_TEST(TimeTest, SharedClock, O, 3) {
    test_net<combo<O>, std::tuple<bool>()> n{
        [&](auto& node){
            return std::make_tuple(
                coordination::shared_clock(node, 0) < 0
            );
        }
    };
    EXPECT_ROUND(n, {true, true, true});
    EXPECT_ROUND(n, {true, true, true});
    EXPECT_ROUND(n, {true, true, true});
}
