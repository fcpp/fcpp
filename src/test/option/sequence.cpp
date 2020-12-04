// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <limits>
#include <random>

#include "gtest/gtest.h"

#include "lib/option/sequence.hpp"

using namespace fcpp;


TEST(SequenceTest, Never) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::never e(rnd);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SequenceTest, MultipleSame) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::multiple_n<3, 52, 10> e(rnd);
    d = e(rnd);
    EXPECT_NEAR(5.2, d, 1e-6);
    d = e.next();
    EXPECT_NEAR(5.2, d, 1e-6);
    d = e(rnd);
    EXPECT_NEAR(5.2, d, 1e-6);
    d = e.next();
    e.step(rnd);
    EXPECT_NEAR(5.2, d, 1e-6);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(TIME_MAX, d);
    times_t f;
    sequence::multiple<distribution::constant_n<size_t, 2>, distribution::uniform_n<times_t, 50, 10, 10>> ee(rnd);
    d = ee(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    f = ee.next();
    EXPECT_NEAR(d, f, 1e-6);
    f = ee(rnd);
    EXPECT_NEAR(d, f, 1e-6);
    f = ee(rnd);
    EXPECT_EQ(TIME_MAX, f);
}

TEST(SequenceTest, MultipleDiff) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::multiple<distribution::constant_n<size_t, 3>, distribution::constant_n<times_t, 52, 10>, false> e(rnd);
    d = e(rnd);
    EXPECT_NEAR(5.2, d, 1e-6);
    d = e.next();
    e.step(rnd);
    EXPECT_NEAR(5.2, d, 1e-6);
    d = e(rnd);
    EXPECT_NEAR(5.2, d, 1e-6);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(TIME_MAX, d);
    sequence::multiple<distribution::constant_n<size_t, 10>, distribution::interval_n<times_t, 0, 1>, false> ee(rnd);
    std::vector<times_t> v;
    v.push_back(ee(rnd));
    EXPECT_LT(0, v[0]);
    while (ee.next() < TIME_MAX) {
        v.push_back(ee(rnd));
        EXPECT_LT(v[v.size()-2], v[v.size()-1]);
    }
    EXPECT_LT(v.back(), 1);
    EXPECT_EQ(v.size(), 10ULL);
}

TEST(SequenceTest, List) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::list_n<10, 33, 52, 15> e(rnd);
    d = e(rnd);
    EXPECT_NEAR(1.5, d, 1e-6);
    d = e.next();
    EXPECT_NEAR(3.3, d, 1e-6);
    d = e(rnd);
    EXPECT_NEAR(3.3, d, 1e-6);
    d = e.next();
    e.step(rnd);
    EXPECT_NEAR(5.2, d, 1e-6);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SequenceTest, Periodic) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::periodic_n<10, 15, 20, 62, 5> e(rnd);
    d = e(rnd);
    EXPECT_NEAR(1.5, d, 1e-6);
    d = e(rnd);
    EXPECT_NEAR(3.5, d, 1e-6);
    d = e.next();
    EXPECT_NEAR(5.5, d, 1e-6);
    d = e(rnd);
    EXPECT_NEAR(5.5, d, 1e-6);
    d = e(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = e.next();
    e.step(rnd);
    EXPECT_EQ(TIME_MAX, d);
    sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::constant_n<times_t, 1>, distribution::constant_n<times_t, 62, 10>, distribution::constant_n<size_t, 3>> ee(rnd);
    d = ee.next();
    EXPECT_NEAR(1.5, d, 1e-6);
    d = ee(rnd);
    EXPECT_NEAR(1.5, d, 1e-6);
    d = ee(rnd);
    EXPECT_NEAR(2.5, d, 1e-6);
    d = ee(rnd);
    EXPECT_NEAR(3.5, d, 1e-6);
    d = ee(rnd);
    EXPECT_EQ(TIME_MAX, d);
    d = ee(rnd);
    EXPECT_EQ(TIME_MAX, d);
    sequence::periodic<distribution::constant_n<times_t, 15, 10>> ei(rnd);
    d = ei(rnd);
    EXPECT_NEAR(1.5, d, 1e-6);
    d = ei(rnd);
    EXPECT_NEAR(3.0, d, 1e-6);
    d = ei(rnd);
    EXPECT_NEAR(4.5, d, 1e-6);
    d = ei.next();
    EXPECT_NEAR(6.0, d, 1e-6);
    d = ei(rnd);
    EXPECT_NEAR(6.0, d, 1e-6);
}

TEST(SequenceTest, Merge) {
    std::mt19937 rnd(42);
    {
        sequence::merge<
            sequence::multiple_n<3, 52, 10>,
            sequence::never,
            sequence::list_n<10, 73, 52, 15>
        > e(rnd, common::make_tagged_tuple<char>(10));
        EXPECT_NEAR(1.5, e(rnd), 1e-6);
        EXPECT_NEAR(5.2, e(rnd), 1e-6);
        EXPECT_NEAR(5.2, e(rnd), 1e-6);
        EXPECT_NEAR(5.2, e(rnd), 1e-6);
        EXPECT_NEAR(5.2, e(rnd), 1e-6);
        EXPECT_NEAR(7.3, e(rnd), 1e-6);
        EXPECT_EQ(TIME_MAX, e(rnd));
    }
    {
        sequence::merge<sequence::multiple_n<3, 52, 10>> e(rnd);
        EXPECT_NEAR(5.2, e(rnd), 1e-6);
        EXPECT_NEAR(5.2, e(rnd), 1e-6);
        EXPECT_NEAR(5.2, e(rnd), 1e-6);
        EXPECT_EQ(TIME_MAX, e(rnd));
    }
    {
        sequence::merge<> e(rnd);
        EXPECT_EQ(TIME_MAX, e(rnd));
    }
}
