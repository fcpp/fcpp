// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/spreading.hpp"

using namespace fcpp;
using fcpp::coordination::INF;

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

struct main {
    template <typename node_t>
    void operator()(node_t&, times_t) {}
};

using combo1 = component::combine<
    lagdist,
    component::calculus<main, metric::once, times_t, int, field<int>, tuple<double,double>, tuple<double,int>>
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

field<double> nbr_one() {
    return 1;
}

namespace fcpp {
namespace coordination {

//! @brief Computes the distance from a source through adaptive bellmann-ford with old+nbr.
template <typename node_t, typename G, typename = common::if_signature<G, field<double>()>>
double slow_distance(node_t& node, trace_t call_point, bool source, G&& metric) {
    data::trace_call trace_caller(node.stack_trace, call_point);

    return old(node, 0, std::numeric_limits<double>::infinity(), [&] (double d) {
        double r = min_hood(node, 1, nbr(node, 2, d) + metric());
        return source ? 0.0 : r;
    });
}

}
}


TEST(SpreadingTest, ABFH) {
    testnet n;
    auto hop_count = [&](int node, bool source){
        return coordination::abf_hops(n.d(node), 0, source);
    };
    EXPECT_EQ(I, hop_count(0, false));
    EXPECT_EQ(0, hop_count(0, true));
    EXPECT_EQ(I, hop_count(1, false));
    EXPECT_EQ(I, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0, hop_count(0, true));
    EXPECT_EQ(1, hop_count(1, false));
    EXPECT_EQ(I, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0, hop_count(0, true));
    EXPECT_EQ(1, hop_count(1, false));
    EXPECT_EQ(2, hop_count(2, false));
}

TEST(SpreadingTest, ABFD) {
    testnet n;
    auto hop_count = [&](int node, bool source){
        return coordination::abf_distance(n.d(node), 0, source);
    };
    EXPECT_EQ(INF, hop_count(0, false));
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(INF, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(2.0, hop_count(2, false));
}

TEST(SpreadingTest, ABFM) {
    testnet n;
    auto hop_count = [&](int node, bool source){
        return coordination::abf_distance(n.d(node), 0, source, nbr_one);
    };
    EXPECT_EQ(INF, hop_count(0, false));
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(INF, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(2.0, hop_count(2, false));
}

TEST(SpreadingTest, SLOW) {
    testnet n;
    auto hop_count = [&](int node, bool source){
        return coordination::slow_distance(n.d(node), 0, source, nbr_one);
    };
    EXPECT_EQ(INF, hop_count(0, false));
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(INF, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(INF, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(2.0, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(2.0, hop_count(2, false));
}

TEST(SpreadingTest, BISD) {
    testnet n;
    auto hop_count = [&](int node, bool source){
        return coordination::bis_distance(n.d(node), 0, source, 0, 0);
    };
    EXPECT_EQ(INF, hop_count(0, false));
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(INF, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(2.0, hop_count(2, false));
}

TEST(SpreadingTest, BISM) {
    testnet n;
    auto hop_count = [&](int node, bool source){
        return coordination::bis_distance(n.d(node), 0, source, 0, 0, nbr_one);
    };
    EXPECT_EQ(INF, hop_count(0, false));
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(INF, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(2.0, hop_count(2, false));
}

TEST(SpreadingTest, FLEXD) {
    testnet n;
    auto hop_count = [&](int node, bool source){
        return coordination::flex_distance(n.d(node), 0, source, 0, 1, 0, 0);
    };
    EXPECT_EQ(INF, hop_count(0, false));
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(INF, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(2.0, hop_count(2, false));
}

TEST(SpreadingTest, FLEXM) {
    testnet n;
    auto hop_count = [&](int node, bool source){
        return coordination::flex_distance(n.d(node), 0, source, 0, 1, 0, 0, nbr_one);
    };
    EXPECT_EQ(INF, hop_count(0, false));
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(INF, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(INF, hop_count(2, false));
    n.newround();
    EXPECT_EQ(0.0, hop_count(0, true));
    EXPECT_EQ(1.0, hop_count(1, false));
    EXPECT_EQ(2.0, hop_count(2, false));
}

TEST(SpreadingTest, Broadcast) {
    testnet n;
    auto broadcast = [&](int node, int value){
        return coordination::broadcast(n.d(node), 0, node, value);
    };
    EXPECT_EQ(1, broadcast(0, 1));
    EXPECT_EQ(0, broadcast(0, 0));
    EXPECT_EQ(1, broadcast(1, 1));
    EXPECT_EQ(2, broadcast(2, 2));
    n.newround();
    EXPECT_EQ(0, broadcast(0, 0));
    EXPECT_EQ(0, broadcast(1, 1));
    EXPECT_EQ(1, broadcast(2, 2));
    n.newround();
    EXPECT_EQ(0, broadcast(0, 0));
    EXPECT_EQ(0, broadcast(1, 1));
    EXPECT_EQ(0, broadcast(2, 2));
}

TEST(SpreadingTest, BroadcastSource) {
    testnet n;
    auto broadcast = [&](int node, int value){
        return coordination::broadcast(n.d(node), 0, node, value, node==0, I);
    };
    EXPECT_EQ(1, broadcast(0, 1));
    EXPECT_EQ(0, broadcast(0, 0));
    EXPECT_EQ(I, broadcast(1, 1));
    EXPECT_EQ(I, broadcast(2, 2));
    n.newround();
    EXPECT_EQ(0, broadcast(0, 0));
    EXPECT_EQ(0, broadcast(1, 1));
    EXPECT_EQ(I, broadcast(2, 2));
    n.newround();
    EXPECT_EQ(0, broadcast(0, 0));
    EXPECT_EQ(0, broadcast(1, 1));
    EXPECT_EQ(0, broadcast(2, 2));
}
