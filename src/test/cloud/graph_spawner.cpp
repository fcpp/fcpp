// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"
#include "lib/component/timer.hpp"
#include "lib/cloud/graph_connector.hpp"
#include "lib/cloud/graph_spawner.hpp"

#include "test/helper.hpp"

using namespace fcpp;
using namespace component::tags;

struct tag {};
struct gat {};
struct oth {};

struct url {};

template <int O>
using combo1 = component::combine_spec<
    component::graph_spawner<
        node_attributes<url,std::string,uid,device_t>
    >,
    component::graph_connector<message_size<(O & 4) == 4>, parallel<(O & 1) == 1>, delay<distribution::constant_n<times_t, 1, 4>>>,
    component::identifier<
        parallel<(O & 1) == 1>,
        synchronised<(O & 2) == 2>
    >,
    component::storage<tuple_store<tag,bool,gat,int,start,times_t>>,
    component::base<parallel<(O & 1) == 1>>
>;
template <int O>
using combo2 = component::combine_spec<
    component::graph_spawner<
        node_attributes<url,std::string,uid,device_t>
    >,
    component::graph_connector<message_size<(O & 4) == 4>, parallel<(O & 1) == 1>, delay<distribution::constant_n<times_t, 1, 4>>>,
    component::identifier<
        parallel<(O & 1) == 1>,
        synchronised<(O & 2) == 2>
    >,
    component::storage<tuple_store<tag,bool,gat,int,start,times_t>>,
    component::base<parallel<(O & 1) == 1>>
>;

std::string twonodes =
    "1000notes.com	0\n"
    "100500.tv	1";

std::string onearc = "0	1";

MULTI_TEST(SpawnerTest, Sequence, O, 3) {
    std::stringstream ssnodes(twonodes);
    std::stringstream ssarcs(onearc);

    typename combo1<O>::net network{common::make_tagged_tuple<nodesinput,arcsinput>(&ssnodes, &ssarcs)};

    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_EQ(true, network.node_at(0).connected(1));
}

/*
MULTI_TEST(SpawnerTest, MultiSpawn, O, 2) {
    typename combo2<O>::net network{common::make_tagged_tuple<oth>("foo")};
    EXPECT_EQ(0, (int)network.node_size());
    EXPECT_EQ(1, network.next());
    network.update();
    EXPECT_EQ(1, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(0).storage_tuple()));
    EXPECT_EQ(1, network.next());
    network.update();
    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(1).storage_tuple()));
    EXPECT_EQ(1, network.next());
    network.update();
    EXPECT_EQ(3, (int)network.node_size());
    EXPECT_TRUE(common::get<tag>(network.node_at(2).storage_tuple()));
    EXPECT_EQ(2, network.next());
    network.update();
    EXPECT_EQ(4, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(3).storage_tuple()));
    EXPECT_EQ(3, network.next());
    network.update();
    EXPECT_EQ(5, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(4).storage_tuple()));
    EXPECT_EQ(4, network.next());
    network.update();
    EXPECT_EQ(6, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(5).storage_tuple()));
    EXPECT_EQ(5, network.next());
    network.update();
    EXPECT_EQ(7, (int)network.node_size());
    EXPECT_FALSE(common::get<tag>(network.node_at(6).storage_tuple()));
    EXPECT_EQ(TIME_MAX, network.next());
}
*/
