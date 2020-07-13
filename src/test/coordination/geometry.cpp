// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/component/randomizer.hpp"
#include "lib/coordination/geometry.hpp"
#include "lib/simulation/physical_position.hpp"

using namespace fcpp;

constexpr int I = std::numeric_limits<int>::max()-1;

DECLARE_OPTIONS(options, component::tags::exports<vec<2>>);
DECLARE_COMBINE(combo, component::calculus, component::physical_position, component::randomizer);

using combo1 = combo<options>;

struct testnet {
    testnet() : network{common::make_tagged_tuple<>()},
        d0{network, common::make_tagged_tuple<component::tags::uid,component::tags::x>(0, make_vec(0,0))},
        d1{network, common::make_tagged_tuple<component::tags::uid,component::tags::x>(1, make_vec(1,0))},
        d2{network, common::make_tagged_tuple<component::tags::uid,component::tags::x>(2, make_vec(1,1))} {
        topology.push_back({0, 1});
        topology.push_back({0, 1, 2});
        topology.push_back({1, 2});
        for (int i = 0; i < 3; ++i) d(i).round_start(0.0);
        count = 0;
    }

    combo1::node& d(int id) {
        if (id == 0) return d0;
        if (id == 1) return d1;
        if (id == 2) return d2;
        return d0;
    }

    void newround() {
        for (int i = 0; i < 3; ++i) d(i).round_end(count);
        for (int source = 0; source < 3; ++source)
            for (int dest : topology[source]) {
                typename combo1::node::message_t m;
                d(dest).receive(count + 0.5, source, d(source).send(count + 0.5, dest, m));
            }
        ++count;
        for (int i = 0; i < 3; ++i) d(i).round_start(count);
    }
    
    combo1::net  network;
    combo1::node d0, d1, d2;
    std::vector<std::vector<int>> topology;
    int count;
};

#define EXPECT_ROUND(a, x)          \
        EXPECT_EQ(round(0,a), x);   \
        n.newround()


TEST(GeometryTest, Target) {
    testnet n;
    vec<2> p{0,0};
    for (int i=0; i<10000; ++i) {
        vec<2> q = coordination::random_rectangle_target(n.d(0), 0, make_vec(1,2), make_vec(3,8));
        EXPECT_TRUE(1 <= q[0] and q[0] <= 3);
        EXPECT_TRUE(2 <= q[1] and q[1] <= 8);
        p += q;
    }
    p /= 10000;
    EXPECT_LT(distance(p, make_vec(2,5)), 0.1);
    p = {0,0};
    for (int i=0; i<10000; ++i) {
        vec<2> q = coordination::random_rectangle_target(n.d(2), 0, make_vec(1,2), make_vec(3,8), 2);
        EXPECT_TRUE(1 <= q[0] and q[0] <= 3);
        EXPECT_TRUE(2 <= q[1] and q[1] <= 3);
        p += q;
    }
    p /= 10000;
    EXPECT_LT(distance(p, make_vec(2,2.5)), 0.1);
}

TEST(GeometryTest, Follow) {
    testnet n;
    auto round = [&](int node, vec<2> value){
        return coordination::follow_target(n.d(node), 0, value, 3, 1);
    };
    EXPECT_ROUND(make_vec(10, 0), 10);
    EXPECT_ROUND(make_vec(10, 0), 7);
    EXPECT_ROUND(make_vec(10, 0), 4);
    EXPECT_ROUND(make_vec(10, 0), 1);
    EXPECT_ROUND(make_vec(10, 0), 0);
    EXPECT_ROUND(make_vec(10, 0), 0);
}

TEST(GeometryTest, Walk) {
    testnet n;
    vec<2> p;
    for (int i=0; i<10000; ++i) {
        p = coordination::rectangle_walk(n.d(0), 0, make_vec(0,0), make_vec(3,3), 1, 0.5, 1);
        EXPECT_TRUE(0 <= p[0] and p[0] <= 3 and 0 <= p[1] and p[1] <= 3);
        p = n.d(0).position();
        EXPECT_TRUE(0 <= p[0] and p[0] <= 3 and 0 <= p[1] and p[1] <= 3);
        EXPECT_LE(norm(n.d(0).velocity()), 0.5000001);
        EXPECT_EQ(n.d(0).propulsion(), make_vec(0,0));
        EXPECT_EQ(n.d(0).friction(), 0.0);
        n.newround();
    }
}
