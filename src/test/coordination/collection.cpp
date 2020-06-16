// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/collection.hpp"

using namespace fcpp;

constexpr int I = std::numeric_limits<int>::max()-1;

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

using combo1 = component::combine<
    lagdist,
    component::calculus<component::tags::exports<int, double, device_t, field<double>, tuple<int, device_t>>>
>;

struct testnet {
    testnet() : network{common::make_tagged_tuple<>()},
        d0{network, common::make_tagged_tuple<component::tags::uid>(0)},
        d1{network, common::make_tagged_tuple<component::tags::uid>(1)},
        d2{network, common::make_tagged_tuple<component::tags::uid>(2)} {
        topology.push_back({0, 1});
        topology.push_back({0, 1, 2});
        topology.push_back({1, 2});
    }

    combo1::node& d(int id) {
        if (id == 0) return d0;
        if (id == 1) return d1;
        if (id == 2) return d2;
        return d0;
    }

    void newround() {
        for (int i = 0; i < 3; ++i) d(i).round_end(0.0);
        for (int source = 0; source < 3; ++source)
            for (int dest : topology[source]) {
                typename combo1::node::message_t m;
                d(dest).receive(0.0, source, d(source).send(0.0, dest, m));
            }
        for (int i = 0; i < 3; ++i) d(i).round_start(0.0);
    }
    
    combo1::net  network;
    combo1::node d0, d1, d2;
    std::vector<std::vector<int>> topology;
};

double adder(double x, double y) {
    return x+y;
}

double divider(double x, size_t n) {
    return x/n;
}

double multiplier(double x, double f) {
    return x*f;
}


TEST(CollectionTest, SP) {
    testnet n;
    auto collect = [&](int node, double value){
        return coordination::sp_collection(n.d(node), 0, node, value, 0.0, adder);
    };
    EXPECT_EQ(1, collect(0, 1));
    EXPECT_EQ(2, collect(1, 2));
    EXPECT_EQ(4, collect(2, 4));
    n.newround();
    EXPECT_EQ(1, collect(0, 1));
    EXPECT_EQ(2, collect(1, 2));
    EXPECT_EQ(4, collect(2, 4));
    n.newround();
    EXPECT_EQ(3, collect(0, 1));
    EXPECT_EQ(6, collect(1, 2));
    EXPECT_EQ(4, collect(2, 4));
    n.newround();
    EXPECT_EQ(7, collect(0, 1));
    EXPECT_EQ(6, collect(1, 2));
    EXPECT_EQ(4, collect(2, 4));
    n.newround();
    EXPECT_EQ(7, collect(0, 1));
    EXPECT_EQ(6, collect(1, 2));
    EXPECT_EQ(4, collect(2, 4));
}

TEST(CollectionTest, MP) {
    testnet n;
    auto collect = [&](int node, double value){
        return coordination::mp_collection(n.d(node), 0, node, value, 0.0, adder, divider);
    };
    EXPECT_EQ(1.0, collect(0, 1));
    EXPECT_EQ(2.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
    n.newround();
    EXPECT_EQ(3.0, collect(0, 1));
    EXPECT_EQ(6.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
    n.newround();
    EXPECT_EQ(7.0, collect(0, 1));
    EXPECT_EQ(6.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
    n.newround();
    EXPECT_EQ(7.0, collect(0, 1));
    EXPECT_EQ(6.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
}

TEST(CollectionTest, WMP) {
    testnet n;
    auto collect = [&](int node, double value){
        return coordination::wmp_collection(n.d(node), 0, node, 2.0, value, adder, multiplier);
    };
    EXPECT_EQ(1.0, collect(0, 1));
    EXPECT_EQ(2.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
    n.newround();
    EXPECT_EQ(1.0, collect(0, 1));
    EXPECT_EQ(2.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
    n.newround();
    EXPECT_EQ(3.0, collect(0, 1));
    EXPECT_EQ(6.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
    n.newround();
    EXPECT_EQ(7.0, collect(0, 1));
    EXPECT_EQ(6.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
    n.newround();
    EXPECT_EQ(7.0, collect(0, 1));
    EXPECT_EQ(6.0, collect(1, 2));
    EXPECT_EQ(4.0, collect(2, 4));
}
