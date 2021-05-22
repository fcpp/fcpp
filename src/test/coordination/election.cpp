// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/election.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<
        coordination::diameter_election_t<int>,
        coordination::wave_election_t<int>,
        coordination::color_election_t<int>
    >,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

DECLARE_COMBINE(calc_only, component::calculus);

template <int O>
using combo = calc_only<options<O>>;


MULTI_TEST(ElectionTest, Diameter, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::diameter_election(node, 0, value, 3)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 1});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {9, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {9, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {9, 1, 2},
                    {9, 0, 2});
    EXPECT_ROUND(n, {9, 1, 2},
                    {9, 1, 2});
    EXPECT_ROUND(n, {9, 1, 2},
                    {1, 1, 1});
}

MULTI_TEST(ElectionTest, Wave, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::wave_election(node, 0, value, [](int x){ return x+1; })
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 1});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {9, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {9, 1, 2},
                    {0, 1, 0});
    EXPECT_ROUND(n, {9, 1, 2},
                    {1, 1, 1});
    EXPECT_ROUND(n, {9, 1, 2},
                    {1, 1, 1});
    EXPECT_ROUND(n, {9, 1, 2},
                    {1, 1, 1});
    EXPECT_ROUND(n, {9, 8, 2},
                    {1, 1, 1});
    EXPECT_ROUND(n, {9, 8, 2},
                    {9, 1, 2});
    EXPECT_ROUND(n, {9, 8, 2},
                    {9, 2, 2});
    EXPECT_ROUND(n, {9, 8, 2},
                    {2, 2, 2});
}

MULTI_TEST(ElectionTest, Color, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::color_election(node, 0, value)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 1, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 1});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 0});
    EXPECT_ROUND(n, {0, 1, 2},
                    {0, 0, 0});
}
