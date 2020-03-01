// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "gtest/gtest.h"

#include "lib/common/array.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"
#include "lib/simulation/physical_position.hpp"
#include "lib/coordination/spreading.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"

#include "project/sample/slowdistance.hpp"

using namespace fcpp;
using namespace coordination::tags;
using namespace component::tags;

// Component exposing part of the interface for easier debugging.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using P::node::round;
        };
        struct net : public P::net {
            using P::net::net;
            using P::net::node_emplace;
        };
    };
};

using combo = component::combine<
    exposer,
    coordination::slowdistance,
    coordination::spreading,
    component::physical_position<>,
    component::storage<idealdist, double, fastdist, double, slowdist, double, fasterr, double, slowerr, double>,
    component::identifier<true>,
    component::calculus<metric::once, double>
>;

using message_t = typename combo::node::message_t;

void round(times_t t, combo::net& network, device_t uid) {
    common::unique_lock<FCPP_PARALLEL> l;
    network.node_at(uid, l).round(t);
}

void round(times_t t, combo::net& network) {
    for (size_t i = 0; i < network.node_size(); ++i)
        round(t, network, i);
}

void sendto(times_t t, combo::net& network, device_t source, device_t dest) {
    common::unique_lock<FCPP_PARALLEL> l1, l2;
    message_t m;
    network.node_at(dest, l2).receive(t, source, network.node_at(source, l1).send(t, dest, m));
}

void sendto(times_t t, combo::net& network) {
    for (size_t i = 0; i < network.node_size(); ++i) {
        if (i > 0)
            sendto(t, network, i, i-1);
        sendto(    t, network, i, i);
        if (i < network.node_size()-1)
            sendto(t, network, i, i+1);
    }
}

void checker(combo::net& network, device_t uid, double ideal, double fastd, double slowd) {
    EXPECT_EQ(ideal, common::get<idealdist>(network.node_at(uid).storage_tuple()));
    EXPECT_EQ(fastd, common::get< fastdist>(network.node_at(uid).storage_tuple()));
    EXPECT_EQ(slowd, common::get< slowdist>(network.node_at(uid).storage_tuple()));
}

TEST(SlowdistanceTest, Synchronous) {
    double inf = 1.0/0.0;
    combo::net network{common::make_tagged_tuple<>()};
    network.node_emplace(common::make_tagged_tuple<x>(std::make_array(0.0, 0.0)));
    network.node_emplace(common::make_tagged_tuple<x>(std::make_array(1.0, 0.0)));
    network.node_emplace(common::make_tagged_tuple<x>(std::make_array(1.5, 0.0)));
    round( 0.0, network);
    checker(network, 0, 0.0, 0.0, 0.0);
    checker(network, 1, 1.0, inf, inf);
    checker(network, 2, 1.5, inf, inf);
    sendto(0.5, network);
    round( 1.0, network);
    checker(network, 0, 0.0, 0.0, 0.0);
    checker(network, 1, 1.0, 1.0, inf);
    checker(network, 2, 1.5, inf, inf);
    sendto(1.5, network);
    round( 2.0, network);
    checker(network, 0, 0.0, 0.0, 0.0);
    checker(network, 1, 1.0, 1.0, 1.0);
    checker(network, 2, 1.5, 1.5, inf);
    sendto(2.5, network);
    round( 3.0, network);
    checker(network, 0, 0.0, 0.0, 0.0);
    checker(network, 1, 1.0, 1.0, 1.0);
    checker(network, 2, 1.5, 1.5, inf);
    sendto(3.5, network);
    round( 4.0, network);
    checker(network, 0, 0.0, 0.0, 0.0);
    checker(network, 1, 1.0, 1.0, 1.0);
    checker(network, 2, 1.5, 1.5, 1.5);
}
