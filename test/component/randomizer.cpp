// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/randomizer.hpp"


// Component exposing the storage interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using P::node::next_double;
            using P::node::next_int;
        };
        using net  = typename P::net;
    };
};

using combo1 = fcpp::combine<exposer,fcpp::randomizer<>>;

using combo2 = fcpp::combine<exposer,fcpp::randomizer<fcpp::crand>>;


TEST(ComponentTest, Twister) {
    combo1::net  network{fcpp::make_tagged_tuple<>()};
    combo1::node device{network, fcpp::make_tagged_tuple<tags::id>(42)};
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

TEST(ComponentTest, Crand) {
    combo2::net  network{fcpp::make_tagged_tuple<>()};
    combo2::node device{network, fcpp::make_tagged_tuple<tags::id,tags::seed>(42,2)};
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
