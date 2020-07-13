// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "gtest/gtest.h"

#include "lib/common/tagged_tuple.hpp"
#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/data/field.hpp"

using namespace fcpp;

template <typename node_t>
times_t delayed(node_t& node, trace_t call_point, times_t t) {
    return old(node, call_point, t);
}

template <typename node_t>
times_t delayed(node_t& node, trace_t call_point, times_t start, times_t t) {
    return old(node, call_point, t, start);
}

template <typename node_t>
int counter(node_t& node, trace_t call_point) {
    return old(node, call_point, 0, [](const int& o) {
        return o+1;
    });
}

template <typename node_t>
int sharing(node_t& node, trace_t call_point, int x) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return fold_hood(node, 0, [](int x, int y) {
        return x+y;
    }, nbr(node, 1, x));
}

template <typename node_t>
int gossip(node_t& node, trace_t call_point, int x) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return nbr(node, 0, x, [&](fcpp::field<int> n) {
        return std::max(fold_hood(node, 1, [](int x, int y) {
            return std::max(x,y);
        }, n), x);
    });
}

struct main {
    template <typename node_t>
    void operator()(node_t&, times_t) {}
};

using combo1 = component::combine_spec<
    component::calculus<component::tags::exports<fcpp::field<int>, times_t, int>>,
    component::base<>
>;

using message_t = typename combo1::node::message_t;

void sendto(const combo1::node& source, combo1::node& dest) {
    message_t m;
    dest.receive(0.0, source.uid, source.send(0.0, dest.uid, m));
}

void rounder(combo1::node& node) {
    node.round_end(0.0);
    node.round_start(0.0);
}

TEST(CalculusTest, Size) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid, component::tags::hoodsize>(0, device_t(3))};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid>(1)};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid>(2)};
    combo1::node d3{network, common::make_tagged_tuple<component::tags::uid>(3)};
    combo1::node d4{network, common::make_tagged_tuple<component::tags::uid>(4)};
    d0.round_start(0.0);
    EXPECT_EQ(1, (int)d0.size());
    d0.round_end(0.0);
    sendto(d0, d0);
    d0.round_start(0.0);
    EXPECT_EQ(1, (int)d0.size());
    d0.round_end(0.0);
    sendto(d1, d0);
    d0.round_start(0.0);
    EXPECT_EQ(2, (int)d0.size());
    d0.round_end(0.0);
    sendto(d2, d0);
    d0.round_start(0.0);
    EXPECT_EQ(2, (int)d0.size());
    d0.round_end(0.0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    d0.round_start(0.0);
    EXPECT_EQ(3, (int)d0.size());
    d0.round_end(0.0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    sendto(d3, d0);
    d0.round_start(0.0);
    EXPECT_EQ(3, (int)d0.size());
    d0.round_end(0.0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    sendto(d3, d0);
    sendto(d4, d0);
    d0.round_start(0.0);
    EXPECT_EQ(3, (int)d0.size());
    d0.round_end(0.0);
    d0.round_start(0.0);
    EXPECT_EQ(1, (int)d0.size());
    d0.round_end(0.0);
    sendto(d4, d0);
    d0.round_start(0.0);
    EXPECT_EQ(2, (int)d0.size());
    d0.round_end(0.0);
}

TEST(CalculusTest, Old) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid>(0)};
    double d;
    d = delayed(d0, 0, 2.0);
    EXPECT_EQ(2.0, d);
    d = delayed(d0, 0, 1.0);
    EXPECT_EQ(1.0, d);
    sendto(d0, d0);
    rounder(d0);
    d = delayed(d0, 0, 3.0);
    EXPECT_EQ(1.0, d);
    rounder(d0);
    d = delayed(d0, 0, 5.0);
    EXPECT_EQ(1.0, d);
    sendto(d0, d0);
    rounder(d0);
    d = delayed(d0, 0, 3.0);
    EXPECT_EQ(5.0, d);
    d = delayed(d0, 1, 3.0, 2.0);
    EXPECT_EQ(2.0, d);
    sendto(d0, d0);
    rounder(d0);
    d = delayed(d0, 1, 6.0, 2.0);
    EXPECT_EQ(3.0, d);
    d = counter(d0, 2);
    EXPECT_EQ(1, d);
    d = counter(d0, 2);
    EXPECT_EQ(1, d);
    sendto(d0, d0);
    rounder(d0);
    d = counter(d0, 2);
    EXPECT_EQ(2, d);
    sendto(d0, d0);
    rounder(d0);
    d = counter(d0, 2);
    EXPECT_EQ(3, d);
}

TEST(CalculusTest, Nbr) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid>(0)};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid>(1)};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid>(2)};
    int d;
    d = sharing(d0, 0, 3);
    EXPECT_EQ(3, d);
    d = sharing(d0, 0, 4);
    EXPECT_EQ(4, d);
    d = sharing(d1, 0, 2);
    EXPECT_EQ(2, d);
    d = sharing(d2, 0, 1);
    EXPECT_EQ(1, d);
    d0.round_end(0.0);
    d1.round_end(0.0);
    d2.round_end(0.0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    d0.round_start(0.0);
    d = sharing(d0, 0, 3);
    EXPECT_EQ(7, d);
    d = gossip(d0, 1, 3);
    EXPECT_EQ(3, d);
    d = gossip(d1, 1, 2);
    EXPECT_EQ(2, d);
    d = gossip(d2, 1, 4);
    EXPECT_EQ(4, d);
    d0.round_end(0.0);
    d1.round_end(0.0);
    d2.round_end(0.0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    d0.round_start(0.0);
    d = gossip(d0, 1, 1);
    EXPECT_EQ(4, d);
    d = gossip(d0, 1, 5);
    EXPECT_EQ(5, d);
    d = gossip(d0, 1, 3);
    EXPECT_EQ(4, d);
}
