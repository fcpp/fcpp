// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <vector>

#include "gtest/gtest.h"

#include "lib/component/scheduler.hpp"
#include "lib/deployment/hardware_connector.hpp"
#include "lib/deployment/hardware_identifier.hpp"

#include "test/fake_os.hpp"
#include "test/helper.hpp"

using namespace fcpp;
using namespace component::tags;


struct tag {};
struct gat {};
struct oth {};

// Component sending some useless messages.
struct messager {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using message_t = typename P::node::message_t::template push_back<tag,int>;
        };
        using net = typename P::net;
    };
};

using seq_per = sequence::periodic<distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 1>, distribution::constant_n<times_t, 9>>;

template <int O>
using combo = component::combine_spec<
    messager,
    component::scheduler<round_schedule<seq_per>>,
    component::hardware_connector<parallel<(O & 1) == 1>, delay<distribution::constant_n<times_t, 1, 2>>, message_push<(O & 2) == 2>>,
    component::hardware_identifier<parallel<(O & 1) == 1>>,
    component::base<parallel<(O & 1) == 1>>
>;

#define EXPECT_ROUND(t, s)                              \
        EXPECT_EQ(n.next(), double{t});                 \
        EXPECT_EQ(conn->fake_send().size(), size_t{s}); \
        n.update()

MULTI_TEST(ConnectorTest, Messages, O, 2) {
    typename combo<O>::net n{common::make_tagged_tuple<oth>("foo")};
    auto conn = n.node_at(42).connector_data();
    EXPECT_ROUND(2.0, 0);
    EXPECT_ROUND(2.5, 0);
    EXPECT_ROUND(3.0, 4);
    conn->fake_receive({3.2, 10, 2.5, {2, 0, 0, 0}});
    conn->fake_receive({3.3, 17, 3.5, {4, 0, 0, 0}});
    conn->fake_receive({3.7, 12, 3.0, {3, 0, 0, 0}});
    EXPECT_ROUND(3.5, 4);
    EXPECT_ROUND(4.0, 4);
    EXPECT_ROUND(4.5, 4);
}
