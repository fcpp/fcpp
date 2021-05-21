// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/timer.hpp"

using namespace fcpp;
using namespace component::tags;


// Very simple scheduler performing updates every times_t(10).
struct scheduler {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;

            times_t next() const {
                return m_next;
            }

            void update() {
                times_t t = P::node::as_final().next();
                m_next += 10;
                P::node::round(t);
            }

            times_t m_next = 0;
        };
        struct net : public P::net {
            using P::net::net;

            times_t next() const {
                return m_next;
            }

            void update() {
                m_next += 20;
            }

            times_t m_next = 0;
        };
    };
};

// Component exposing the timer interface and planning rounds if `plan` is true.
template <bool plan>
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using P::node::receive;
            using P::node::previous_time;
            using P::node::current_time;
            using P::node::next_time;
            using P::node::terminate;
            using P::node::message_time;
            using P::node::frequency;

            void round_main(times_t t) {
                EXPECT_EQ(t, current_time());
                EXPECT_LT(previous_time(), current_time());
                EXPECT_LT(current_time(), next_time());
                if (plan) next_time(current_time() + 5);
            }
        };
        struct net : public P::net {
            using P::net::net;
            using P::net::terminate;
            using P::net::frequency;
        };
    };
};

using combo1 = component::combine_spec<exposer<true>,component::timer<>,component::base<>>;
using combo2 = component::combine_spec<exposer<false>,component::timer<>,scheduler,component::base<>>;


TEST(TimerTest, NodePlanning) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node device{network, common::make_tagged_tuple<uid,start>(1,2)};
    EXPECT_EQ(2, device.next_time());
    device.update();
    EXPECT_EQ(2, device.current_time());
    EXPECT_EQ(7, device.next_time());
    device.receive(3, 2, common::make_tagged_tuple<>());
    device.receive(4, 3, common::make_tagged_tuple<>());
    device.update();
    EXPECT_EQ(2, device.previous_time());
    EXPECT_EQ(7, device.current_time());
    EXPECT_EQ(12, device.next_time());
    fcpp::field<bool> f = device.message_time() == details::make_field({1,2,3}, std::vector<times_t>{TIME_MIN,2,3,4});
    EXPECT_TRUE(f);
    device.next_time(8);
    EXPECT_EQ(8, device.next_time());
    device.update();
    EXPECT_EQ(7, device.previous_time());
    EXPECT_EQ(8, device.current_time());
    EXPECT_EQ(13, device.next_time());
    device.terminate();
    EXPECT_EQ(7, device.previous_time());
    EXPECT_EQ(8, device.current_time());
    EXPECT_EQ(TIME_MAX, device.next_time());
}

TEST(TimerTest, NodeScheduling) {
    combo2::net  network{common::make_tagged_tuple<>()};
    combo2::node device{network, common::make_tagged_tuple<uid>(1)};
    EXPECT_EQ(0, device.next_time());
    device.update();
    EXPECT_EQ(0, device.current_time());
    EXPECT_EQ(10, device.next_time());
    device.next_time(3);
    EXPECT_EQ(0, device.current_time());
    EXPECT_EQ(3, device.next_time());
    device.update();
    EXPECT_EQ(0, device.previous_time());
    EXPECT_EQ(3, device.current_time());
    EXPECT_EQ(13, device.next_time());
    device.frequency(2);
    EXPECT_EQ(0, device.previous_time());
    EXPECT_EQ(3, device.current_time());
    EXPECT_EQ(8, device.next_time());
    device.receive(5, 2, common::make_tagged_tuple<>());
    device.receive(7, 3, common::make_tagged_tuple<>());
    device.update();
    fcpp::field<bool> f = device.message_time() == details::make_field({1,2,3}, std::vector<times_t>{TIME_MIN,3,5,7});
    EXPECT_TRUE(f);
    EXPECT_EQ(3, device.previous_time());
    EXPECT_EQ(8, device.current_time());
    EXPECT_EQ(13, device.next_time());
    device.next_time(10);
    EXPECT_EQ(3, device.previous_time());
    EXPECT_EQ(8, device.current_time());
    EXPECT_EQ(10, device.next_time());
    device.update();
    EXPECT_EQ(8, device.previous_time());
    EXPECT_EQ(10, device.current_time());
    EXPECT_EQ(15, device.next_time());
    device.frequency(4);
    EXPECT_EQ(8, device.previous_time());
    EXPECT_EQ(10, device.current_time());
    EXPECT_EQ(12.5f, device.next_time());
    device.next_time(12);
    EXPECT_EQ(8, device.previous_time());
    EXPECT_EQ(10, device.current_time());
    EXPECT_EQ(12, device.next_time());
    device.update();
    EXPECT_EQ(10, device.previous_time());
    EXPECT_EQ(12, device.current_time());
    EXPECT_EQ(14.5f, device.next_time());
    device.terminate();
    EXPECT_EQ(10, device.previous_time());
    EXPECT_EQ(12, device.current_time());
    EXPECT_EQ(TIME_MAX, device.next_time());
}

TEST(TimerTest, NetScheduling) {
    combo2::net  network{common::make_tagged_tuple<>()};
    EXPECT_EQ(0, network.next());
    network.update();
    EXPECT_EQ(20, network.next());
    network.update();
    EXPECT_EQ(40, network.next());
    network.frequency(4);
    EXPECT_EQ(25, network.next());
    network.update();
    EXPECT_EQ(30, network.next());
    network.terminate();
    EXPECT_EQ(TIME_MAX, network.next());
}
