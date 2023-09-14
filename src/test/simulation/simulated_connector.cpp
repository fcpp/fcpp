// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <vector>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/scheduler.hpp"
#include "lib/simulation/simulated_positioner.hpp"
#include "lib/simulation/simulated_connector.hpp"

#include "test/helper.hpp"

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
            using P::node::nbr_dist;
        };
        using net = typename P::net;
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

template <int O>
using combo = component::combine_spec<
    exposer,
    component::simulated_connector<message_size<(O & 2) == 2>, parallel<(O & 1) == 1>, connector<connect::fixed<1>>, delay<distribution::constant_n<times_t, 1, 4>>>,
    component::simulated_positioner<>,
    mytimer,
    component::scheduler<round_schedule<seq_per>>,
    component::base<parallel<(O & 1) == 1>>
>;


MULTI_TEST(SimulatedConnectorTest, Cell, O, 2) {
    int n[4]; // 4 nodes
    component::details::cell<(O & 1) == 1, int> c[4]; // 4 cells
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

MULTI_TEST(SimulatedConnectorTest, Connection, O, 2) {
    typename combo<O>::net network{common::make_tagged_tuple<oth>("foo")};
    EXPECT_EQ(1, network.connection_radius());
    typename connect::fixed<1>::data_type data;
    bool connect;
    connect = network.connection_success(nullptr, data, make_vec(0.5,1), data, make_vec(0.4,0.9));
    EXPECT_TRUE(connect);
    connect = network.connection_success(nullptr, data, make_vec(0.5,1), data, make_vec(7,10));
    EXPECT_FALSE(connect);
    connect = network.connection_success(nullptr, data, make_vec(0.5,1), data, make_vec(0.5,0));
    EXPECT_TRUE(connect);
    connect = network.connection_success(nullptr, data, make_vec(0.5,1), data, make_vec(0.51,0));
    EXPECT_FALSE(connect);
}

MULTI_TEST(SimulatedConnectorTest, EnterLeave, O, 2) {
    typename combo<O>::net  network{common::make_tagged_tuple<oth>("foo")};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid, x>(0, make_vec(0.5,0.5))};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid, x>(1, make_vec(0.0,0.0))};
    typename combo<O>::node d2{network, common::make_tagged_tuple<uid, x>(2, make_vec(1.5,0.5))};
    typename combo<O>::node d3{network, common::make_tagged_tuple<uid, x>(3, make_vec(1.5,1.5))};
    typename combo<O>::node d4{network, common::make_tagged_tuple<uid, x>(4, make_vec(9.0,9.0))};
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

MULTI_TEST(SimulatedConnectorTest, Messages, O, 2) {
    auto update = [](auto& node) {
        common::lock_guard<(O & 1) == 1> l(node.mutex);
        node.update();
    };
    typename combo<O>::net  network{common::make_tagged_tuple<oth>("foo")};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid, x>(0, make_vec(0.25,0.25))};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid, x>(1, make_vec(0.0,0.0))};
    typename combo<O>::node d2{network, common::make_tagged_tuple<uid, x>(2, make_vec(1.5,0.5))};
    typename combo<O>::node d3{network, common::make_tagged_tuple<uid, x>(3, make_vec(1.5,1.5))};
    typename combo<O>::node d4{network, common::make_tagged_tuple<uid, x>(4, make_vec(9.0,9.0))};
    EXPECT_EQ(2, d0.next());
    EXPECT_EQ(2, d1.next());
    EXPECT_EQ(2, d2.next());
    EXPECT_EQ(2, d3.next());
    EXPECT_EQ(2, d4.next());
    d0.velocity() = make_vec(1,1);
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
    real_t d;
    d = fcpp::details::self(d0.nbr_dist(), 0);
    EXPECT_NEAR(0, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 1);
    EXPECT_NEAR(0.7071067811865476, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 2);
    EXPECT_NEAR(1, d, 1e-9);
    d = fcpp::details::self(d0.nbr_dist(), 3);
    EXPECT_EQ(INF, d);
    d = fcpp::details::self(d0.nbr_dist(), 4);
    EXPECT_EQ(INF, d);
    d = fcpp::details::self(d0.nbr_dist(), 5);
    EXPECT_EQ(INF, d);
    EXPECT_NEAR(2.75, d0.next(), FCPP_TIME_EPSILON);
    EXPECT_EQ(3, d1.next());
    EXPECT_EQ(3, d2.next());
    EXPECT_EQ(3, d3.next());
    EXPECT_EQ(3, d4.next());
    update(d0);
    EXPECT_EQ(3, d0.next());
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
    EXPECT_EQ(0, d);
    d = fcpp::details::self(d0.nbr_dist(), 4);
    EXPECT_EQ(INF, d);
    d = fcpp::details::self(d0.nbr_dist(), 5);
    EXPECT_EQ(INF, d);
}
