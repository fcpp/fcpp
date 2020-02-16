// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"


// Empty component.
template <bool b=true, int i=0>
struct empty {
    template <typename F, typename P>
    struct component {
        using node = typename P::node;
        using net  = typename P::net;
    };
};

// HG2G-themed component.
struct theanswer {
    template <typename F, typename P>
    struct component {
        struct node : public P::node {
            using P::node::node;
            
            int tester() {
                return 4;
            }
            
            int virtualize() {
                return P::node::as_final().tester() - 10;
            }
        };
        struct net : public P::net {
            using P::net::net;
            
            int retest() {
                return 2;
            }
        };
    };
};

// A component calling parents' functions.
struct caller {
    template <typename F, typename P>
    struct component {
        struct node : public P::node {
            using P::node::node;
            
            int tester() {
                return P::node::net.retest() + 1;
            }
            
            int virtualize() {
                return 7 * P::node::virtualize();
            }
        };
        using net = typename P::net;
    };
};

// A component overwriting retest.
struct overwriter {
    template <typename F, typename P>
    struct component {
        using node = typename P::node;
        struct net : public P::net {
            using P::net::net;
            
            int retest() {
                return 11*P::net::retest();
            }
            
            int something() {
                return 7;
            }
        };
    };
};

using combo1 = fcpp::combine<empty<true,2>, overwriter, empty<>, caller, theanswer, empty<false>>;

using combo2 = fcpp::combine<theanswer, caller, overwriter>;


TEST(ComponentTest, Move) {
    combo1::net  net1{fcpp::make_tagged_tuple<>()};
    combo1::net  net2 = std::move(net1);
    combo1::node dev1{net2, fcpp::make_tagged_tuple<>()};
    combo1::node dev2 = std::move(dev1);
    net2.run();
    dev2.update();
}

TEST(ComponentTest, Override) {
    combo2::net  network{fcpp::make_tagged_tuple<>()};
    combo2::node device{network, fcpp::make_tagged_tuple<>()};
    EXPECT_EQ(7,  network.something());
    EXPECT_EQ(2,  network.retest());
    EXPECT_EQ(4,  device.tester());
    EXPECT_EQ(-6, device.virtualize());
}

TEST(ComponentTest, Virtualize) {
    combo1::net  network{fcpp::make_tagged_tuple<>()};
    combo1::node device{network, fcpp::make_tagged_tuple<>()};
    EXPECT_EQ(7,  network.something());
    EXPECT_EQ(22, network.retest());
    EXPECT_EQ(23, device.tester());
    EXPECT_EQ(91, device.virtualize());
}
