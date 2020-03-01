// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <cmath>

#include <array>

#include "gtest/gtest.h"

#include "lib/common/array.hpp"
#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/component/base.hpp"
#include "lib/component/scheduler.hpp"
#include "lib/data/field.hpp"
#include "lib/simulation/physical_position.hpp"

using namespace fcpp;


struct tag {};
struct gat {};
struct oth {};

// Component exposing the storage interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using P::node::nbr_vec;
            using P::node::nbr_dist;
        };
        using net  = typename P::net;
    };
};

using seq_per = random::sequence_periodic<random::constant_distribution<times_t, 2>, random::constant_distribution<times_t, 1>, random::constant_distribution<times_t, 9>>;

using combo1 = component::combine<
    exposer,
    component::scheduler<seq_per>,
    component::physical_position<2>
>;

std::array<double, 2> vec(double x, double y) {
    return {x,y};
}

TEST(PhysicalPositionTest, NoFriction) {
    combo1::net  network{common::make_tagged_tuple<oth>("foo")};
    combo1::node device{network, common::make_tagged_tuple<component::tags::uid, component::tags::x, component::tags::a>(0, vec(1.0,2.0), vec(-1.0,0.0))};
    EXPECT_EQ(2.0, device.next());
    device.update();
    std::array<double, 2> v;
    v = vec(1.0, 2.0);
    EXPECT_EQ(v, device.position());
    EXPECT_EQ(v, device.position(2.0));
    v = vec(0.0, 0.0);
    EXPECT_EQ(v, device.velocity());
    EXPECT_EQ(v, device.velocity(2.0));
    v = vec(-1.0, 0.0);
    EXPECT_EQ(v, device.propulsion());
    v = vec(-1.0, 0.0);
    EXPECT_EQ(v, device.acceleration());
    EXPECT_EQ(0.0, device.friction());
    v = vec(-1.0, 2.0);
    EXPECT_EQ(v, device.position(4.0));
    v = vec(-2.0, 0.0);
    EXPECT_EQ(v, device.velocity(4.0));
    device.velocity() = vec(2.0, 1.0);
    v = vec(3.0, 4.0);
    EXPECT_EQ(v, device.position(4.0));
    v = vec(0.0, 1.0);
    EXPECT_EQ(v, device.velocity(4.0));
    device.propulsion() = vec(0.0, -1.0);
    v = vec(5.0, 2.0);
    EXPECT_EQ(v, device.position(4.0));
    v = vec(2.0, -1.0);
    EXPECT_EQ(v, device.velocity(4.0));
    double t;
    t = device.reach_time(0, 7.0, 2.0);
    EXPECT_EQ(5.0, t);
    t = device.reach_time(1, 2.5, 2.0);
    EXPECT_EQ(3.0, t);
    device.update();
    v = vec(3.0, 2.5);
    EXPECT_EQ(v, device.position());
    v = vec(2.0, 0.0);
    EXPECT_EQ(v, device.velocity());
    v = vec(5.0, 2.0);
    EXPECT_EQ(v, device.position(4.0));
    v = vec(2.0, -1.0);
    EXPECT_EQ(v, device.velocity(4.0));
}

TEST(PhysicalPositionTest, YesFriction) {
    // TODO: test this
}

TEST(PhysicalPositionTest, NbrVec) {
    combo1::net  network{common::make_tagged_tuple<oth>("foo")};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(1, vec(0.0,0.0))};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(2, vec(1.0,0.0))};
    combo1::node d3{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(3, vec(0.0,1.0))};
    combo1::node::message_t m;
    EXPECT_EQ(2.0, d1.next());
    EXPECT_EQ(2.0, d2.next());
    EXPECT_EQ(2.0, d3.next());
    d1.update();
    d2.update();
    d3.update();
    EXPECT_EQ(3.0, d1.next());
    EXPECT_EQ(3.0, d2.next());
    EXPECT_EQ(3.0, d3.next());
    d1.receive(2.00, 1, d1.send(2.00, 1, m));
    d1.receive(2.25, 2, d2.send(2.25, 1, m));
    d1.receive(2.50, 3, d3.send(2.50, 1, m));
    double d;
    d = norm(details::self(d1.nbr_vec(), 1) - vec(0.0, 0.0));
    EXPECT_GT(1e-6, d);
    d = norm(details::self(d1.nbr_vec(), 2) - vec(1.0, 0.0));
    EXPECT_GT(1e-6, d);
    d = norm(details::self(d1.nbr_vec(), 3) - vec(0.0, 1.0));
    EXPECT_GT(1e-6, d);
    std::array<double, 2> res = details::self(d1.nbr_vec(), 0);
    EXPECT_TRUE(std::isnan(res[0]));
    EXPECT_TRUE(std::isnan(res[1]));
    EXPECT_EQ(1.0/0.0, details::self(d1.nbr_dist(), 0));
    EXPECT_EQ(0.0, details::self(d1.nbr_dist(), 1));
    EXPECT_EQ(1.0, details::self(d1.nbr_dist(), 2));
    EXPECT_EQ(1.0, details::self(d1.nbr_dist(), 3));
}
