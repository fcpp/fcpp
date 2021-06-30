// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/scheduler.hpp"
#include "lib/component/storage.hpp"
#include "lib/deployment/hardware_identifier.hpp"

#include "test/fake_os.hpp"
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

struct tag {};
struct gat {};

using seq_per = sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 62, 10>, distribution::constant_n<size_t, 5>>;

template <int O>
using combo1 = component::combine_spec<
    component::hardware_identifier<parallel<(O & 1) == 1>>,
    component::storage<tuple_store<tag,bool,gat,int>>,
    component::base<parallel<(O & 1) == 1>>
>;

template <int O>
using combo2 = component::combine_spec<
    worker,
    component::scheduler<round_schedule<seq_per>>,
    component::hardware_identifier<parallel<(O & 1) == 1>>,
    component::base<parallel<(O & 1) == 1>>
>;


MULTI_TEST(HardwareIdentifierTest, Storage, O, 1) {
    typename combo1<O>::net network{common::make_tagged_tuple<tag,gat>(false,10)};
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_EQ(0, (int)network.node_count(0));
    EXPECT_EQ(1, (int)network.node_count(42));
    EXPECT_EQ(false, network.node_at(42).storage(tag{}));
    EXPECT_EQ(10, network.node_at(42).storage(gat{}));
    EXPECT_EQ(TIME_MAX, network.next());
    network.update();
    EXPECT_EQ(TIME_MAX, network.next());
}

MULTI_TEST(HardwareIdentifierTest, Schedule, O, 1) {
    typename combo2<O>::net network{common::make_tagged_tuple<>()};
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_EQ(0, (int)network.node_count(0));
    EXPECT_EQ(1, (int)network.node_count(42));
    EXPECT_EQ(1.5f, network.next());
    network.update();
    EXPECT_EQ(3.5f, network.next());
    network.update();
    EXPECT_EQ(5.5f, network.next());
    network.update();
    EXPECT_EQ(TIME_MAX, network.next());
}
