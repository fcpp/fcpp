// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <limits>
#include <random>

#include "gtest/gtest.h"

#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"

using namespace fcpp;


TEST(SequenceTest, Never) {
    std::mt19937 rnd(42);
    double d;
    random::sequence_never e(rnd);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SequenceTest, MultipleSame) {
    std::mt19937 rnd(42);
    double d;
    random::sequence_multiple<random::constant_distribution<times_t, 52, 10>, 3> e(rnd);
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
    EXPECT_EQ(TIME_MAX, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(TIME_MAX, d);
    double f;
    random::sequence_multiple<random::uniform_d<times_t, 50, 10, 10>, 2> ee(rnd);
    d = ee(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    f = ee.next();
    EXPECT_DOUBLE_EQ(d, f);
    f = ee(rnd);
    EXPECT_DOUBLE_EQ(d, f);
    f = ee(rnd);
    EXPECT_EQ(TIME_MAX, f);
}

TEST(SequenceTest, MultipleDiff) {
    std::mt19937 rnd(42);
    double d;
    random::sequence_multiple<random::constant_distribution<times_t, 52, 10>, 3, false> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e.next();
    e.step(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(TIME_MAX, d);
    double f;
    random::sequence_multiple<random::uniform_d<times_t, 50, 10, 10>, 2, false> ee(rnd);
    d = ee(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    f = ee(rnd);
    EXPECT_NE(d, f);
    f = ee(rnd);
    EXPECT_EQ(TIME_MAX, f);
}

TEST(SequenceTest, List) {
    std::mt19937 rnd(42);
    double d;
    random::sequence_list<random::constant_distribution<times_t, 33, 10>, random::constant_distribution<times_t, 52, 10>, random::constant_distribution<times_t, 15, 10>> e(rnd);
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
    EXPECT_EQ(TIME_MAX, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SequenceTest, Periodic) {
    std::mt19937 rnd(42);
    double d;
    random::sequence_periodic<random::constant_distribution<times_t, 15, 10>, random::constant_distribution<times_t, 2>, random::constant_distribution<times_t, 62, 10>, random::constant_distribution<size_t, 5>> e(rnd);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(3.5, d);
    d = e.next();
    EXPECT_DOUBLE_EQ(5.5, d);
    d = e(rnd);
    EXPECT_DOUBLE_EQ(5.5, d);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(TIME_MAX, d);
    random::sequence_periodic<random::constant_distribution<times_t, 15, 10>, random::constant_distribution<times_t, 1>, random::constant_distribution<times_t, 62, 10>, random::constant_distribution<size_t, 3>> ee(rnd);
    d = ee.next();
    EXPECT_DOUBLE_EQ(1.5, d);
    d = ee(rnd);
    EXPECT_DOUBLE_EQ(1.5, d);
    d = ee(rnd);
    EXPECT_DOUBLE_EQ(2.5, d);
    d = ee(rnd);
    EXPECT_DOUBLE_EQ(3.5, d);
    d = ee(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = ee(rnd);
    EXPECT_EQ(TIME_MAX, d);
    random::sequence_periodic<random::constant_distribution<times_t, 15, 10>> ei(rnd);
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
