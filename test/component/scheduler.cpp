// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/sequence.hpp"
#include "lib/component/base.hpp"
#include "lib/component/randomizer.hpp"
#include "lib/component/scheduler.hpp"

const fcpp::times_t inf = fcpp::TIME_MAX;

struct meantag {};
struct devtag {};

using seq_mul = fcpp::sequence_multiple<fcpp::constant_distribution<fcpp::times_t, 52, 10>, 3>;
using seq_per = fcpp::sequence_periodic<fcpp::constant_distribution<fcpp::times_t, 15, 10>, fcpp::uniform_d<fcpp::times_t, 2, 10, 1, meantag, devtag>, fcpp::constant_distribution<fcpp::times_t, 62, 10>, fcpp::constant_distribution<size_t, 5>>;

using combo1 = fcpp::combine<fcpp::scheduler<seq_mul>,fcpp::randomizer<>>;
using combo2 = fcpp::combine<fcpp::scheduler<seq_per>,fcpp::scheduler<seq_mul>,fcpp::randomizer<>>;


TEST(ComponentTest, Single) {
    combo1::net  network{fcpp::make_tagged_tuple<>()};
    combo1::node device{network, fcpp::make_tagged_tuple<tags::id,seq_mul>(7,'b')};
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
    EXPECT_EQ(inf, d);
    d = device.next();
    device.update();
    EXPECT_EQ(inf, d);
}

TEST(ComponentTest, Multiple) {
    combo2::net  network{fcpp::make_tagged_tuple<>()};
    combo2::node device{network, fcpp::make_tagged_tuple<tags::id,devtag>(7,0.0)};
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
    EXPECT_EQ(inf, d);
    d = device.next();
    device.update();
    EXPECT_EQ(inf, d);
}
