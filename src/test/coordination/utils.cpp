// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/utils.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<int, double>,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

DECLARE_COMBINE(calc_only, component::calculus);

template <int O>
using combo = calc_only<options<O>>;


MULTI_TEST(UtilsTest, IsInf, O, 3) {
    test_net<combo<O>, std::tuple<bool>(double)> n{
        [&](auto& node, double value){
            return std::make_tuple(
                coordination::max_hood(node, 0, coordination::isinf(nbr(node, 0, value)))
            );
        }
    };
    EXPECT_ROUND(n, {0,     1,     1.0/0.0},
                    {false, false, true});
    EXPECT_ROUND(n, {0,     1,     1.0/0.0},
                    {false, true,  true});
    EXPECT_ROUND(n, {0,     1,     1.0/0.0},
                    {false, true,  true});
}

MULTI_TEST(UtilsTest, SumHood, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::sum_hood(node, 0, nbr(node, 0, value))
            );
        }
    };
    EXPECT_ROUND(n, {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {1, 2, 4},
                    {3, 7, 6});
    EXPECT_ROUND(n, {1, 2, 4},
                    {3, 7, 6});
}

MULTI_TEST(UtilsTest, Counter, O, 3) {
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
