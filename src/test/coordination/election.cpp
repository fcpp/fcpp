// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/election.hpp"

using namespace fcpp;

constexpr int I = std::numeric_limits<int>::max()-1;

using combo1 = component::combine<
    component::calculus<component::tags::exports<tuple<int,int>, tuple<int,int,int,int>>>
>;

struct testnet {
    testnet() : network{common::make_tagged_tuple<>()},
        d0{network, common::make_tagged_tuple<component::tags::uid>(0)},
        d1{network, common::make_tagged_tuple<component::tags::uid>(1)},
        d2{network, common::make_tagged_tuple<component::tags::uid>(2)} {
        topology.push_back({0, 1});
        topology.push_back({0, 1, 2});
        topology.push_back({1, 2});
        for (int i = 0; i < 3; ++i) d(i).round_start(0.0);
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

#define EXPECT_ROUND(a, b, c, x, y, z)                                                  \
        EXPECT_EQ(make_tuple(round(0,a), round(1,b), round(2,c)), make_tuple(x,y,z));   \
        n.newround()


TEST(ElectionTest, Diameter) {
    testnet n;
    auto round = [&](int node, int value){
        return coordination::diameter_election(n.d(node), 0, value, 3);
    };
    EXPECT_ROUND(0, 1, 2,
                 0, 1, 2);
    EXPECT_ROUND(0, 1, 2,
                 0, 0, 1);
    EXPECT_ROUND(0, 1, 2,
                 0, 0, 0);
    EXPECT_ROUND(9, 1, 2,
                 0, 0, 0);
    EXPECT_ROUND(9, 1, 2,
                 0, 0, 0);
    EXPECT_ROUND(9, 1, 2,
                 9, 0, 2);
    EXPECT_ROUND(9, 1, 2,
                 9, 1, 2);
    EXPECT_ROUND(9, 1, 2,
                 1, 1, 1);
}

TEST(ElectionTest, Wave) {
    testnet n;
    auto round = [&](int node, int value){
        return coordination::wave_election(n.d(node), 0, value, [](int x){ return x+1; });
    };
    EXPECT_ROUND(0, 1, 2,
                 0, 1, 2);
    EXPECT_ROUND(0, 1, 2,
                 0, 0, 1);
    EXPECT_ROUND(0, 1, 2,
                 0, 0, 2);
    EXPECT_ROUND(0, 1, 2,
                 0, 0, 2);
    EXPECT_ROUND(0, 1, 2,
                 0, 0, 0);
    EXPECT_ROUND(9, 1, 2,
                 0, 0, 0);
    EXPECT_ROUND(9, 1, 2,
                 0, 1, 0);
    EXPECT_ROUND(9, 1, 2,
                 1, 1, 1);
    EXPECT_ROUND(9, 1, 2,
                 1, 1, 1);
    EXPECT_ROUND(9, 1, 2,
                 1, 1, 1);
    EXPECT_ROUND(9, 8, 2,
                 1, 1, 1);
    EXPECT_ROUND(9, 8, 2,
                 9, 1, 2);
    EXPECT_ROUND(9, 8, 2,
                 9, 2, 2);
    EXPECT_ROUND(9, 8, 2,
                 2, 2, 2);
}
