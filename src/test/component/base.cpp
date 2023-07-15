// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#define FCPP_REALTIME 0

#include <limits>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"

using namespace fcpp;
using namespace component::tags;


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

template <class... Ts>
struct stuffer {
    template <typename F, typename P>
    struct component : public P {
        using node = typename P::node;
        struct net : public P::net {
            using P::net::net;

            void fail() {
                assert(false);
            }
        };
    };
};
template <class... Ts>
struct spuffer : public stuffer<Ts...> {};
template <class... Ts>
struct scuffer : public stuffer<Ts...> {};
template <class... Ts>
struct sbuffer : public stuffer<Ts...> {};

using combo1 = component::combine_spec<empty<true,2>, overwriter, empty<>, caller, theanswer, empty<false>, component::base<>>;

using combo2 = component::combine_spec<theanswer, caller, overwriter, component::base<>>;


// slow computation
int workhard(int n=15) {
    if (n <= 1) return 1;
    return (workhard(n-1) + workhard(n-2))/2;
}


TEST(BaseTest, UID) {
    combo1::net  net1{common::make_tagged_tuple<>()};
    combo1::node dev1{net1, common::make_tagged_tuple<uid>(42)};
    net1.run();
    dev1.update();
    EXPECT_EQ(size_t(42), dev1.uid);
}

TEST(BaseTest, Override) {
    combo2::net  network{common::make_tagged_tuple<>()};
    combo2::node device{network, common::make_tagged_tuple<uid>(42)};
    EXPECT_EQ(7,  network.something());
    EXPECT_EQ(2,  network.retest());
    EXPECT_EQ(4,  device.tester());
    EXPECT_EQ(-6, device.virtualize());
}

TEST(BaseTest, Virtualize) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node device{network, common::make_tagged_tuple<uid>(42)};
    EXPECT_EQ(7,  network.something());
    EXPECT_EQ(22, network.retest());
    EXPECT_EQ(23, device.tester());
    EXPECT_EQ(91, device.virtualize());
}
