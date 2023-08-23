// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/utils.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<int, real_t>,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

DECLARE_COMBINE(calc_only, component::calculus);

template <int O>
using combo = calc_only<options<O>>;


TEST(UtilsTest, BasicFunctions) {
    field<int> fi1 = details::make_field<int>({1,3},{2,1,-1});
    field<int> fi2 = details::make_field<int>({1,2},{1,4,3});
    field<bool> fb1 = details::make_field<bool>({2,3},{true,false,true});
    field<bool> fb2 = details::make_field<bool>({1,2},{false,true,true});
    field<int> x;
    x = mux(true, fi1, fi2);
    EXPECT_EQ(x, fi1);
    x = mux(false, field<int>(fi1), field<int>(fi2));
    EXPECT_EQ(x, fi2);
    x = mux(fb1, fi1, fi2);
    field<int> y = details::make_field<int>({1,2,3},{2,1,3,-1});
    EXPECT_EQ(x, y);
    tuple<field<int>, int> a{fi1, 1};
    tuple<field<int>, int> b{fi2, 2};
    field<tuple<int,int>> c = mux(fb2, a, b);
    field<tuple<int,int>> d = details::make_field<tuple<int,int>>({1,2,3},
        {make_tuple(1, 2), make_tuple(1, 1), make_tuple(2, 1), make_tuple(1, 2)});
    EXPECT_EQ(c, d);
    c = max(a, b);
    d = details::make_field<tuple<int,int>>({1,2,3},
        {make_tuple(2, 1), make_tuple(4, 2), make_tuple(3, 2), make_tuple(1, 2)});
    EXPECT_EQ(c, d);
    c = max(a, c);
    EXPECT_EQ(c, d);
    c = max(a, make_tuple(1, 2));
    d = details::make_field<tuple<int,int>>({1,3},
        {make_tuple(2, 1), make_tuple(1, 2), make_tuple(1, 2)});
    EXPECT_EQ(c, d);
    c = mux(true, a, make_tuple(1,2));
    d = details::make_field<tuple<int,int>>({1,3},
        {make_tuple(2, 1), make_tuple(1, 1), make_tuple(-1, 1)});
    EXPECT_EQ(c, d);
    c = min(a, b);
    d = details::make_field<tuple<int,int>>({1,2,3},
        {make_tuple(1, 2), make_tuple( 1, 1), make_tuple( 2, 1), make_tuple(-1, 1)});
    EXPECT_EQ(c, d);
    x = get<0>(d);
    y = details::make_field<int>({1,2,3}, {1,1,2,-1});
    EXPECT_EQ(x,y);
    x = get<1>(d);
    y = details::make_field<int>({1,2,3}, {2,1,1,1});
    EXPECT_EQ(x,y);
}

MULTI_TEST(UtilsTest, IsInf, O, 3) {
    test_net<combo<O>, std::tuple<bool>(real_t)> n{
        [&](auto& node, real_t value){
            return std::make_tuple(
                coordination::max_hood(node, 0, isinf(coordination::nbr(node, 0, value)))
            );
        }
    };
    EXPECT_ROUND(n, {0,     1,     INF},
                    {false, false, true});
    EXPECT_ROUND(n, {0,     1,     INF},
                    {false, true,  true});
    EXPECT_ROUND(n, {0,     1,     INF},
                    {false, true,  true});
}

MULTI_TEST(UtilsTest, SumHood, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::sum_hood(node, 0, coordination::nbr(node, 0, value))
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

MULTI_TEST(UtilsTest, MeanHood, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(real_t)> n{
        [&](auto& node, real_t value){
            return std::make_tuple(
                coordination::mean_hood(node, 0, coordination::nbr(node, 0, value))
            );
        }
    };
    EXPECT_ROUND(n, {1,   2,   6},
                    {1,   2,   6});
    EXPECT_ROUND(n, {1,   2,   6},
                    {1.5f,3,   4});
    EXPECT_ROUND(n, {1,   2,   6},
                    {1.5f,3,   4});
}

MULTI_TEST(UtilsTest, ListHood, O, 3) {
    test_net<combo<O>, std::tuple<real_t,real_t>(real_t)> n{
        [&](auto& node, real_t value){
            std::vector<real_t> v;
            v = coordination::list_hood(node, 0, std::vector<real_t>(), coordination::nbr(node, 0, value), coordination::nothing);
            real_t r = 0;
            for (real_t x : v) r += x;
            v = coordination::list_hood(node, 0, std::vector<real_t>(), coordination::nbr(node, 1, value));
            real_t s = 0;
            for (real_t x : v) s += x;
            return std::make_tuple(r, s);
        }
    };
    EXPECT_ROUND(n, {1,   2,   6},
                    {0,   0,   0},
                    {1,   2,   6});
    EXPECT_ROUND(n, {1,   3,   6},
                    {2,   7,   2},
                    {3,   9,   8});
    EXPECT_ROUND(n, {1,   4,   6},
                    {3,   7,   3},
                    {4,   10,  9});
}
