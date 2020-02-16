// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/storage.hpp"


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

using combo1 = fcpp::combine<exposer,fcpp::storage<tag,bool,gat,int>>;


TEST(ComponentTest, Move) {
    combo1::net  net1{fcpp::make_tagged_tuple<>()};
    combo1::net  net2 = std::move(net1);
    combo1::node dev1{net2, fcpp::make_tagged_tuple<>()};
    combo1::node dev2 = std::move(dev1);
    net2.run();
    dev2.update();
}

TEST(ComponentTest, Storage) {
    combo1::net  network{fcpp::make_tagged_tuple<>()};
    combo1::node device{network, fcpp::make_tagged_tuple<oth,gat>('b',3)};
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
