// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/timer.hpp"
#include "lib/component/randomizer.hpp"

using namespace fcpp;
using namespace component::tags;


struct meantag {};
struct devtag {};


// Mock identifier component.
struct identifier {
    template <typename F, typename P>
    struct component : public P {
        DECLARE_COMPONENT(identifier);
        using node = typename P::node;
        struct net : public P::net {
            using P::net::net;

            void push_event(device_t, times_t) {}
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
        using net = typename P::net;
    };
};

using d3 = reactive_delay<distribution::constant_n<times_t, 3>>;
using s10 = round_schedule<sequence::periodic_n<1, 0, 10>>;

using combo1 = component::combine_spec<exposer<true>,component::timer<>,component::base<>>;
using combo2 = component::combine_spec<exposer<false>,component::timer<s10>,component::base<>>;
using combo3 = component::combine_spec<identifier,exposer<false>,component::timer<d3,s10>,component::base<>>;

using seq_mul = sequence::multiple_n<3, 52, 10>;
using seq_per = sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::uniform_n<times_t, 2, 10, 1, meantag, devtag>, distribution::constant_n<times_t, 62, 10>, distribution::constant_n<size_t, 5>>;

using scombo1 = component::combine_spec<
    component::timer<round_schedule<seq_mul>>,
    component::randomizer<>,
    component::base<>
>;
using scombo2 = component::combine_spec<
    component::timer<round_schedule<seq_per>,round_schedule<seq_mul>>,
    component::randomizer<>,
    component::base<>
>;
using scombo3 = component::combine_spec<component::timer<round_schedule<seq_mul>>,component::base<>>;
using scombo4 = component::combine_spec<
    component::timer<round_schedule<seq_per,seq_mul>>,
    component::randomizer<>,
    component::base<>
>;


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

TEST(TimerTest, NodeReacting) {
    combo3::net  network{common::make_tagged_tuple<>()};
    combo3::node device{network, common::make_tagged_tuple<uid>(1)};
    EXPECT_EQ(0, device.next_time());
    device.update();
    EXPECT_EQ(0, device.current_time());
    EXPECT_EQ(10, device.next_time());
    device.receive(3, 2, common::make_tagged_tuple<>());
    EXPECT_EQ(6, device.next_time());
    device.receive(4, 3, common::make_tagged_tuple<>());
    EXPECT_EQ(6, device.next_time());
    device.update();
    EXPECT_EQ(0, device.previous_time());
    EXPECT_EQ(6, device.current_time());
    EXPECT_EQ(16, device.next_time());
    device.update();
    EXPECT_EQ(6, device.previous_time());
    EXPECT_EQ(16, device.current_time());
    EXPECT_EQ(26, device.next_time());
    device.receive(17, 1, common::make_tagged_tuple<>());
    EXPECT_EQ(6, device.previous_time());
    EXPECT_EQ(16, device.current_time());
    EXPECT_EQ(20, device.next_time());
    device.update();
    EXPECT_EQ(16, device.previous_time());
    EXPECT_EQ(20, device.current_time());
    EXPECT_EQ(30, device.next_time());
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

TEST(TimerTest, SingleScheduler) {
    scombo1::net  network{common::make_tagged_tuple<>()};
    scombo1::node device{network, common::make_tagged_tuple<uid,seq_mul>(7,'b')};
    times_t d;
    d = device.next();
    device.update();
    EXPECT_NEAR(5.2f, d, 1e-6f);
    d = device.next();
    device.update();
    EXPECT_NEAR(5.2f, d, 1e-6f);
    d = device.next();
    device.update();
    EXPECT_NEAR(5.2f, d, 1e-6f);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_FAR, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_FAR, d);
}

TEST(TimerTest, MultipleScheduler) {
    {
        scombo2::net  network{common::make_tagged_tuple<>()};
        scombo2::node device{network, common::make_tagged_tuple<uid,devtag>(7,0)};
        times_t d;
        d = device.next();
        device.update();
        EXPECT_NEAR(1.5f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(3.5f, d, 1e-6f);
        d = device.next();
        EXPECT_NEAR(5.2f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(5.2f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(5.2f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(5.2f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(5.5f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_EQ(TIME_FAR, d);
        d = device.next();
        device.update();
        EXPECT_EQ(TIME_FAR, d);
    }
    {
        scombo4::net  network{common::make_tagged_tuple<>()};
        scombo4::node device{network, common::make_tagged_tuple<uid,devtag>(7,0)};
        times_t d;
        d = device.next();
        device.update();
        EXPECT_NEAR(1.5f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(3.5f, d, 1e-6f);
        d = device.next();
        EXPECT_NEAR(5.2f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(5.2f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(5.2f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(5.2f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_NEAR(5.5f, d, 1e-6f);
        d = device.next();
        device.update();
        EXPECT_EQ(TIME_FAR, d);
        d = device.next();
        device.update();
        EXPECT_EQ(TIME_FAR, d);
    }
}

TEST(TimerTest, NoRandomizer) {
    scombo1::net  network{common::make_tagged_tuple<>()};
    scombo1::node device{network, common::make_tagged_tuple<uid,seq_mul>(7,'b')};
    times_t d;
    d = device.next();
    device.update();
    EXPECT_NEAR(5.2f, d, 1e-6f);
    d = device.next();
    device.update();
    EXPECT_NEAR(5.2f, d, 1e-6f);
    d = device.next();
    device.update();
    EXPECT_NEAR(5.2f, d, 1e-6f);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_FAR, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_FAR, d);
}
