// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/fcpp.hpp"
#include "test/fake_os.hpp"
#include "test/helper.hpp"

#define DEGREE       10  // maximum degree allowed for a deployment
#define DIAMETER     10  // maximum diameter in hops for a deployment
#define WINDOW_TIME  60  // time in seconds during which positive node information is retained
#define ROUND_PERIOD 1   // time in seconds between transmission rounds
#define BUFFER_SIZE  80  // size in KB to be used for buffering the output

using namespace fcpp;
using namespace component::tags;

//! @brief \return the maximum stack used by the node starting from the boot
uint16_t getMaxStackUsed() {
    return 42;
}

//! @brief \return the maximum heap used by the node (divided by 2 to fit in a short)
uint16_t getMaxHeapUsed() {
    return 1234;
}

//! @brief Storage tags
//! @{
//! @brief Total round count since start.
struct round_count {};
//! @brief A shared global clock.
struct global_clock {};
//! @brief Current count of neighbours.
struct neigh_count {};
//! @brief Current count of neighbours from which I received a packet now.
struct neigh_now {};
//! @brief Count of neighbours from which I received a packet now and in the last round.
struct neigh_twice {};
//! @brief Minimum UID in the network.
struct min_uid {};
//! @brief Distance in hops to the device with minimum UID.
struct hop_dist {};
//! @brief Whether some device in the network has only one neighbour.
struct some_weak {};
//! @brief Maximum stack size ever experienced.
struct max_stack {};
//! @brief Maximum heap size ever experienced.
struct max_heap {};
//! @brief Maximum message size ever experienced.
struct max_msg {};
//! @brief List of neighbours encountered at least 50% of the times.
struct nbr_list {};
//! @brief Whether the device has been infected.
struct infected {};
//! @brief The list of positive devices in the network.
struct positives {};
//! @}

//! @brief Does not change a value after a given time.
FUN(T) T fix_after(ARGS, T value, times_t t) { CODE
    return old(node, 0, value, [&](T o){
        return node.current_time() < t ? value : o;
    });
}

//! @brief Computes the number of neighbours from which I am receiving twice in a row.
FUN() uint8_t recurring_neighbours(ARGS) { CODE
    return old(node, 0, field<device_t>(node.uid), [&](field<device_t> x){
        field<device_t> nbr_uids = mux(node.message_time() > node.previous_time(), nbr_uid(node, 0), node.uid);
        uint8_t c = fold_hood(node, 0, [&](tuple<device_t,device_t> val, int acc){
            return acc + (get<0>(val) != node.uid and get<1>(val) != node.uid);
        }, make_tuple(x, nbr_uids), 0);
        return make_tuple(c, nbr_uids);
    });
}

//! @brief Tracks the maximum consumption of memory and message resources.
FUN() void resource_tracking(ARGS) { CODE
    node.storage(max_stack{}) = coordination::gossip_max(node, 0, getMaxStackUsed());
    node.storage(max_heap{}) = uint32_t{2} * coordination::gossip_max(node, 1, getMaxHeapUsed());
    node.storage(max_msg{}) = coordination::gossip_max(node, 2, (int8_t)min(node.msg_size(), size_t{127}));
}

//! @brief Records the set of neighbours connected at least 50% of the time.
FUN() void topology_recording(ARGS) { CODE
    using map_t = std::unordered_map<device_t,times_t>;
    field<device_t> nbr_uids = nbr_uid(node, 0);
    map_t nbr_counters = old(node, 1, map_t{}, [&](map_t n){
        fold_hood(node, 2, [&](device_t i, int){
            n[i] += 1;
            return 0;
        }, nbr_uids, 0);
        return n;
    });
    node.storage(nbr_list{}).clear();
    for (const auto& it : nbr_counters)
        if (it.second >= node.storage(round_count{})/2)
            node.storage(nbr_list{}).insert(it.first);
}

//! @brief Computes whether there is a node with only one connected neighbour at a given time.
FUN() void vulnerability_detection(ARGS, int diameter) { CODE
    tie(node.storage(min_uid{}), node.storage(hop_dist{})) = coordination::diameter_election_distance(node, 0, diameter);
    bool collect_weak = coordination::sp_collection(node, 1, node.storage(hop_dist{}), node.storage(neigh_count{}) <= 2, false, [](bool x, bool y) {
        return x or y;
    });
    node.storage(some_weak{}) = coordination::broadcast(node, 2, node.storage(hop_dist{}), collect_weak);
}

//! @brief Computes whether the current node got in contact with a positive node within a time window.
FUN() void contact_tracing(ARGS, times_t window, bool positive) { CODE
    using contact_t = std::unordered_map<device_t, times_t>;
    contact_t contacts = old(node, 0, contact_t{}, [&](contact_t c){
        // discard old contacts
        for (auto it = c.begin(); it != c.end();) {
          if (node.current_time() - it->second > window)
            it = c.erase(it);
          else ++it;
        }
        // add new contacts
        field<device_t> nbr_uids = nbr_uid(node, 1);
        fold_hood(node, 2, [&](device_t i, int){
            c[i] = node.current_time();
            return 0;
        }, nbr_uids, 0);
        return c;
    });
    node.storage(positives{}) = nbr(node, 3, contact_t{}, [&](field<contact_t> np){
        contact_t p{};
        if (positive) p[node.uid] = node.current_time();
        fold_hood(node, 4, [&](contact_t const& cs, int){
            for (auto c : cs)
                if (node.current_time() - c.second < window)
                    p[c.first] = max(p[c.first], c.second);
            return 0;
        }, np, 0);
        return p;
    });
    node.storage(infected{}) = false;
    for (auto c : node.storage(positives{}))
        if (contacts.count(c.first))
            node.storage(infected{}) = true;
}

//! @brief Main aggregate function.
FUN() void case_study(ARGS) { CODE
    node.storage(round_count{}) = coordination::counter(node, 0, uint16_t{1});
    node.storage(global_clock{}) = coordination::shared_clock(node, 1);
    node.storage(neigh_count{}) = count_hood(node, 2);
    node.storage(neigh_now{}) = coordination::sum_hood(node, 2, node.message_time() > node.previous_time(), 0);
    node.storage(neigh_twice{}) = recurring_neighbours(node, 3);
    vulnerability_detection(node, 7, DIAMETER);
    contact_tracing(node, 4, WINDOW_TIME, false);
    resource_tracking(node, 5);
    topology_recording(node, 6);
    if (node.current_time() >= 300) node.terminate();
}


//! @brief Main struct calling `case_study`.
MAIN(case_study,);

using rows_type = plot::rows<
    tuple_store<
        neigh_count,    uint8_t,
        min_uid,        device_t,
        hop_dist,       hops_t,
        some_weak,      bool,
        max_stack,      uint16_t,
        max_heap,       uint32_t,
        max_msg,        int8_t,
        infected,       bool,
        positives,      std::unordered_map<device_t, times_t>,
        nbr_list,       std::unordered_set<device_t>
    >,
    tuple_store<
        plot::time,     uint16_t,
        round_count,    uint16_t,
        global_clock,   times_t,
        neigh_now,      uint8_t,
        neigh_twice,    uint8_t
    >,
    void,
    BUFFER_SIZE*1024
>;

//! @brief FCPP setup.
DECLARE_OPTIONS(opt,
    program<main>,
    retain<metric::retain<5, 1>>, // messages are thrown away after 5/1 secs
    round_schedule<sequence::periodic_n<1, ROUND_PERIOD, ROUND_PERIOD>>, // rounds are happening every 1 secs (den, start, period)
    exports< // types that may appear in messages
        bool, hops_t, device_t, int8_t, uint16_t, times_t, tuple<device_t, hops_t>,
        field<device_t>, std::unordered_map<device_t, times_t>
    >,
    tuple_store< // tag/type that can appear in node.storage(tag{}) = type{}, are printed in output
        round_count,    uint16_t,
        global_clock,   times_t,
        neigh_now,      uint8_t,
        neigh_twice,    uint8_t,
        neigh_count,    uint8_t,
        min_uid,        device_t,
        hop_dist,       hops_t,
        some_weak,      bool,
        max_stack,      uint16_t,
        max_heap,       uint32_t,
        max_msg,        int8_t,
        infected,       bool,
        positives,      std::unordered_map<device_t, times_t>,
        nbr_list,       std::unordered_set<device_t>
    >,
    plot_type<rows_type>
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
    rows_type row_store;
    std::stringstream m_stream;
    component::deployment<opt>::net n{common::make_tagged_tuple<hoodsize,output,plotter>(device_t{DEGREE},&m_stream,&row_store)};
    auto& d = n.node_at(42);
    EXPECT_ROUND(0.0, 0, 0, 0, 0, false);
    EXPECT_ROUND(0.0, 1, 1, 42, 0, true);
    EXPECT_ROUND(1.0, 1, 1, 42, 0, true);
    EXPECT_ROUND(1.0, 2, 1, 42, 0, true);
    EXPECT_ROUND(2.0, 2, 1, 42, 0, true);
    EXPECT_ROUND(2.0, 3, 1, 42, 0, true);
    row_store.print(std::cerr);
    return 0;
}
