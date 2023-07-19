// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/storage.hpp"

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
            using P::node::storage;
            using P::node::storage_tuple;
        };
        using net  = typename P::net;
    };
};

using combo1 = component::combine_spec<
    exposer,
    component::storage<node_store<tag,bool,gat,int>, net_store<oth,double>>,
    component::base<>
>;


TEST(StorageTest, Node) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node device{network, common::make_tagged_tuple<uid,oth,gat>(7,'b',3)};
    EXPECT_EQ(false, device.storage<tag>());
    EXPECT_EQ(3,     device.storage(gat{}));
    EXPECT_EQ(false, common::get<tag>(device.storage_tuple()));
    EXPECT_EQ(3,     common::get<gat>(device.storage_tuple()));
    common::get<tag>(device.storage_tuple()) = true;
    device.storage(tag{}) = true;
    device.storage<gat>() = 42;
    EXPECT_EQ(true,  device.storage<tag>());
    EXPECT_EQ(42,    device.storage<gat>());
    EXPECT_EQ(true,  common::get<tag>(device.storage_tuple()));
    EXPECT_EQ(42,    common::get<gat>(device.storage_tuple()));
}

TEST(StorageTest, Net) {
    combo1::net  network{common::make_tagged_tuple<>()};
    EXPECT_EQ(0.0, network.storage<oth>());
    EXPECT_EQ(0.0, common::get<oth>(network.storage_tuple()));
    common::get<oth>(network.storage_tuple()) = 5.0;
    network.storage(oth{}) = 2.5;
    network.storage<oth>() = 4.25;
    EXPECT_EQ(4.25, network.storage<oth>());
    EXPECT_EQ(4.25, common::get<oth>(network.storage_tuple()));
}
