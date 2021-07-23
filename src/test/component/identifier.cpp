// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/scheduler.hpp"

#include "test/helper.hpp"

using namespace fcpp;
using namespace component::tags;


// Component losing time during rounds.
struct worker {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;

            void round_main(times_t) {
                result += workhard();
            }

            int workhard(int n=15) {
                if (n <= 1) return 1;
                return (workhard(n-1) + workhard(n-2))/2;
            }

            int result = 0;
        };
        using net = typename P::net;
    };
};

// Component exposing the storage interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        using node = typename P::node;
        struct net : public P::net {
            using P::net::net;
            using P::net::node_begin;
            using P::net::node_end;
            using P::net::node_emplace;
            using P::net::node_erase;
        };
    };
};

using seq_per = sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 62, 10>, distribution::constant_n<size_t, 5>>;

template <int O>
using combo1 = component::combine_spec<
    exposer,
    component::identifier<
        parallel<(O & 1) == 1>,
        synchronised<(O & 2) == 2>
    >,
    component::base<parallel<(O & 1) == 1>>
>;

template <int O>
using combo2 = component::combine_spec<
    exposer,
    worker,
    component::scheduler<round_schedule<seq_per>>,
    component::identifier<
        parallel<(O & 1) == 1>,
        synchronised<(O & 2) == 2>
    >,
    component::base<parallel<(O & 1) == 1>>
>;


MULTI_TEST(IdentifierTest, Sequential, O, 2) {
    typename combo1<O>::net network{common::make_tagged_tuple<>()};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(0, (int)network.node_count(0));
    EXPECT_EQ(network.node_begin(), network.node_end());
    device_t i;
    i = network.node_emplace(common::make_tagged_tuple<>());
    EXPECT_EQ(0, (int)i);
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_EQ(1, (int)network.node_count(0));
    EXPECT_NE(network.node_begin(), network.node_end());
    EXPECT_EQ(0, (int)network.node_at(0).uid);
    EXPECT_EQ(0, (int)network.node_begin()->second.uid);
    EXPECT_EQ(TIME_MAX, network.next());
    network.update();
    i = network.node_emplace(common::make_tagged_tuple<>());
    EXPECT_EQ(1, (int)i);
    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_EQ(1, (int)network.node_count(1));
    EXPECT_EQ(0, (int)network.node_erase(2));
    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_EQ(1, (int)network.node_erase(0));
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_EQ(1, (int)network.node_at(1).uid);
}

MULTI_TEST(IdentifierTest, Customised, O, 2) {
    typename combo1<O>::net network{common::make_tagged_tuple<>()};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(0, (int)network.node_count(0));
    EXPECT_EQ(network.node_begin(), network.node_end());
    device_t i;
    i = network.node_emplace(common::make_tagged_tuple<uid>(42));
    EXPECT_EQ(42, (int)i);
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_EQ(0, (int)network.node_count(0));
    EXPECT_EQ(1, (int)network.node_count(42));
    EXPECT_NE(network.node_begin(), network.node_end());
    EXPECT_EQ(42, (int)network.node_at(42).uid);
    EXPECT_EQ(42, (int)network.node_begin()->second.uid);
    EXPECT_EQ(TIME_MAX, network.next());
    network.update();
    i = network.node_emplace(common::make_tagged_tuple<uid>(24));
    EXPECT_EQ(24, (int)i);
    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_EQ(1, (int)network.node_count(24));
    EXPECT_EQ(0, (int)network.node_erase(2));
    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_EQ(1, (int)network.node_erase(42));
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_EQ(24, (int)network.node_at(24).uid);
}

MULTI_TEST(IdentifierTest, Parallel, O, 2) {
    typename combo2<O>::net network{common::make_tagged_tuple<>()};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(0, (int)network.node_count(0));
    EXPECT_EQ(network.node_begin(), network.node_end());
    for (int i=0; i<100; ++i)
        EXPECT_EQ(i, (int)network.node_emplace(common::make_tagged_tuple<>()));
    EXPECT_EQ(100, (int)network.node_size());
    EXPECT_EQ(1, (int)network.node_count(0));
    EXPECT_EQ(100, network.node_end() - network.node_begin());
    EXPECT_EQ(42, (int)network.node_at(42).uid);
    EXPECT_EQ(1.5f, network.next());
    network.update();
    EXPECT_EQ(100, (int)network.node_size());
    EXPECT_EQ(0, (int)network.node_erase(222));
    EXPECT_EQ(100, (int)network.node_size());
    EXPECT_EQ(1, (int)network.node_erase(42));
    EXPECT_EQ(99, (int)network.node_size());
}
