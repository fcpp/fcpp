// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

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

#define EXPECT_ROUND(t, send, ...)                                      \
        std::this_thread::sleep_for(std::chrono::milliseconds(30));     \
        EXPECT_EQ(n.next(), times_t{t});                                \
        EXPECT_EQ(details::get_ids(n.node_at(42).nbr_dist()),           \
                  (std::vector<device_t>__VA_ARGS__));                  \
        EXPECT_EQ(conn->fake_send().size(), send ? sizeof(int)+1 : 0);  \
        n.update();

#ifndef FCPP_DISABLE_THREADS
MULTI_TEST(HardwareConnectorTest, Messages, O, 2) {
    bool message_push = (O & 2) == 2;
    typename combo<O>::net n{common::make_tagged_tuple<oth>("foo")};
    auto conn = n.node_at(42).connector_data();
    EXPECT_ROUND(2, false, {});
    EXPECT_ROUND(2.5f, false, {});
    EXPECT_ROUND(3, true,  {});
    conn->fake_receive({3.2f, 10, 2.5f, {2, 0, 0, 0, 0}});
    conn->fake_receive({3.3f, 17, 3.5f, {4, 0, 0, 0, 0}});
    conn->fake_receive({3.4f, 17, 4.5f, {42}});
    conn->fake_receive({3.5f, 17, 5.5f, {24, 0, 0, 0, 0, 0, 0, 0}});
    if (message_push) {
        EXPECT_ROUND(3.5f, false, {10, 17});
    } else {
        EXPECT_ROUND(3.5f, false, {});
    }
    if (message_push) {
        EXPECT_ROUND(4, true,  {10, 17});
    } else {
        EXPECT_ROUND(4, true,  {});
    }
    EXPECT_ROUND(4.5f, false, {10, 17});
    conn->fake_receive({3.7f, 12, 3, {3, 0, 0, 0, 0}});
    if (message_push) {
        EXPECT_ROUND(5, true,  {10, 12, 17});
    } else {
        EXPECT_ROUND(5, true,  {10, 17});
    }
    EXPECT_ROUND(5.5f, false, {10, 12, 17});
}
#endif
