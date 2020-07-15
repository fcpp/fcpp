// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"
#include "lib/component/timer.hpp"
#include "lib/simulation/spawner.hpp"

using namespace fcpp;


struct tag {};
struct gat {};
struct oth {};

using seq_rep = sequence::multiple<distribution::constant<times_t, 1>, 3>;
using seq_per = sequence::periodic<distribution::constant<times_t, 2>, distribution::constant<times_t, 1>, distribution::constant<times_t, 5>>;
using ever_true = distribution::constant<bool, true>;
using ever_false = distribution::constant<bool, false>;

using combo1 = component::combine_spec<
    component::spawner<component::tags::spawn_schedule<seq_rep>,component::tags::init<tag,ever_true,gat,seq_per>>,
    component::identifier<component::tags::synchronised<false>>,
    component::storage<component::tags::tuple_store<tag,bool,gat,int,component::tags::start,times_t>>,
    component::base<>
>;
using combo2 = component::combine_spec<
    component::spawner<component::tags::spawn_schedule<seq_rep>,component::tags::init<tag,ever_true,gat,seq_per>>,
    component::spawner<component::tags::spawn_schedule<seq_per>,component::tags::init<tag,ever_false,gat,seq_per>>,
    component::identifier<component::tags::synchronised<false>>,
    component::storage<component::tags::tuple_store<tag,bool,gat,int,component::tags::start,times_t>>,
    component::base<>
>;
using combo3 = component::combine_spec<
    component::spawner<
        component::tags::spawn_schedule<seq_rep>,
        component::tags::init<tag,ever_true,gat,seq_per>,
        component::tags::spawn_schedule<seq_per>,
        component::tags::init<tag,ever_false,gat,seq_per>
    >,
    component::identifier<component::tags::synchronised<false>>,
    component::storage<component::tags::tuple_store<tag,bool,gat,int,component::tags::start,times_t>>,
    component::base<>
>;


TEST(SpawnerTest, Sequence) {
    combo1::net network{common::make_tagged_tuple<oth>("foo")};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(0).storage_tuple()));
    EXPECT_EQ(2, common::get<gat>(network.node_at(0).storage_tuple()));
    EXPECT_EQ(1.0, common::get<component::tags::start>(network.node_at(0).storage_tuple()));
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(1).storage_tuple()));
    EXPECT_EQ(3, common::get<gat>(network.node_at(1).storage_tuple()));
    EXPECT_EQ(1.0, common::get<component::tags::start>(network.node_at(1).storage_tuple()));
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(3, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(2).storage_tuple()));
    EXPECT_EQ(4, common::get<gat>(network.node_at(2).storage_tuple()));
    EXPECT_EQ(1.0, common::get<component::tags::start>(network.node_at(2).storage_tuple()));
    EXPECT_EQ(TIME_MAX, network.next());
    network.update();
    EXPECT_EQ(3, (int)network.node_size());
}

TEST(SpawnerTest, MultiSpawn) {
    combo2::net network{common::make_tagged_tuple<oth>("foo")};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(0).storage_tuple()));
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(1).storage_tuple()));
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(3, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(2).storage_tuple()));
    EXPECT_EQ(2.0, network.next());
    network.update();
    EXPECT_EQ(4, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(3).storage_tuple()));
    EXPECT_EQ(3.0, network.next());
    network.update();
    EXPECT_EQ(5, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(4).storage_tuple()));
    EXPECT_EQ(4.0, network.next());
    network.update();
    EXPECT_EQ(6, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(5).storage_tuple()));
    EXPECT_EQ(5.0, network.next());
    network.update();
    EXPECT_EQ(7, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(6).storage_tuple()));
    EXPECT_EQ(TIME_MAX, network.next());
}

TEST(SpawnerTest, ComboSpawn) {
    combo3::net network{common::make_tagged_tuple<oth>("foo")};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(0).storage_tuple()));
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(1).storage_tuple()));
    EXPECT_EQ(1.0, network.next());
    network.update();
    EXPECT_EQ(3, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(2).storage_tuple()));
    EXPECT_EQ(2.0, network.next());
    network.update();
    EXPECT_EQ(4, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(3).storage_tuple()));
    EXPECT_EQ(3.0, network.next());
    network.update();
    EXPECT_EQ(5, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(4).storage_tuple()));
    EXPECT_EQ(4.0, network.next());
    network.update();
    EXPECT_EQ(6, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(5).storage_tuple()));
    EXPECT_EQ(5.0, network.next());
    network.update();
    EXPECT_EQ(7, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(6).storage_tuple()));
    EXPECT_EQ(TIME_MAX, network.next());
}
