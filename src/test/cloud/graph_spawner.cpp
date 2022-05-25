// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"
#include "lib/component/timer.hpp"
#include "lib/option/distribution.hpp"
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
        node_attributes<url,std::string,uid,device_t>,
        init<start,distribution::interval_n<times_t, 0, 1>>
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

MULTI_TEST(GraphSpawnerTest, Sequence, O, 3) {
    std::stringstream ssnodes(twonodes);
    std::stringstream ssarcs(onearc);

    typename combo1<O>::net network{common::make_tagged_tuple<nodesinput,arcsinput>(&ssnodes, &ssarcs)};

    EXPECT_EQ(2, (int)network.node_size());
    EXPECT_EQ(true, network.node_at(0).connected(1));
}
