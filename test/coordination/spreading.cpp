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


// Component exposing the calculus interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            
            double hop_count(bool source) {
                return P::node::template distance<___>(source, [](){
                    return fcpp::field<double>(1.0);
                });
            }
        };
        using net = typename P::net;
    };
};

using combo1 = component::combine<
    exposer,
    coordination::spreading,
    component::calculus<metric::once, fcpp::field<int>, times_t, int>
>;

using message_t = typename combo1::node::message_t;

void sendto(times_t t, const combo1::node& source, combo1::node& dest) {
    message_t m;
    dest.receive(t, source.uid, source.send(t, dest.uid, m));
}

TEST(SpreadingTest, Distance) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid>(0)};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid>(1)};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid>(2)};
    double d;
    d = d0.hop_count(false);
    EXPECT_EQ(1.0/0.0, d);
    d = d0.hop_count(true);
    EXPECT_EQ(0.0, d);
    d = d1.hop_count(false);
    EXPECT_EQ(1.0/0.0, d);
    d = d2.hop_count(false);
    EXPECT_EQ(1.0/0.0, d);
    sendto(1.0, d0, d0);
    sendto(1.0, d0, d1);
    sendto(1.0, d1, d0);
    sendto(1.0, d1, d1);
    sendto(1.0, d1, d2);
    sendto(1.0, d2, d1);
    sendto(1.0, d2, d2);
    d = d0.hop_count(true);
    EXPECT_EQ(0.0, d);
    d = d1.hop_count(false);
    EXPECT_EQ(1.0, d);
    d = d2.hop_count(false);
    EXPECT_EQ(1.0/0.0, d);
    sendto(1.0, d0, d0);
    sendto(1.0, d0, d1);
    sendto(1.0, d1, d0);
    sendto(1.0, d1, d1);
    sendto(1.0, d1, d2);
    sendto(1.0, d2, d1);
    sendto(1.0, d2, d2);
    d = d0.hop_count(true);
    EXPECT_EQ(0.0, d);
    d = d1.hop_count(false);
    EXPECT_EQ(1.0, d);
    d = d2.hop_count(false);
    EXPECT_EQ(2.0, d);
}
