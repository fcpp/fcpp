// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <limits>
#include <random>

#include "gtest/gtest.h"

#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"

const fcpp::times_t inf = fcpp::TIME_MAX;


TEST(SequenceTest, Never) {
    std::mt19937 rnd(42);
    double d;
    fcpp::sequence_never e(rnd);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
}

TEST(SequenceTest, MultipleSame) {
    std::mt19937 rnd(42);
    double d;
    fcpp::sequence_multiple<fcpp::constant_distribution<fcpp::times_t, 52, 10>, 3> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e.next();
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e.next();
    e.step(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(inf, d);
    double f;
    fcpp::sequence_multiple<fcpp::uniform_d<fcpp::times_t, 50, 10, 10>, 2> ee(rnd);
    d = ee(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    f = ee.next();
    EXPECT_DOUBLE_EQ(d, f);
    f = ee(rnd);
    EXPECT_DOUBLE_EQ(d, f);
    f = ee(rnd);
    EXPECT_EQ(inf, f);
}

TEST(SequenceTest, MultipleDiff) {
    std::mt19937 rnd(42);
    double d;
    fcpp::sequence_multiple<fcpp::constant_distribution<fcpp::times_t, 52, 10>, 3, false> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e.next();
    e.step(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(inf, d);
    double f;
    fcpp::sequence_multiple<fcpp::uniform_d<fcpp::times_t, 50, 10, 10>, 2, false> ee(rnd);
    d = ee(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    f = ee(rnd);
    EXPECT_NE(d, f);
    f = ee(rnd);
    EXPECT_EQ(inf, f);
}

TEST(SequenceTest, List) {
    std::mt19937 rnd(42);
    double d;
    fcpp::sequence_list<fcpp::constant_distribution<fcpp::times_t, 33, 10>, fcpp::constant_distribution<fcpp::times_t, 52, 10>, fcpp::constant_distribution<fcpp::times_t, 15, 10>> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = e.next();
    EXPECT_DOUBLE_EQ(3.3, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(3.3, d);
    d = e.next();
    e.step(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(inf, d);
}

TEST(SequenceTest, Periodic) {
    std::mt19937 rnd(42);
    double d;
    fcpp::sequence_periodic<fcpp::constant_distribution<fcpp::times_t, 15, 10>, fcpp::constant_distribution<fcpp::times_t, 2>, fcpp::constant_distribution<fcpp::times_t, 62, 10>, fcpp::constant_distribution<size_t, 5>> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(3.5, d);
    d = e.next();
    EXPECT_DOUBLE_EQ(5.5, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.5, d);
    d = e(rnd);
    EXPECT_EQ(inf, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(inf, d);
    fcpp::sequence_periodic<fcpp::constant_distribution<fcpp::times_t, 15, 10>, fcpp::constant_distribution<fcpp::times_t, 1>, fcpp::constant_distribution<fcpp::times_t, 62, 10>, fcpp::constant_distribution<size_t, 3>> ee(rnd);
    d = ee.next();
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
    fcpp::sequence_periodic<fcpp::constant_distribution<fcpp::times_t, 15, 10>> ei(rnd);
    d = ei(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = ei(rnd);
    EXPECT_DOUBLE_EQ(3.0, d);
    d = ei(rnd);
    EXPECT_DOUBLE_EQ(4.5, d);
    d = ei.next();
    EXPECT_DOUBLE_EQ(6.0, d);
    d = ei(rnd);
    EXPECT_DOUBLE_EQ(6.0, d);
}
