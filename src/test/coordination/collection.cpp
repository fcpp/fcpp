// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/collection.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<
        coordination::gossip_max_t<int>,
        coordination::gossip_mean_t<real_t>,
        coordination::sp_collection_t<int,real_t>,
        coordination::mp_collection_t<int,real_t>,
        coordination::wmp_collection_t<real_t>
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

            field<real_t> nbr_lag() {
                return {1};
            }
        };
        using net = typename P::net;
    };
};
DECLARE_COMBINE(calc_dist, lagdist, component::calculus);

template <int O>
using combo = calc_dist<options<O>>;


real_t adder(real_t x, real_t y) {
    return x+y;
}

real_t divider(real_t x, size_t n) {
    return x/n;
}

real_t multiplier(real_t x, real_t f) {
    return x*f;
}


MULTI_TEST(CollectionTest, Gossip, O, 3) {
    {
        test_net<combo<O>, std::tuple<int>(int)> n{
            [&](auto& node, int val){
                return std::make_tuple(
                    coordination::gossip_max(node, 0, val)
                );
            }
        };
        EXPECT_ROUND(n, {0, 1, 2},
                        {0, 1, 2});
        EXPECT_ROUND(n, {0, 1, 0},
                        {1, 2, 2});
        EXPECT_ROUND(n, {0, 0, 0},
                        {2, 2, 2});
        EXPECT_ROUND(n, {0, 3, 0},
                        {2, 3, 2});
        EXPECT_ROUND(n, {0, 3, 0},
                        {3, 3, 3});
    }
    {
        test_net<combo<O>, std::tuple<real_t>(real_t)> n{
            [&](auto& node, real_t val){
                return std::make_tuple(
                    coordination::gossip_mean(node, 0, val)
                );
            }
        };
        EXPECT_ROUND(n, {0, 4, 8},
                        {0, 4, 8});
        EXPECT_ROUND(n, {0, 4, 8},
                        {2, 4, 6});
        EXPECT_ROUND(n, {6, 4, 2},
                        {5, 4, 3});
    }
}

MULTI_TEST(CollectionTest, SP, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(int, real_t)> n{
        [&](auto& node, int id, real_t val){
            return std::make_tuple(
                coordination::sp_collection(node, 0, id, val, 0, adder)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {3, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
}

MULTI_TEST(CollectionTest, MP, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(int, real_t)> n{
        [&](auto& node, int id, real_t val){
            return std::make_tuple(
                coordination::mp_collection(node, 0, id, val, 0, adder, divider)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {3, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
}

MULTI_TEST(CollectionTest, WMP, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(int, real_t)> n{
        [&](auto& node, int id, real_t val){
            return std::make_tuple(
                coordination::wmp_collection(node, 0, id, 2, val, adder, multiplier)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {3, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
}
