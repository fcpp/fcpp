// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#define FCPP_SYSTEM FCPP_SYSTEM_EMBEDDED
#define FCPP_ENVIRONMENT FCPP_ENVIRONMENT_PHYSICAL
#define FCPP_WARNING_TRACE false

#include "lib/fcpp.hpp"
#include "test/fake_os.hpp"
#include "test/helper.hpp"

#define DEGREE       10  // maximum degree allowed for a deployment
#define DIAMETER     10  // maximum diameter in hops for a deployment
#define WINDOW_TIME  60  // time in seconds during which positive node information is retained
#define PRESS_TIME   5   // time in seconds of button press after which termination is triggered
#define ROUND_PERIOD 1   // time in seconds between transmission rounds
#define BUFFER_SIZE  40  // size in KB to be used for buffering the output

using namespace fcpp;
using namespace component::tags;

//! @brief Storage tags
//! @{
//! @brief Total round count since start.
struct round_count {};
//! @brief A shared global clock.
struct global_clock {};
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
//! @brief Percentage of transmission success for the strongest link.
struct strongest_link {};
//! @brief List of neighbours encountered at least 50% of the times.
struct nbr_list {};
//! @brief Whether the device has been infected.
struct infected {};
//! @brief The list of positive devices in the network.
struct positives {};
//! @}


// PURE C++ FUNCTIONS

//! @brief \return the maximum stack used by the node starting from the boot
uint16_t usedStack() {
    return 42;
}

//! @brief \return the maximum heap used by the node (divided by 2 to fit in a short)
uint16_t usedHeap() {
    return 1234;
}

//! @brief Whether the button is currently pressed.
inline bool buttonPressed() {
    return false;
}


// AGGREGATE STATUS TRACKING

//! @brief Tracks the passage of time.
FUN void time_tracking(ARGS) { CODE
    node.storage(round_count{}) = coordination::counter(CALL, uint16_t{1});
    node.storage(global_clock{}) = coordination::shared_clock(CALL);
}

//! @brief Export list for time_tracking.
FUN_EXPORT time_tracking_t = common::export_list<coordination::counter_t<uint16_t>, coordination::shared_clock_t>;

//! @brief Tracks the maximum consumption of memory and message resources.
FUN void resource_tracking(ARGS) { CODE
    node.storage(max_stack{}) = coordination::gossip_max(CALL, usedStack());
    node.storage(max_heap{}) = uint32_t{2} * coordination::gossip_max(CALL, usedHeap());
    node.storage(max_msg{}) = coordination::gossip_max(CALL, (int8_t)min(node.msg_size(), size_t{127}));
}

//! @brief Export list for resource_tracking.
FUN_EXPORT resource_tracking_t = common::export_list<coordination::gossip_max_t<uint16_t>, coordination::gossip_max_t<int8_t>>;

//! @brief Records the set of neighbours connected at least 50% of the time.
FUN void topology_recording(ARGS) { CODE
    node.storage(nbr_list{}).clear();
    coordination::list_hood(CALL, node.storage(nbr_list{}), coordination::nbr_uid(CALL), coordination::nothing);

    using map_t = std::unordered_map<device_t,times_t>;
    map_t nbr_counters = coordination::old(CALL, map_t{}, [&](map_t n){
        coordination::fold_hood(CALL, [&](device_t i, times_t t, coordination::tags::nothing){
            if (t > node.previous_time()) n[i] += 1;
            return coordination::nothing;
        }, node.message_time(), coordination::nothing);
        return n;
    });
    times_t c = 0;
    for (auto const& it : nbr_counters)
        c = max(c, it.second);
    c = c * 100 / node.storage(round_count{});
    node.storage(strongest_link{}) = (int8_t)round(c);
}

//! @brief Export list for topology_recording.
FUN_EXPORT topology_recording_t = common::export_list<std::unordered_map<device_t,times_t>>;

//! @brief Checks whether to terminate the execution.
FUN void termination_check(ARGS) { CODE
    if (coordination::round_since(CALL, not buttonPressed()) >= PRESS_TIME) node.terminate();
}

//! @brief Export list for termination_check.
FUN_EXPORT termination_check_t = common::export_list<coordination::round_since_t>;


// AGGREGATE CASE STUDIES

//! @brief Computes whether there is a node with only one connected neighbour at a given time.
FUN void vulnerability_detection(ARGS, int diameter) { CODE
    tie(node.storage(min_uid{}), node.storage(hop_dist{})) = coordination::diameter_election_distance(CALL, diameter);
    bool collect_weak = coordination::sp_collection(CALL, node.storage(hop_dist{}), coordination::count_hood(CALL) <= 2, false, [](bool x, bool y) {
        return x or y;
    });
    node.storage(some_weak{}) = coordination::broadcast(CALL, node.storage(hop_dist{}), collect_weak);
}

//! @brief Export list for vulnerability_detection.
FUN_EXPORT vulnerability_detection_t = common::export_list<coordination::diameter_election_distance_t<>, coordination::sp_collection_t<hops_t, bool>, coordination::broadcast_t<hops_t, bool>>;

//! @brief Computes whether the current node got in contact with a positive node within a time window.
FUN void contact_tracing(ARGS, times_t window) { CODE
    bool positive = coordination::toggle_filter(CALL, buttonPressed());
    using contact_t = std::unordered_map<device_t, times_t>;
    contact_t contacts = coordination::old(CALL, contact_t{}, [&](contact_t c){
        // discard old contacts
        for (auto it = c.begin(); it != c.end();) {
          if (node.current_time() - it->second > window)
            it = c.erase(it);
          else ++it;
        }
        // add new contacts
        field<device_t> nbr_uids = coordination::nbr_uid(CALL);
        coordination::fold_hood(CALL, [&](device_t i, int){
            c[i] = node.current_time();
            return 0;
        }, nbr_uids, 0);
        return c;
    });
    node.storage(positives{}) = coordination::nbr(CALL, contact_t{}, [&](field<contact_t> np){
        contact_t p{};
        if (positive) p[node.uid] = node.current_time();
        coordination::fold_hood(CALL, [&](contact_t const& cs, int){
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

//! @brief Export list for contact_tracing.
FUN_EXPORT contact_tracing_t = common::export_list<coordination::toggle_filter_t, std::unordered_map<device_t, times_t>>;


// AGGREGATE MAIN AND SETTINGS

//! @brief Main aggregate function.
MAIN() {
        time_tracking(CALL);
        vulnerability_detection(CALL, DIAMETER);
        contact_tracing(CALL, WINDOW_TIME);
        resource_tracking(CALL);
        topology_recording(CALL);
        termination_check(CALL);
}


using rows_type = plot::rows<
    tuple_store<
        min_uid,        device_t,
        hop_dist,       hops_t,
        some_weak,      bool,
        infected,       bool,
        positives,      std::unordered_map<device_t, times_t>,
        max_stack,      uint16_t,
        max_heap,       uint32_t,
        max_msg,        int8_t,
        strongest_link, int8_t,
        nbr_list,       std::vector<device_t>
    >,
    tuple_store<
        plot::time,     uint16_t,
        round_count,    uint16_t,
        global_clock,   times_t
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
        time_tracking_t,
        vulnerability_detection_t,
        contact_tracing_t,
        resource_tracking_t,
        topology_recording_t,
        termination_check_t
    >,
    tuple_store< // tag/type that can appear in node.storage(tag{}) = type{}, are printed in output
        round_count,    uint16_t,
        global_clock,   times_t,
        min_uid,        device_t,
        hop_dist,       hops_t,
        some_weak,      bool,
        infected,       bool,
        positives,      std::unordered_map<device_t, times_t>,
        max_stack,      uint16_t,
        max_heap,       uint32_t,
        max_msg,        int8_t,
        strongest_link, int8_t,
        nbr_list,       std::vector<device_t>
    >,
    plot_type<rows_type>
);

#define EXPECT_ROUND(t, rc, nc, mu, hd, sw)                                 \
        std::this_thread::sleep_for(std::chrono::milliseconds(30));         \
        EXPECT_EQ(n.next(), times_t{t});                                    \
        {                                                                   \
        auto res = make_tuple(d.storage(round_count{}),                     \
                              d.storage(min_uid{}),                         \
                              d.storage(hop_dist{}),                        \
                              d.storage(some_weak{}));                      \
        EXPECT_EQ(res, make_tuple(rc, device_t{mu}, hops_t{hd}, sw));       \
        }                                                                   \
        std::cerr << "message size: " << d.connector_data()->fake_send().size() << std::endl; \
        n.update();

#ifndef FCPP_DISABLE_THREADS
TEST(EmbeddedTest, Main) {
    rows_type row_store;
    std::stringstream m_stream;
    component::deployment<opt>::net n{common::make_tagged_tuple<hoodsize,output,plotter>(device_t{DEGREE},&m_stream,&row_store)};
    auto& d = n.node_at(42);
    EXPECT_ROUND(0, 0, 0, 0, 0, false);
    EXPECT_ROUND(0, 1, 1, 42, 0, true);
    EXPECT_ROUND(1, 1, 1, 42, 0, true);
    EXPECT_ROUND(1, 2, 1, 42, 0, true);
    EXPECT_ROUND(2, 2, 1, 42, 0, true);
    EXPECT_ROUND(2, 3, 1, 42, 0, true);
    row_store.print(std::cerr);
}
#endif
