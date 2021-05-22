// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/randomizer.hpp"
#include "lib/component/scheduler.hpp"

using namespace fcpp;
using namespace component::tags;


struct meantag {};
struct devtag {};

using seq_mul = sequence::multiple_n<3, 52, 10>;
using seq_per = sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::uniform_n<times_t, 2, 10, 1, meantag, devtag>, distribution::constant_n<times_t, 62, 10>, distribution::constant_n<size_t, 5>>;

using combo1 = component::combine_spec<
    component::scheduler<round_schedule<seq_mul>>,
    component::randomizer<>,
    component::base<>
>;
using combo2 = component::combine_spec<
    component::scheduler<round_schedule<seq_per>,round_schedule<seq_mul>>,
    component::randomizer<>,
    component::base<>
>;
using combo3 = component::combine_spec<component::scheduler<round_schedule<seq_mul>>,component::base<>>;
using combo4 = component::combine_spec<
    component::scheduler<round_schedule<seq_per,seq_mul>>,
    component::randomizer<>,
    component::base<>
>;


TEST(SchedulerTest, Single) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node device{network, common::make_tagged_tuple<uid,seq_mul>(7,'b')};
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
    EXPECT_EQ(TIME_MAX, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SchedulerTest, Multiple) {
    {
        combo2::net  network{common::make_tagged_tuple<>()};
        combo2::node device{network, common::make_tagged_tuple<uid,devtag>(7,0)};
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
        EXPECT_EQ(TIME_MAX, d);
        d = device.next();
        device.update();
        EXPECT_EQ(TIME_MAX, d);
    }
    {
        combo4::net  network{common::make_tagged_tuple<>()};
        combo4::node device{network, common::make_tagged_tuple<uid,devtag>(7,0)};
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
        EXPECT_EQ(TIME_MAX, d);
        d = device.next();
        device.update();
        EXPECT_EQ(TIME_MAX, d);
    }
}

TEST(SchedulerTest, NoRandomizer) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node device{network, common::make_tagged_tuple<uid,seq_mul>(7,'b')};
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
    EXPECT_EQ(TIME_MAX, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_MAX, d);
}
