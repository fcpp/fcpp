// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"
#include "lib/component/timer.hpp"
#include "lib/simulation/spawner.hpp"

#include "test/helper.hpp"

using namespace fcpp;
using namespace component::tags;


struct tag {};
struct gat {};
struct oth {};

using seq_rep = sequence::multiple_n<3, 1>;
using seq_per = sequence::periodic<distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 1>, distribution::constant_n<times_t, 5>>;
using ever_true = distribution::constant_n<bool, true>;
using ever_false = distribution::constant_n<bool, false>;

using tt = common::tagged_tuple_t<tag,bool,gat,int,start,times_t>;

template <int O>
using combo1 = component::combine_spec<
    component::spawner<spawn_schedule<seq_rep>, init<tag,ever_true,gat,seq_per>>,
    component::identifier<
        parallel<(O & 1) == 1>,
        synchronised<(O & 2) == 2>
    >,
    component::storage<tuple_store<tag,bool,gat,int,start,times_t>>,
    component::base<parallel<(O & 1) == 1>>
>;
template <int O>
using combo2 = component::combine_spec<
    component::spawner<
        spawn_schedule<seq_rep>, init<tag,ever_true,gat,seq_per>,
        spawn_schedule<seq_per>, init<tag,ever_false,gat,seq_per>
    >,
    component::identifier<
        parallel<(O & 1) == 1>,
        synchronised<(O & 2) == 2>
    >,
    component::storage<tuple_store<tag,bool,gat,int,start,times_t>>,
    component::base<parallel<(O & 1) == 1>>
>;


MULTI_TEST(SpawnerTest, Sequence, O, 2) {
    typename combo1<O>::net network{common::make_tagged_tuple<oth>("foo")};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(1, network.next());
    network.update();
    EXPECT_EQ(3, (int)network.node_size());
    EXPECT_EQ(network.node_at(0).storage_tuple(), tt(true, 2, 1));
    EXPECT_EQ(network.node_at(1).storage_tuple(), tt(true, 3, 1));
    EXPECT_EQ(network.node_at(2).storage_tuple(), tt(true, 4, 1));
    EXPECT_EQ(TIME_MAX, network.next());
    network.update();
    EXPECT_EQ(3, (int)network.node_size());
}

MULTI_TEST(SpawnerTest, MultiSpawn, O, 2) {
    typename combo2<O>::net network{common::make_tagged_tuple<oth>("foo")};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(1, network.next());
    network.update();
    EXPECT_EQ(3, (int)network.node_size());
    EXPECT_EQ(network.node_at(0).storage_tuple(), tt(true, 2, 1));
    EXPECT_EQ(network.node_at(1).storage_tuple(), tt(true, 3, 1));
    EXPECT_EQ(network.node_at(2).storage_tuple(), tt(true, 4, 1));
    EXPECT_EQ(2, network.next());
    network.update();
    EXPECT_EQ(4, (int)network.node_size());
    EXPECT_EQ(network.node_at(3).storage_tuple(), tt(false, 2, 2));
    EXPECT_EQ(3, network.next());
    network.update();
    EXPECT_EQ(5, (int)network.node_size());
    EXPECT_EQ(network.node_at(4).storage_tuple(), tt(false, 3, 3));
    EXPECT_EQ(4, network.next());
    network.update();
    EXPECT_EQ(6, (int)network.node_size());
    EXPECT_EQ(network.node_at(5).storage_tuple(), tt(false, 4, 4));
    EXPECT_EQ(5, network.next());
    network.update();
    EXPECT_EQ(7, (int)network.node_size());
    EXPECT_EQ(network.node_at(6).storage_tuple(), tt(false, 5, 5));
    EXPECT_EQ(TIME_MAX, network.next());
    network.update();
    EXPECT_EQ(7, (int)network.node_size());
}
