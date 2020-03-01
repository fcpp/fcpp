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


// Component exposing the calculus interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using P::node::trace_call;
            using P::node::trace_cycle;
            using P::node::fold_hood;
            using P::node::old;
            using P::node::nbr;
            using P::node::oldnbr;
            using P::node::round;
            
            times_t delayed(times_t t) {
                return P::node::template old<___>(t);
            }
            
            times_t delayed(times_t start, times_t t) {
                return P::node::template old<___>(t, start);
            }

            int counter() {
                return P::node::template old<___, int>(0, [](const int& o){
                    return o+1;
                });
            }
            
            int sharing(int x) {
                return P::node::template fold_hood<___>([](int x, int y) {
                    return x+y;
                }, P::node::template nbr<___>(x));
            }
            
            int gossip(int x) {
                return P::node::template nbr<___, int>(x, [x,this](fcpp::field<int> n){
                    return std::max(P::node::template fold_hood<___>([](int x, int y) {
                        return std::max(x,y);
                    }, n), x);
                });
            }
        };
        using net = typename P::net;
    };
};

using combo1 = component::combine<
    exposer,
    component::calculus<metric::once, fcpp::field<int>, times_t, int>
>;

using message_t = typename combo1::node::message_t;

void sendto(times_t t, const combo1::node& source, combo1::node& dest) {
    message_t m;
    dest.receive(t, source.uid, source.send(t, dest.uid, m));
}

TEST(CalculusTest, Size) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid, component::tags::hoodsize>(0, device_t(3))};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid>(1)};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid>(2)};
    combo1::node d3{network, common::make_tagged_tuple<component::tags::uid>(3)};
    combo1::node d4{network, common::make_tagged_tuple<component::tags::uid>(4)};
    EXPECT_EQ(1, (int)d0.size());
    sendto(1.0, d0, d0);
    EXPECT_EQ(1, (int)d0.size());
    sendto(1.0, d1, d0);
    EXPECT_EQ(2, (int)d0.size());
    sendto(1.0, d2, d0);
    EXPECT_EQ(3, (int)d0.size());
    sendto(1.0, d3, d0);
    EXPECT_EQ(3, (int)d0.size());
    sendto(1.0, d4, d0);
    EXPECT_EQ(3, (int)d0.size());
    d0.round(2.0);
    EXPECT_EQ(1, (int)d0.size());
    sendto(3.0, d4, d0);
    EXPECT_EQ(2, (int)d0.size());
}


TEST(CalculusTest, Old) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid>(0)};
    double d;
    d = d0.delayed(2.0);
    EXPECT_EQ(2.0, d);
    d = d0.delayed(1.0);
    EXPECT_EQ(1.0, d);
    sendto(1.0, d0, d0);
    d = d0.delayed(3.0);
    EXPECT_EQ(1.0, d);
    d0.round(3.0);
    d = d0.delayed(5.0);
    EXPECT_EQ(1.0, d);
    sendto(1.0, d0, d0);
    d = d0.delayed(3.0);
    EXPECT_EQ(5.0, d);
    d = d0.delayed(3.0, 2.0);
    EXPECT_EQ(2.0, d);
    sendto(1.0, d0, d0);
    d = d0.delayed(6.0, 2.0);
    EXPECT_EQ(3.0, d);
    d = d0.counter();
    EXPECT_EQ(1, d);
    d = d0.counter();
    EXPECT_EQ(1, d);
    sendto(1.0, d0, d0);
    d = d0.counter();
    EXPECT_EQ(2, d);
    sendto(1.0, d0, d0);
    d = d0.counter();
    EXPECT_EQ(3, d);
}


TEST(CalculusTest, Nbr) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid>(0)};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid>(1)};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid>(2)};
    int d;
    d = d0.sharing(3);
    EXPECT_EQ(3, d);
    d = d0.sharing(4);
    EXPECT_EQ(4, d);
    d = d1.sharing(2);
    EXPECT_EQ(2, d);
    d = d2.sharing(1);
    EXPECT_EQ(1, d);
    sendto(1.0, d0, d0);
    sendto(1.0, d1, d0);
    sendto(1.0, d2, d0);
    d = d0.sharing(3);
    EXPECT_EQ(7, d);
    d = d0.gossip(3);
    EXPECT_EQ(3, d);
    d = d1.gossip(2);
    EXPECT_EQ(2, d);
    d = d2.gossip(4);
    EXPECT_EQ(4, d);
    sendto(1.0, d0, d0);
    sendto(1.0, d1, d0);
    sendto(1.0, d2, d0);
    d = d0.gossip(1);
    EXPECT_EQ(4, d);
    d = d0.gossip(5);
    EXPECT_EQ(5, d);
}
