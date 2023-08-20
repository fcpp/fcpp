// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include <cmath>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/scheduler.hpp"
#include "lib/simulation/simulated_positioner.hpp"

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

struct mytimer {
    template <typename F, typename P>
    struct component : public P {
        DECLARE_COMPONENT(timer);
        struct node : public P::node {
            using P::node::node;
            field<times_t> const& nbr_lag() const {
                return m_nl;
            }
            field<times_t> m_nl = field<times_t>(1);
        };
        using net  = typename P::net;
    };
};

using seq_per = sequence::periodic<distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 1>, distribution::constant_n<times_t, 9>>;

using combo1 = component::combine_spec<
    component::simulated_positioner<dimension<2>>,
    mytimer,
    component::scheduler<round_schedule<seq_per>>,
    component::base<>
>;

TEST(SimulatedPositionerTest, NoFriction) {
    combo1::net  network{common::make_tagged_tuple<oth>("foo")};
    combo1::node device{network, common::make_tagged_tuple<uid, x, a>(0, make_vec(1,2), make_vec(-1,0))};
    EXPECT_EQ(2, device.next());
    device.update();
    vec<2> v;
    v = make_vec(1, 2);
    EXPECT_EQ(v, device.position());
    EXPECT_EQ(v, device.position(2));
    v = make_vec(0, 0);
    EXPECT_EQ(v, device.velocity());
    EXPECT_EQ(v, device.velocity(2));
    v = make_vec(-1, 0);
    EXPECT_EQ(v, device.propulsion());
    v = make_vec(-1, 0);
    EXPECT_EQ(v, device.acceleration());
    EXPECT_EQ(0, device.friction());
    v = make_vec(-1, 2);
    EXPECT_EQ(v, device.position(4));
    v = make_vec(-2, 0);
    EXPECT_EQ(v, device.velocity(4));
    device.velocity() = make_vec(2, 1);
    v = make_vec(3, 4);
    EXPECT_EQ(v, device.position(4));
    v = make_vec(0, 1);
    EXPECT_EQ(v, device.velocity(4));
    device.propulsion() = make_vec(0, -1);
    v = make_vec(5, 2);
    EXPECT_EQ(v, device.position(4));
    v = make_vec(2, -1);
    EXPECT_EQ(v, device.velocity(4));
    times_t t;
    t = device.reach_time(0, 7, 2);
    EXPECT_EQ(5, t);
    t = device.reach_time(1, 2.5, 2);
    EXPECT_EQ(3, t);
    device.update();
    v = make_vec(3, 2.5);
    EXPECT_EQ(v, device.position());
    v = make_vec(2, 0);
    EXPECT_EQ(v, device.velocity());
    v = make_vec(5, 2);
    EXPECT_EQ(v, device.position(4));
    v = make_vec(2, -1);
    EXPECT_EQ(v, device.velocity(4));
}

TEST(SimulatedPositionerTest, YesFriction) {
    // TODO: test this
}

TEST(SimulatedPositionerTest, NbrVec) {
    combo1::net  network{common::make_tagged_tuple<oth>("foo")};
    combo1::node d1{network, common::make_tagged_tuple<uid, x>(1, make_vec(0,0))};
    combo1::node d2{network, common::make_tagged_tuple<uid, x>(2, make_vec(1,0))};
    combo1::node d3{network, common::make_tagged_tuple<uid, x>(3, make_vec(0,1))};
    combo1::node::message_t m;
    EXPECT_EQ(2, d1.next());
    EXPECT_EQ(2, d2.next());
    EXPECT_EQ(2, d3.next());
    d1.update();
    d2.update();
    d3.update();
    EXPECT_EQ(3, d1.next());
    EXPECT_EQ(3, d2.next());
    EXPECT_EQ(3, d3.next());
    d1.receive(2.00, 1, d1.send(2.00, m));
    d1.receive(2.25, 2, d2.send(2.25, m));
    d1.receive(2.50, 3, d3.send(2.50, m));
    real_t d;
    d = norm(details::self(d1.nbr_vec(), 1) - make_vec(0, 0));
    EXPECT_GT(1e-6, d);
    d = norm(details::self(d1.nbr_vec(), 2) - make_vec(1, 0));
    EXPECT_GT(1e-6, d);
    d = norm(details::self(d1.nbr_vec(), 3) - make_vec(0, 1));
    EXPECT_GT(1e-6, d);
    vec<2> res = details::self(d1.nbr_vec(), 0);
    EXPECT_TRUE(std::isnan(res[0]));
    EXPECT_TRUE(std::isnan(res[1]));
    EXPECT_EQ(INF, details::self(d1.nbr_dist(), 0));
    EXPECT_EQ(0, details::self(d1.nbr_dist(), 1));
    EXPECT_EQ(1, details::self(d1.nbr_dist(), 2));
    EXPECT_EQ(1, details::self(d1.nbr_dist(), 3));
    EXPECT_EQ(d1.nbr_dist_lag(), field<times_t>(1));
}
