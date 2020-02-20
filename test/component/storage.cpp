// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/storage.hpp"

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
            using P::node::storage;
            using P::node::storage_tuple;
        };
        using net  = typename P::net;
    };
};

using combo1 = component::combine<exposer,component::storage<tag,bool,gat,int>>;


TEST(StorageTest, Storage) {
    combo1::net  network{fcpp::make_tagged_tuple<>()};
    combo1::node device{network, fcpp::make_tagged_tuple<component::tags::uid,oth,gat>(7,'b',3)};
    EXPECT_EQ(false, device.storage<tag>());
    EXPECT_EQ(3,     device.storage<gat>());
    EXPECT_EQ(false, fcpp::get<tag>(device.storage_tuple()));
    EXPECT_EQ(3,     fcpp::get<gat>(device.storage_tuple()));
    device.storage<tag>() = true;
    device.storage<gat>() = 42;
    EXPECT_EQ(true,  device.storage<tag>());
    EXPECT_EQ(42,    device.storage<gat>());
    EXPECT_EQ(true,  fcpp::get<tag>(device.storage_tuple()));
    EXPECT_EQ(42,    fcpp::get<gat>(device.storage_tuple()));
}
