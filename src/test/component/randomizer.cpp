// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/randomizer.hpp"

using namespace fcpp;


// Component exposing the storage interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using P::node::next_int;
            using P::node::next_double;
            using P::node::random_error;
        };
        struct net : public P::net {
            using P::net::net;
            using P::net::next_int;
            using P::net::next_double;
            using P::net::random_error;
        };
    };
};

using combo1 = component::combine_spec<exposer,component::randomizer<>,component::base<>>;

using combo2 = component::combine_spec<
    exposer,
    component::randomizer<component::tags::generator<crand>>,
    component::base<>
>;


TEST(RandomizerTest, Twister) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node device{network, common::make_tagged_tuple<component::tags::uid>(42)};
    for (int i=0; i<1000; ++i) {
        EXPECT_LE(0, device.next_int());
        EXPECT_LE(0, device.next_int(9));
        EXPECT_GE(9, device.next_int(9));
        EXPECT_LE(3, device.next_int(3, 8));
        EXPECT_GE(8, device.next_int(3, 8));
        EXPECT_LE(0.0, device.next_double());
        EXPECT_GE(1.0, device.next_double());
        EXPECT_LE(0.0, device.next_double(9.0));
        EXPECT_GE(9.0, device.next_double(9.0));
        EXPECT_LE(3.0, device.next_double(3.0, 8.0));
        EXPECT_GE(8.0, device.next_double(3.0, 8.0));
    }
}

TEST(RandomizerTest, Crand) {
    combo2::net  network{common::make_tagged_tuple<>()};
    combo2::node device{network, common::make_tagged_tuple<component::tags::uid,component::tags::seed>(42,2)};
    for (int i=0; i<1000; ++i) {
        EXPECT_LE(0, device.next_int());
        EXPECT_LE(0, device.next_int(9));
        EXPECT_GE(9, device.next_int(9));
        EXPECT_LE(3, device.next_int(3, 8));
        EXPECT_GE(8, device.next_int(3, 8));
        EXPECT_LE(0.0, device.next_double());
        EXPECT_GE(1.0, device.next_double());
        EXPECT_LE(0.0, device.next_double(9.0));
        EXPECT_GE(9.0, device.next_double(9.0));
        EXPECT_LE(3.0, device.next_double(3.0, 8.0));
        EXPECT_GE(8.0, device.next_double(3.0, 8.0));
    }
}

TEST(RandomizerTest, Net) {
    combo1::net  network{common::make_tagged_tuple<component::tags::seed>(20)};
    for (int i=0; i<1000; ++i) {
        EXPECT_LE(0, network.next_int());
        EXPECT_LE(0, network.next_int(9));
        EXPECT_GE(9, network.next_int(9));
        EXPECT_LE(3, network.next_int(3, 8));
        EXPECT_GE(8, network.next_int(3, 8));
        EXPECT_LE(0.0, network.next_double());
        EXPECT_GE(1.0, network.next_double());
        EXPECT_LE(0.0, network.next_double(9.0));
        EXPECT_GE(9.0, network.next_double(9.0));
        EXPECT_LE(3.0, network.next_double(3.0, 8.0));
        EXPECT_GE(8.0, network.next_double(3.0, 8.0));
    }
}

TEST(RandomizerTest, Error) {
    combo1::net  network{common::make_tagged_tuple<component::tags::seed>(20)};
    double d;
    d = 0;
    for (int i=0; i<10000; ++i)
        d += network.random_error<std::uniform_real_distribution>(5.0, 0.1, 0.5);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += network.random_error<std::normal_distribution>(5.0, 0.1, 0.5);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += network.random_error<std::weibull_distribution>(5.0, 0.1, 0.5);
    EXPECT_NEAR(50000.0, d, 300.0);
}
