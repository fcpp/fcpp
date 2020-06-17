// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"
#include "lib/component/base.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"
#include "lib/component/timer.hpp"
#include "lib/simulation/spawner.hpp"

using namespace fcpp;


struct tag {};
struct gat {};
struct oth {};

using seq_rep = random::sequence_multiple<random::constant_distribution<times_t, 1>, 3>;
using seq_per = random::sequence_periodic<random::constant_distribution<times_t, 2>, random::constant_distribution<times_t, 1>, random::constant_distribution<times_t, 9>>;
using ever_true = random::constant_distribution<bool, true>;

using combo1 = component::combine<
    component::spawner<seq_rep,tag,ever_true,gat,seq_per>,
    component::identifier<component::tags::synchronised<false>>,
    component::storage<component::tags::tuple_store<tag,bool,gat,int,component::tags::start,times_t>>
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
