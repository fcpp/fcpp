// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <array>
#include <vector>

#include "gtest/gtest.h"

#include "lib/common/array.hpp"
#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/component/base.hpp"
#include "lib/component/scheduler.hpp"
#include "lib/data/field.hpp"
#include "lib/simulation/physical_connector.hpp"
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
            using P::node::nbr_dist;
        };
        using net = typename P::net;
    };
};

using seq_per = random::sequence_periodic<random::constant_distribution<times_t, 2>, random::constant_distribution<times_t, 1>, random::constant_distribution<times_t, 9>>;

using combo1 = component::combine<
    exposer,
    component::scheduler<component::tags::round_schedule<seq_per>>,
    component::physical_connector<connector::fixed<1>, random::constant_distribution<times_t, 1, 4>>,
    component::physical_position<>
>;

std::array<double, 2> vec(double x, double y) {
    return {x,y};
}

TEST(PhysicalConnectorTest, Cell) {
    int n[4]; // 4 nodes
    component::details::cell<int> c[4]; // 4 cells
    n[0] = n[1] = n[2] = n[3] = 0;
    c[0].insert(n[0]);
    c[1].insert(n[1]);
    c[2].insert(n[2]);
    c[3].insert(n[3]);
    c[2].erase(n[2]);
    c[1].insert(n[2]);
    EXPECT_EQ(1ULL, c[0].content().size());
    EXPECT_EQ(2ULL, c[1].content().size());
    EXPECT_EQ(0ULL, c[2].content().size());
    EXPECT_EQ(1ULL, c[3].content().size());
    for (auto nn : c[1].content()) *nn = 1;
    EXPECT_EQ(0, n[0]);
    EXPECT_EQ(1, n[1]);
    EXPECT_EQ(1, n[2]);
    EXPECT_EQ(0, n[3]);
    c[0].link(c[0]);
    c[0].link(c[1]);
    c[1].link(c[0]);
    c[1].link(c[1]);
    c[0].link(c[2]);
    c[3].link(c[3]);
    EXPECT_EQ(3ULL, c[0].linked().size());
    EXPECT_EQ(2ULL, c[1].linked().size());
    EXPECT_EQ(0ULL, c[2].linked().size());
    EXPECT_EQ(1ULL, c[3].linked().size());
    for (auto nc : c[0].linked()) for (auto nn : nc->content()) *nn = 2;
    EXPECT_EQ(2, n[0]);
    EXPECT_EQ(2, n[1]);
    EXPECT_EQ(2, n[2]);
    EXPECT_EQ(0, n[3]);
    for (auto nc : c[3].linked()) for (auto nn : nc->content()) *nn = 3;
    EXPECT_EQ(2, n[0]);
    EXPECT_EQ(2, n[1]);
    EXPECT_EQ(2, n[2]);
    EXPECT_EQ(3, n[3]);
    for (auto nc : c[1].linked()) for (auto nn : nc->content()) *nn = 4;
    EXPECT_EQ(4, n[0]);
    EXPECT_EQ(4, n[1]);
    EXPECT_EQ(4, n[2]);
    EXPECT_EQ(3, n[3]);
}

TEST(PhysicalConnectorTest, Connection) {
    combo1::net network{common::make_tagged_tuple<oth>("foo")};
    EXPECT_EQ(1.0, network.connection_radius());
    typename connector::fixed<1>::type data;
    bool connect;
    connect = network.connection_success(data, vec(0.5,1), data, vec(0.4,0.9));
    EXPECT_TRUE(connect);
    connect = network.connection_success(data, vec(0.5,1), data, vec(7,10));
    EXPECT_FALSE(connect);
    connect = network.connection_success(data, vec(0.5,1), data, vec(0.5,0));
    EXPECT_TRUE(connect);
    connect = network.connection_success(data, vec(0.5,1), data, vec(0.51,0));
    EXPECT_FALSE(connect);
}

TEST(PhysicalConnectorTest, EnterLeave) {
    combo1::net  network{common::make_tagged_tuple<oth>("foo")};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(0, vec(0.5,0.5))};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(1, vec(0.0,0.0))};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(2, vec(1.5,0.5))};
    combo1::node d3{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(3, vec(1.5,1.5))};
    combo1::node d4{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(4, vec(9.0,9.0))};
    network.cell_enter(d0);
    network.cell_enter(d1);
    network.cell_enter(d2);
    network.cell_leave(d0);
    network.cell_enter(d3);
    network.cell_enter(d4);
    network.cell_enter(d0);
    std::vector<device_t> close, target;
    for (auto c : network.cell_of(d0).linked()) for (auto n : c->content()) close.push_back(n->uid);
    std::sort(close.begin(), close.end());
    target = {0,1,2,3};
    EXPECT_EQ(target, close);
}

template <typename node_t>
void update(node_t& node) {
    common::lock_guard<FCPP_PARALLEL> l(node.mutex);
    node.update();
}

TEST(PhysicalConnectorTest, Messages) {
    combo1::net  network{common::make_tagged_tuple<oth>("foo")};
    combo1::node d0{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(0, vec(0.25,0.25))};
    combo1::node d1{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(1, vec(0.0,0.0))};
    combo1::node d2{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(2, vec(1.5,0.5))};
    combo1::node d3{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(3, vec(1.5,1.5))};
    combo1::node d4{network, common::make_tagged_tuple<component::tags::uid, component::tags::x>(4, vec(9.0,9.0))};
    EXPECT_EQ(2.0, d0.next());
    EXPECT_EQ(2.0, d1.next());
    EXPECT_EQ(2.0, d2.next());
    EXPECT_EQ(2.0, d3.next());
    EXPECT_EQ(2.0, d4.next());
    d0.velocity() = vec(1,1);
    update(d0);
    update(d1);
    update(d2);
    update(d3);
    update(d4);
    EXPECT_EQ(2.25, d0.next());
    EXPECT_EQ(2.25, d1.next());
    EXPECT_EQ(2.25, d2.next());
    EXPECT_EQ(2.25, d3.next());
    EXPECT_EQ(2.25, d4.next());
    update(d0);
    update(d1);
    update(d2);
    update(d3);
    update(d4);
    double d;
    d = fcpp::details::self(d0.nbr_dist(), 0);
    EXPECT_NEAR(0, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 1);
    EXPECT_NEAR(0.7071067811865476, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 2);
    EXPECT_NEAR(1, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 3);
    EXPECT_EQ(1.0/0.0, d);
    d = fcpp::details::self(d0.nbr_dist(), 4);
    EXPECT_EQ(1.0/0.0, d);
    d = fcpp::details::self(d0.nbr_dist(), 5);
    EXPECT_EQ(1.0/0.0, d);
    EXPECT_NEAR(2.75, d0.next(), FCPP_TIME_EPSILON);
    EXPECT_EQ(3.0, d1.next());
    EXPECT_EQ(3.0, d2.next());
    EXPECT_EQ(3.0, d3.next());
    EXPECT_EQ(3.0, d4.next());
    update(d0);
    EXPECT_EQ(3.0, d0.next());
    update(d0);
    update(d1);
    update(d2);
    update(d3);
    update(d4);
    EXPECT_EQ(3.25, d0.next());
    EXPECT_EQ(3.25, d1.next());
    EXPECT_EQ(3.25, d2.next());
    EXPECT_EQ(3.25, d3.next());
    EXPECT_EQ(3.25, d4.next());
    update(d0);
    update(d1);
    update(d2);
    update(d3);
    update(d4);
    d = fcpp::details::self(d0.nbr_dist(), 0);
    EXPECT_NEAR(0, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 1);
    EXPECT_NEAR(0.7071067811865476, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 2);
    EXPECT_NEAR(1, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 3);
    EXPECT_EQ(0.0, d);
    d = fcpp::details::self(d0.nbr_dist(), 4);
    EXPECT_EQ(1.0/0.0, d);
    d = fcpp::details::self(d0.nbr_dist(), 5);
    EXPECT_EQ(1.0/0.0, d);
}
