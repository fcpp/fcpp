// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/fcpp.hpp"
#include "test/fake_os.hpp"
#include "test/helper.hpp"

#define DIAMETER  10 // maximum diameter in hops for a deployment

using namespace fcpp;
using namespace component::tags;

//! @brief Storage tags
//! @{
//! @brief Total round count since start.
struct round_count {};
//! @brief Current count of neighbours.
struct neigh_count {};
//! @brief Minimum UID in the network.
struct min_uid {};
//! @brief Distance in hops to the device with minimum UID.
struct hop_dist {};
//! @brief Whether some device in the network has only one neighbour.
struct some_weak {};
//! @}


//! @brief Main aggregate function.
FUN() void test_program(ARGS, int diameter) { CODE
    node.storage(round_count{}) = coordination::counter(CALL, hops_t{1});
    node.storage(neigh_count{}) = count_hood(CALL);
    node.storage(min_uid{}) = coordination::diameter_election(CALL, diameter);
    node.storage(hop_dist{}) = coordination::abf_hops(CALL, node.uid == node.storage(min_uid{}));
    bool collect_weak = coordination::sp_collection(CALL, node.storage(hop_dist{}), node.storage(neigh_count{}) <= 2, false, [](bool x, bool y) {
        return x or y;
    });
    node.storage(some_weak{}) = coordination::broadcast(CALL, node.storage(hop_dist{}), collect_weak);
}


//! @brief Main struct calling `test_program`.
MAIN(test_program,,DIAMETER);

//! @brief FCPP setup.
DECLARE_OPTIONS(opt,
    program<main>,
    round_schedule<sequence::periodic_n<1, 1, 1>>,
    exports<
        bool, hops_t, device_t, tuple<device_t, hops_t>
    >,
    tuple_store<
        round_count,int,
        neigh_count,int,
        min_uid,    device_t,
        hop_dist,   hops_t,
        some_weak,  bool
    >
);

#define EXPECT_ROUND(t, rc, nc, mu, hd, sw)                                 \
        std::this_thread::sleep_for(std::chrono::milliseconds(30));         \
        EXPECT_EQ(n.next(), double{t});                                     \
        {                                                                   \
        auto res = make_tuple(d.storage(round_count{}),                     \
                              d.storage(neigh_count{}),                     \
                              d.storage(min_uid{}),                         \
                              d.storage(hop_dist{}),                        \
                              d.storage(some_weak{}));                      \
        EXPECT_EQ(res, make_tuple(rc, nc, device_t{mu}, hops_t{hd}, sw));   \
        }                                                                   \
        std::cerr << "message size: " << d.connector_data()->fake_send().size() << std::endl; \
        n.update();

//! @brief Main function starting FCPP.
int main() {
    std::stringstream m_stream;
    component::deployment<opt>::net n{common::make_tagged_tuple<output>(&m_stream)};
    auto& d = n.node_at(42);
    EXPECT_ROUND(0.0, 0, 0, 0, 0, false);
    EXPECT_ROUND(0.0, 1, 1, 42, 0, true);
    EXPECT_ROUND(1.0, 1, 1, 42, 0, true);
    EXPECT_ROUND(1.0, 2, 1, 42, 0, true);
    EXPECT_ROUND(2.0, 2, 1, 42, 0, true);
    EXPECT_ROUND(2.0, 3, 1, 42, 0, true);
    return 0;
}
