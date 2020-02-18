// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/timer.hpp"

using fcpp::times_t;

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

using combo1 = fcpp::combine<exposer<true>,fcpp::timer>;
using combo2 = fcpp::combine<exposer<false>,fcpp::timer,scheduler>;


TEST(ComponentTest, NodePlanning) {
    combo1::net  network{fcpp::make_tagged_tuple<>()};
    combo1::node device{network, fcpp::make_tagged_tuple<tags::id,tags::start_time>(1,2.0)};
    EXPECT_EQ(2.0, device.next_time());
    device.update();
    EXPECT_EQ(2.0, device.current_time());
    EXPECT_EQ(7.0, device.next_time());
    device.receive(3.0, 2, fcpp::make_tagged_tuple<>());
    device.receive(4.0, 3, fcpp::make_tagged_tuple<>());
    device.update();
    EXPECT_EQ(2.0, device.previous_time());
    EXPECT_EQ(7.0, device.current_time());
    EXPECT_EQ(12.0, device.next_time());
    fcpp::field<bool> f = device.message_time() == fcpp::details::make_field(fcpp::TIME_MIN, {{1,2},{2,3},{3,4}});
    EXPECT_TRUE(f);
    device.next_time(8.0);
    EXPECT_EQ(8.0, device.next_time());
    device.update();
    EXPECT_EQ(7.0, device.previous_time());
    EXPECT_EQ(8.0, device.current_time());
    EXPECT_EQ(13.0, device.next_time());
    device.terminate();
    EXPECT_EQ(7.0, device.previous_time());
    EXPECT_EQ(8.0, device.current_time());
    EXPECT_EQ(fcpp::TIME_MAX, device.next_time());
}

TEST(ComponentTest, NodeScheduling) {
    combo2::net  network{fcpp::make_tagged_tuple<>()};
    combo2::node device{network, fcpp::make_tagged_tuple<tags::id>(1)};
    EXPECT_EQ(0.0, device.next_time());
    device.update();
    EXPECT_EQ(0.0, device.current_time());
    EXPECT_EQ(10.0, device.next_time());
    device.next_time(3.0);
    EXPECT_EQ(0.0, device.current_time());
    EXPECT_EQ(3.0, device.next_time());
    device.update();
    EXPECT_EQ(0.0, device.previous_time());
    EXPECT_EQ(3.0, device.current_time());
    EXPECT_EQ(13.0, device.next_time());
    device.frequency(2.0);
    EXPECT_EQ(0.0, device.previous_time());
    EXPECT_EQ(3.0, device.current_time());
    EXPECT_EQ(8.0, device.next_time());
    device.receive(5.0, 2, fcpp::make_tagged_tuple<>());
    device.receive(7.0, 3, fcpp::make_tagged_tuple<>());
    device.update();
    fcpp::field<bool> f = device.message_time() == fcpp::details::make_field(fcpp::TIME_MIN, {{1,3},{2,5},{3,7}});
    EXPECT_TRUE(f);
    EXPECT_EQ(3.0, device.previous_time());
    EXPECT_EQ(8.0, device.current_time());
    EXPECT_EQ(13.0, device.next_time());
    device.next_time(10.0);
    EXPECT_EQ(3.0, device.previous_time());
    EXPECT_EQ(8.0, device.current_time());
    EXPECT_EQ(10.0, device.next_time());
    device.update();
    EXPECT_EQ(8.0, device.previous_time());
    EXPECT_EQ(10.0, device.current_time());
    EXPECT_EQ(15.0, device.next_time());
    device.frequency(4.0);
    EXPECT_EQ(8.0, device.previous_time());
    EXPECT_EQ(10.0, device.current_time());
    EXPECT_EQ(12.5, device.next_time());
    device.next_time(12.0);
    EXPECT_EQ(8.0, device.previous_time());
    EXPECT_EQ(10.0, device.current_time());
    EXPECT_EQ(12.0, device.next_time());
    device.update();
    EXPECT_EQ(10.0, device.previous_time());
    EXPECT_EQ(12.0, device.current_time());
    EXPECT_EQ(14.5, device.next_time());
    device.terminate();
    EXPECT_EQ(10.0, device.previous_time());
    EXPECT_EQ(12.0, device.current_time());
    EXPECT_EQ(fcpp::TIME_MAX, device.next_time());
}

TEST(ComponentTest, NetScheduling) {
    combo2::net  network{fcpp::make_tagged_tuple<>()};
    EXPECT_EQ(0.0, network.next());
    network.update();
    EXPECT_EQ(20.0, network.next());
    network.update();
    EXPECT_EQ(40.0, network.next());
    network.frequency(4.0, 20);
    EXPECT_EQ(25.0, network.next());
    network.update();
    EXPECT_EQ(30.0, network.next());
    network.terminate();
    EXPECT_EQ(fcpp::TIME_MAX, network.next());
}
