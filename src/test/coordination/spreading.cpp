// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "gtest/gtest.h"

#include "lib/common/tagged_tuple.hpp"
#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/spreading.hpp"
#include "lib/data/field.hpp"

using namespace fcpp;


template <typename node_t>
double hop_count(node_t& node, bool source) {
    return coordination::distance(node, 0, source, [](){
        return fcpp::field<double>(1.0);
    });
}

struct main {
    template <typename node_t>
    void operator()(node_t&, times_t) {}
};

using combo1 = component::combine<
    component::calculus<main, metric::once, fcpp::field<int>, times_t, int>
>;

using message_t = typename combo1::node::message_t;

void sendto(const combo1::node& source, combo1::node& dest) {
    message_t m;
    dest.receive(0.0, source.uid, source.send(0.0, dest.uid, m));
}

TEST(SpreadingTest, Distance) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid>(0)};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid>(1)};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid>(2)};
    auto newround = [&]() {
        d0.round_end(0.0);
        d1.round_end(0.0);
        d2.round_end(0.0);
        sendto(d0, d0);
        sendto(d0, d1);
        sendto(d1, d0);
        sendto(d1, d1);
        sendto(d1, d2);
        sendto(d2, d1);
        sendto(d2, d2);
        d0.round_start(0.0);
        d1.round_start(0.0);
        d2.round_start(0.0);
    };
    double d;
    d = hop_count(d0, false);
    EXPECT_EQ(1.0/0.0, d);
    d = hop_count(d0, true);
    EXPECT_EQ(0.0, d);
    d = hop_count(d1, false);
    EXPECT_EQ(1.0/0.0, d);
    d = hop_count(d2, false);
    EXPECT_EQ(1.0/0.0, d);
    newround();
    d = hop_count(d0, true);
    EXPECT_EQ(0.0, d);
    d = hop_count(d1, false);
    EXPECT_EQ(1.0, d);
    d = hop_count(d2, false);
    EXPECT_EQ(1.0/0.0, d);
    newround();
    d = hop_count(d0, true);
    EXPECT_EQ(0.0, d);
    d = hop_count(d1, false);
    EXPECT_EQ(1.0, d);
    d = hop_count(d2, false);
    EXPECT_EQ(2.0, d);
}
