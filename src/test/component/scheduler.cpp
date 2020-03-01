// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"
#include "lib/component/base.hpp"
#include "lib/component/randomizer.hpp"
#include "lib/component/scheduler.hpp"

using namespace fcpp;


struct meantag {};
struct devtag {};

using seq_mul = random::sequence_multiple<random::constant_distribution<times_t, 52, 10>, 3>;
using seq_per = random::sequence_periodic<random::constant_distribution<times_t, 15, 10>, random::uniform_d<times_t, 2, 10, 1, meantag, devtag>, random::constant_distribution<times_t, 62, 10>, random::constant_distribution<size_t, 5>>;

using combo1 = component::combine<component::scheduler<seq_mul>,component::randomizer<>>;
using combo2 = component::combine<component::scheduler<seq_per>,component::scheduler<seq_mul>,component::randomizer<>>;
using combo3 = component::combine<component::scheduler<seq_mul>>;


TEST(SchedulerTest, Single) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node device{network, common::make_tagged_tuple<component::tags::uid,seq_mul>(7,'b')};
    double d;
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_MAX, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SchedulerTest, Multiple) {
    combo2::net  network{common::make_tagged_tuple<>()};
    combo2::node device{network, common::make_tagged_tuple<component::tags::uid,devtag>(7,0.0)};
    double d;
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(1.5, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(3.5, d);
    d = device.next();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.5, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_MAX, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SchedulerTest, NoRandomizer) {
    combo1::net  network{common::make_tagged_tuple<>()};
    combo1::node device{network, common::make_tagged_tuple<component::tags::uid,seq_mul>(7,'b')};
    double d;
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_MAX, d);
    d = device.next();
    device.update();
    EXPECT_EQ(TIME_MAX, d);
}
