// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "gtest/gtest.h"

#include "lib/fcpp.hpp"

#include "project/sample/collection_compare.hpp"

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
    component::calculus<program<main>, exports<
        device_t, double, field<double>, std::array<double, 2>,
        tuple<double,device_t>, tuple<double,int>, tuple<double,double>
    >>,
    component::storage<
        algorithm,  int,
        spc_sum,    double,
        mpc_sum,    double,
        wmpc_sum,   double,
        ideal_sum,  double,
        spc_max,    double,
        mpc_max,    double,
        wmpc_max,   double,
        ideal_max,  double>,
    component::physical_position<>,
    component::timer,
    component::identifier<synchronised<true>>,
    component::randomizer<>
>;

using message_t = typename combo::node::message_t;

void fullround(times_t t, combo::net& network, device_t uid) {
    common::unique_lock<FCPP_PARALLEL> l;
    network.node_at(uid, l).round(t);
}

void fullround(times_t t, combo::net& network) {
    for (size_t i = 0; i < network.node_size(); ++i)
        fullround(t, network, i);
}

void sendto(times_t t, combo::net& network, device_t source, device_t dest) {
    if (source != dest) {
        common::unique_lock<FCPP_PARALLEL> l1, l2;
        message_t m;
        network.node_at(dest, l2).receive(t, source, network.node_at(source, l1).send(t, dest, m));
    } else {
        common::unique_lock<FCPP_PARALLEL> l;
        message_t m;
        auto& n = network.node_at(dest, l);
        n.receive(t, source, n.send(t, dest, m));
    }
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

void checker(combo::net& network, device_t uid) {
}

TEST(CollectionCompareTest, Synchronous) {
}
