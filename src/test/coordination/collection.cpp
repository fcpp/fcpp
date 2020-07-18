// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/collection.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<int, double, device_t, field<double>, tuple<int, device_t>>,
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

            field<double> nbr_dist() {
                return {1.0};
            }

            field<double> nbr_lag() {
                return {1.0};
            }
        };
        using net = typename P::net;
    };
};
DECLARE_COMBINE(calc_dist, lagdist, component::calculus);

template <int O>
using combo = calc_dist<options<O>>;


double adder(double x, double y) {
    return x+y;
}

double divider(double x, size_t n) {
    return x/n;
}

double multiplier(double x, double f) {
    return x*f;
}


MULTI_TEST(CollectionTest, SP, O, 3) {
    test_net<combo<O>, std::tuple<double>(int, double)> n{
        [&](auto& node, int id, double val){
            return std::make_tuple(
                coordination::sp_collection(node, 0, id, val, 0.0, adder)
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
    test_net<combo<O>, std::tuple<double>(int, double)> n{
        [&](auto& node, int id, double val){
            return std::make_tuple(
                coordination::mp_collection(node, 0, id, val, 0.0, adder, divider)
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
    test_net<combo<O>, std::tuple<double>(int, double)> n{
        [&](auto& node, int id, double val){
            return std::make_tuple(
                coordination::wmp_collection(node, 0, id, 2.0, val, adder, multiplier)
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
