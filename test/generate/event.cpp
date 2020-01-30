// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <limits>
#include <random>

#include "gtest/gtest.h"

#include "lib/generate/distribution.hpp"
#include "lib/generate/event.hpp"

const fcpp::times_t inf = std::numeric_limits<fcpp::times_t>::max();


TEST(EventTest, Never) {
    std::mt19937 rnd(42);
    double d;
    fcpp::event_never e(rnd);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
}

TEST(EventTest, MultipleSame) {
    std::mt19937 rnd(42);
    double d;
    fcpp::event_multiple<fcpp::constant_distribution<fcpp::times_t, 52, 10>, 3> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e.next(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    double f;
    fcpp::event_multiple<fcpp::uniform_d<fcpp::times_t, 50, 10, 10>, 2> ee(rnd);
    d = ee(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    f = ee.next(rnd);
    EXPECT_DOUBLE_EQ(d, f);
    f = ee(rnd);
    EXPECT_DOUBLE_EQ(d, f);
    f = ee(rnd);
    EXPECT_EQ(inf, f);
}

TEST(EventTest, MultipleDiff) {
    std::mt19937 rnd(42);
    double d;
    fcpp::event_multiple<fcpp::constant_distribution<fcpp::times_t, 52, 10>, 3, false> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    double f;
    fcpp::event_multiple<fcpp::uniform_d<fcpp::times_t, 50, 10, 10>, 2, false> ee(rnd);
    d = ee(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    f = ee(rnd);
    EXPECT_NE(d, f);
    f = ee(rnd);
    EXPECT_EQ(inf, f);
}

TEST(EventTest, Sequence) {
    std::mt19937 rnd(42);
    double d;
    fcpp::event_sequence<fcpp::constant_distribution<fcpp::times_t, 33, 10>, fcpp::constant_distribution<fcpp::times_t, 52, 10>, fcpp::constant_distribution<fcpp::times_t, 15, 10>> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = e.next(rnd);
    EXPECT_DOUBLE_EQ(3.3, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(3.3, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
}

TEST(EventTest, Periodic) {
    std::mt19937 rnd(42);
    double d;
    fcpp::event_periodic<fcpp::constant_distribution<fcpp::times_t, 15, 10>, fcpp::constant_distribution<fcpp::times_t, 2>, fcpp::constant_distribution<fcpp::times_t, 62, 10>, fcpp::constant_distribution<size_t, 5>> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(3.5, d);
    d = e.next(rnd);
    EXPECT_DOUBLE_EQ(5.5, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.5, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    fcpp::event_periodic<fcpp::constant_distribution<fcpp::times_t, 15, 10>, fcpp::constant_distribution<fcpp::times_t, 1>, fcpp::constant_distribution<fcpp::times_t, 62, 10>, fcpp::constant_distribution<size_t, 3>> ee(rnd);
    d = ee.next(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = ee(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = ee(rnd);
    EXPECT_DOUBLE_EQ(2.5, d);
    d = ee(rnd);
    EXPECT_DOUBLE_EQ(3.5, d);
    d = ee(rnd);
    EXPECT_EQ(inf, d);
    d = ee(rnd);
    EXPECT_EQ(inf, d);
    fcpp::event_periodic<fcpp::constant_distribution<fcpp::times_t, 15, 10>> ei(rnd);
    d = ei(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = ei(rnd);
    EXPECT_DOUBLE_EQ(3.0, d);
    d = ei(rnd);
    EXPECT_DOUBLE_EQ(4.5, d);
    d = ei.next(rnd);
    EXPECT_DOUBLE_EQ(6.0, d);
    d = ei(rnd);
    EXPECT_DOUBLE_EQ(6.0, d);
}
