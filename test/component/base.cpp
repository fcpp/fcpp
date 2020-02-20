// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#define FCPP_REALTIME 0.0

#include <limits>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"


// Empty component.
template <bool b=true, int i=0>
struct empty {
    template <typename F, typename P>
    struct component : public P {
        using node = typename P::node;
        using net  = typename P::net;
    };
};

// HG2G-themed component.
struct theanswer {
    template <typename F, typename P>
    struct component : public P {
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
    struct component : public P {
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
    struct component : public P {
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

// A component exposing the real time data in net.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        using node = typename P::node;
        struct net : public P::net {
            using P::net::net;
            using P::net::real_time;
        };
    };
};

using combo1 = fcpp::combine<empty<true,2>, overwriter, empty<>, caller, theanswer, empty<false>>;

using combo2 = fcpp::combine<exposer, theanswer, caller, overwriter>;

// slow computation
int workhard(int n=15) {
    if (n <= 1) return 1;
    return (workhard(n-1) + workhard(n-2))/2;
}


TEST(BaseTest, UID) {
    combo1::net  net1{fcpp::make_tagged_tuple<>()};
    combo1::node dev1{net1, fcpp::make_tagged_tuple<tags::uid>(42)};
    net1.run();
    dev1.update();
    EXPECT_EQ(size_t(42), dev1.uid);
}

TEST(BaseTest, Override) {
    combo2::net  network{fcpp::make_tagged_tuple<>()};
    combo2::node device{network, fcpp::make_tagged_tuple<tags::uid>(42)};
    EXPECT_EQ(7,  network.something());
    EXPECT_EQ(2,  network.retest());
    EXPECT_EQ(4,  device.tester());
    EXPECT_EQ(-6, device.virtualize());
}

TEST(BaseTest, Virtualize) {
    combo1::net  network{fcpp::make_tagged_tuple<>()};
    combo1::node device{network, fcpp::make_tagged_tuple<tags::uid>(42)};
    EXPECT_EQ(7,  network.something());
    EXPECT_EQ(22, network.retest());
    EXPECT_EQ(23, device.tester());
    EXPECT_EQ(91, device.virtualize());
}

TEST(BaseTest, RealTime) {
    int acc;
    combo2::net net1{fcpp::make_tagged_tuple<int>('a')};
    // just waste some time in a non-optimizable way
    for (int i=acc=0; i<1000; ++i) acc += workhard();
    EXPECT_EQ(1000, acc);
    EXPECT_EQ(fcpp::times_t(0.0), net1.real_time());
    combo2::net net2{fcpp::make_tagged_tuple<tags::realtime>(std::numeric_limits<double>::infinity())};
    EXPECT_EQ(fcpp::TIME_MAX,     net2.real_time());
    combo2::net net3{fcpp::make_tagged_tuple<tags::realtime>(1.0)};
    // just waste some time in a non-optimizable way
    for (int i=acc=0; i<1000; ++i) acc += workhard();
    EXPECT_EQ(1000, acc);
    EXPECT_LT(fcpp::times_t(0.0), net3.real_time());
    EXPECT_GT(fcpp::TIME_MAX,     net3.real_time());
}
