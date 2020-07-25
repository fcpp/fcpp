// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <cmath>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/scheduler.hpp"
#include "lib/simulation/physical_position.hpp"

using namespace fcpp;
using namespace component::tags;


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

using seq_per = sequence::periodic<distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 1>, distribution::constant_n<times_t, 9>>;

using combo1 = component::combine_spec<
    exposer,
    component::scheduler<round_schedule<seq_per>>,
    component::physical_position<dimension<2>>,
    component::base<>
>;

TEST(PhysicalPositionTest, NoFriction) {
    combo1::net  network{common::make_tagged_tuple<oth>("foo")};
    combo1::node device{network, common::make_tagged_tuple<uid, x, a>(0, make_vec(1.0,2.0), make_vec(-1.0,0.0))};
    EXPECT_EQ(2.0, device.next());
    device.update();
    vec<2> v;
    v = make_vec(1.0, 2.0);
    EXPECT_EQ(v, device.position());
    EXPECT_EQ(v, device.position(2.0));
    v = make_vec(0.0, 0.0);
    EXPECT_EQ(v, device.velocity());
    EXPECT_EQ(v, device.velocity(2.0));
    v = make_vec(-1.0, 0.0);
    EXPECT_EQ(v, device.propulsion());
    v = make_vec(-1.0, 0.0);
    EXPECT_EQ(v, device.acceleration());
    EXPECT_EQ(0.0, device.friction());
    v = make_vec(-1.0, 2.0);
    EXPECT_EQ(v, device.position(4.0));
    v = make_vec(-2.0, 0.0);
    EXPECT_EQ(v, device.velocity(4.0));
    device.velocity() = make_vec(2.0, 1.0);
    v = make_vec(3.0, 4.0);
    EXPECT_EQ(v, device.position(4.0));
    v = make_vec(0.0, 1.0);
    EXPECT_EQ(v, device.velocity(4.0));
    device.propulsion() = make_vec(0.0, -1.0);
    v = make_vec(5.0, 2.0);
    EXPECT_EQ(v, device.position(4.0));
    v = make_vec(2.0, -1.0);
    EXPECT_EQ(v, device.velocity(4.0));
    double t;
    t = device.reach_time(0, 7.0, 2.0);
    EXPECT_EQ(5.0, t);
    t = device.reach_time(1, 2.5, 2.0);
    EXPECT_EQ(3.0, t);
    device.update();
    v = make_vec(3.0, 2.5);
    EXPECT_EQ(v, device.position());
    v = make_vec(2.0, 0.0);
    EXPECT_EQ(v, device.velocity());
    v = make_vec(5.0, 2.0);
    EXPECT_EQ(v, device.position(4.0));
    v = make_vec(2.0, -1.0);
    EXPECT_EQ(v, device.velocity(4.0));
}

TEST(PhysicalPositionTest, YesFriction) {
    // TODO: test this
}

TEST(PhysicalPositionTest, NbrVec) {
    combo1::net  network{common::make_tagged_tuple<oth>("foo")};
    combo1::node d1{network, common::make_tagged_tuple<uid, x>(1, make_vec(0.0,0.0))};
    combo1::node d2{network, common::make_tagged_tuple<uid, x>(2, make_vec(1.0,0.0))};
    combo1::node d3{network, common::make_tagged_tuple<uid, x>(3, make_vec(0.0,1.0))};
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
    d = norm(details::self(d1.nbr_vec(), 1) - make_vec(0.0, 0.0));
    EXPECT_GT(1e-6, d);
    d = norm(details::self(d1.nbr_vec(), 2) - make_vec(1.0, 0.0));
    EXPECT_GT(1e-6, d);
    d = norm(details::self(d1.nbr_vec(), 3) - make_vec(0.0, 1.0));
    EXPECT_GT(1e-6, d);
    vec<2> res = details::self(d1.nbr_vec(), 0);
    EXPECT_TRUE(std::isnan(res[0]));
    EXPECT_TRUE(std::isnan(res[1]));
    EXPECT_EQ(1.0/0.0, details::self(d1.nbr_dist(), 0));
    EXPECT_EQ(0.0, details::self(d1.nbr_dist(), 1));
    EXPECT_EQ(1.0, details::self(d1.nbr_dist(), 2));
    EXPECT_EQ(1.0, details::self(d1.nbr_dist(), 3));
}
