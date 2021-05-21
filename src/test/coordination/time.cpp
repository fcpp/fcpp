// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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
    exports<
        coordination::counter_t<>,
        coordination::round_since_t,
        coordination::constant_t<int>,
        coordination::toggle_t,
        coordination::toggle_filter_t,
        coordination::delay_t<int>,
        coordination::exponential_filter_t<real_t>,
        coordination::shared_filter_t<real_t>,
        coordination::shared_decay_t<real_t>,
        coordination::shared_clock_t
    >,
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

MULTI_TEST(TimeTest, RoundSince, O, 3) {
    test_net<combo<O>, std::tuple<int>(bool)> n{
        [&](auto& node, bool value){
            return std::make_tuple(
                coordination::round_since(node, 0, value)
            );
        }
    };
    EXPECT_ROUND(n, {false, false, true},
                    {1,     1,     0});
    EXPECT_ROUND(n, {false, true,  false},
                    {2,     0,     1});
    EXPECT_ROUND(n, {false, true,  false},
                    {3,     0,     2});
    EXPECT_ROUND(n, {false, false, true},
                    {4,     1,     0});
}

MULTI_TEST(TimeTest, Constant, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::constant(node, 0, value)
            );
        }
    };
    EXPECT_ROUND(n, {0,   1,   2},
                    {0,   1,   2});
    EXPECT_ROUND(n, {2,   5,   8},
                    {0,   1,   2});
    EXPECT_ROUND(n, {1,   4,   3},
                    {0,   1,   2});
}

MULTI_TEST(TimeTest, Toggle, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::toggle(node, 0, value)
            );
        }
    };
    EXPECT_ROUND(n, {0,   1,   0},
                    {0,   1,   0});
    EXPECT_ROUND(n, {0,   1,   1},
                    {0,   0,   1});
    EXPECT_ROUND(n, {1,   1,   0},
                    {1,   1,   1});
}

MULTI_TEST(TimeTest, ToggleFilter, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::toggle_filter(node, 0, value)
            );
        }
    };
    EXPECT_ROUND(n, {0,   1,   0},
                    {0,   1,   0});
    EXPECT_ROUND(n, {0,   1,   1},
                    {0,   1,   1});
    EXPECT_ROUND(n, {1,   0,   0},
                    {1,   1,   1});
    EXPECT_ROUND(n, {0,   1,   1},
                    {1,   0,   0});
}

MULTI_TEST(TimeTest, FixedDelay, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::delay(node, 0, value, 2)
            );
        }
    };
    EXPECT_ROUND(n, {0,   0,   0},
                    {0,   0,   0});
    EXPECT_ROUND(n, {1,   2,   3},
                    {0,   0,   0});
    EXPECT_ROUND(n, {2,   1,   0},
                    {0,   0,   0});
    EXPECT_ROUND(n, {2,   1,   0},
                    {1,   2,   3});
    EXPECT_ROUND(n, {2,   1,   0},
                    {2,   1,   0});
}

MULTI_TEST(TimeTest, VariableDelay, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::delay(node, 0, coordination::counter(node, 1), value)
            );
        }
    };
    EXPECT_ROUND(n, {0,   0,   0},
                    {1,   1,   1});
    EXPECT_ROUND(n, {0,   1,   2},
                    {2,   1,   1});
    EXPECT_ROUND(n, {0,   2,   2},
                    {3,   1,   1});
    EXPECT_ROUND(n, {1,   2,   1},
                    {3,   2,   3});
    EXPECT_ROUND(n, {1,   1,   1},
                    {4,   4,   4});
    EXPECT_ROUND(n, {0,   0,   0},
                    {6,   6,   6});
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
