// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <limits>
#include <random>

#include "gtest/gtest.h"

#include "test/helper.hpp"
#include "lib/option/sequence.hpp"

using namespace fcpp;

common::tagged_tuple_t<> nothing{};


TEST(SequenceTest, Never) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::never e(rnd, nothing);
    EXPECT_TRUE(e.empty());
    d = e(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    EXPECT_TRUE(e.empty());
    d = e(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SequenceTest, MultipleSame) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::multiple_n<3, 52, 10> e(rnd, nothing);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(5.2, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e.next();
    EXPECT_NEAR(5.2, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(5.2, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e.next();
    e.step(rnd, nothing);
    EXPECT_NEAR(5.2, d, 1e-6);
    EXPECT_TRUE(e.empty());
    d = e(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    EXPECT_TRUE(e.empty());
    d = e.next();
    e.step(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    times_t f;
    sequence::multiple<distribution::constant_n<size_t, 2>, distribution::uniform_n<times_t, 50, 10, 10>> ee(rnd, nothing);
    EXPECT_FALSE(ee.empty());
    d = ee(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    EXPECT_FALSE(ee.empty());
    f = ee.next();
    EXPECT_NEAR(d, f, 1e-6);
    EXPECT_FALSE(ee.empty());
    f = ee(rnd, nothing);
    EXPECT_NEAR(d, f, 1e-6);
    EXPECT_TRUE(ee.empty());
    f = ee(rnd, nothing);
    EXPECT_EQ(TIME_MAX, f);
}

TEST(SequenceTest, MultipleDiff) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::multiple<distribution::constant_n<size_t, 3>, distribution::constant_n<times_t, 52, 10>, false> e(rnd, nothing);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(5.2, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e.next();
    e.step(rnd, nothing);
    EXPECT_NEAR(5.2, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(5.2, d, 1e-6);
    EXPECT_TRUE(e.empty());
    d = e(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    EXPECT_TRUE(e.empty());
    d = e.next();
    e.step(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    sequence::multiple<distribution::constant_n<size_t, 10>, distribution::interval_n<times_t, 0, 1>, false> ee(rnd, nothing);
    std::vector<times_t> v;
    v.push_back(ee(rnd, nothing));
    EXPECT_LT(0, v[0]);
    while (ee.next() < TIME_MAX) {
        EXPECT_FALSE(ee.empty());
        v.push_back(ee(rnd, nothing));
        EXPECT_LT(v[v.size()-2], v[v.size()-1]);
    }
    EXPECT_TRUE(ee.empty());
    EXPECT_LT(v.back(), 1);
    EXPECT_EQ(v.size(), 10ULL);
}

TEST(SequenceTest, List) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::list_n<10, 33, 52, 15> e(rnd, nothing);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(1.5, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e.next();
    EXPECT_NEAR(3.3, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(3.3, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e.next();
    e.step(rnd, nothing);
    EXPECT_NEAR(5.2, d, 1e-6);
    EXPECT_TRUE(e.empty());
    d = e(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    EXPECT_TRUE(e.empty());
    d = e.next();
    e.step(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
}

TEST(SequenceTest, Periodic) {
    std::mt19937 rnd(42);
    times_t d;
    sequence::periodic_n<10, 15, 20, 62, 5> e(rnd, nothing);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(1.5, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(3.5, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e.next();
    EXPECT_NEAR(5.5, d, 1e-6);
    EXPECT_FALSE(e.empty());
    d = e(rnd, nothing);
    EXPECT_NEAR(5.5, d, 1e-6);
    EXPECT_TRUE(e.empty());
    d = e(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    EXPECT_TRUE(e.empty());
    d = e.next();
    e.step(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::constant_n<times_t, 1>, distribution::constant_n<times_t, 62, 10>, distribution::constant_n<size_t, 3>> ee(rnd, nothing);
    EXPECT_FALSE(ee.empty());
    d = ee.next();
    EXPECT_NEAR(1.5, d, 1e-6);
    EXPECT_FALSE(ee.empty());
    d = ee(rnd, nothing);
    EXPECT_NEAR(1.5, d, 1e-6);
    EXPECT_FALSE(ee.empty());
    d = ee(rnd, nothing);
    EXPECT_NEAR(2.5, d, 1e-6);
    EXPECT_FALSE(ee.empty());
    d = ee(rnd, nothing);
    EXPECT_NEAR(3.5, d, 1e-6);
    EXPECT_TRUE(ee.empty());
    d = ee(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    EXPECT_TRUE(ee.empty());
    d = ee(rnd, nothing);
    EXPECT_EQ(TIME_MAX, d);
    sequence::periodic<distribution::constant_n<times_t, 15, 10>> ei(rnd, nothing);
    EXPECT_FALSE(ei.empty());
    d = ei(rnd, nothing);
    EXPECT_NEAR(1.5, d, 1e-6);
    EXPECT_FALSE(ei.empty());
    d = ei(rnd, nothing);
    EXPECT_NEAR(3.0, d, 1e-6);
    EXPECT_FALSE(ei.empty());
    d = ei(rnd, nothing);
    EXPECT_NEAR(4.5, d, 1e-6);
    EXPECT_FALSE(ei.empty());
    d = ei.next();
    EXPECT_NEAR(6.0, d, 1e-6);
    d = ei(rnd, nothing);
    EXPECT_NEAR(6.0, d, 1e-6);
    EXPECT_FALSE(ei.empty());
}

TEST(SequenceTest, Merge) {
    std::mt19937 rnd(42);
    {
        sequence::merge<
            sequence::multiple_n<3, 52, 10>,
            sequence::never,
            sequence::list_n<10, 73, 52, 15>
        > e(rnd, common::make_tagged_tuple<char>(10));
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(1.5, e(rnd, nothing), 1e-6);
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(5.2, e(rnd, nothing), 1e-6);
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(5.2, e(rnd, nothing), 1e-6);
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(5.2, e(rnd, nothing), 1e-6);
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(5.2, e(rnd, nothing), 1e-6);
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(7.3, e(rnd, nothing), 1e-6);
        EXPECT_TRUE(e.empty());
        EXPECT_EQ(TIME_MAX, e(rnd, nothing));
    }
    {
        sequence::merge<sequence::multiple_n<3, 52, 10>> e(rnd, nothing);
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(5.2, e(rnd, nothing), 1e-6);
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(5.2, e(rnd, nothing), 1e-6);
        EXPECT_FALSE(e.empty());
        EXPECT_NEAR(5.2, e(rnd, nothing), 1e-6);
        EXPECT_TRUE(e.empty());
        EXPECT_EQ(TIME_MAX, e(rnd, nothing));
    }
    {
        sequence::merge<> e(rnd, nothing);
        EXPECT_TRUE(e.empty());
        EXPECT_EQ(TIME_MAX, e(rnd, nothing));
    }
}

TEST(SequenceTest, Grid) {
    std::mt19937 rnd(42);
    {
        sequence::grid_n<10, 0, 0, 40, 25, 3, 6> e(rnd, nothing);
        for (int z=0; z<2; ++z)
            for (int y=0; y<6; ++y)
                for (int x=0; x<3; ++x) {
                    EXPECT_EQ(e.empty(), z > 0);
                    EXPECT_EQ(make_vec(2*x,0.5*y), e(rnd, nothing));
                }
        EXPECT_TRUE(e.empty());
    }
    {
        struct lo {};
        struct hi {};
        struct nm {};
        sequence::grid_i<lo, lo, lo, hi, hi, hi, nm, nm, nm> e(nullptr, common::make_tagged_tuple<lo,hi,nm>(0,2,3));
        for (int z=0; z<3; ++z)
            for (int y=0; y<3; ++y)
                for (int x=0; x<3; ++x) {
                    EXPECT_FALSE(e.empty());
                    EXPECT_EQ(make_vec(x,y,z), e(rnd, nothing));
                }
        EXPECT_TRUE(e.empty());
        EXPECT_EQ(make_vec(0,0,0), e(rnd, nothing));
        EXPECT_EQ(make_vec(1,0,0), e(rnd, nothing));
    }
}

TEST(SequenceTest, Circle) {
    std::mt19937 rnd(42);
    {
        sequence::circle_n<1, 0, 0, 0, 1, 1, 1, 4> e(rnd, nothing);
        std::vector<vec<3>> v;
        for (int i=0; i<4; ++i) {
            EXPECT_FALSE(e.empty());
            v.push_back(e(rnd, nothing));
            EXPECT_NEAR(norm(v[i]), norm(make_vec(1,1,1)), 1e-9);
            EXPECT_NEAR(v[i]*make_vec(1,1,1), 0, 1e-9);
        }
        EXPECT_TRUE(e.empty());
        for (int i=0; i<4; ++i) {
            EXPECT_NEAR(v[i]*v[(i+1)%4], 0, 1e-9);
            EXPECT_NEAR(v[i]*v[(i+2)%4], -3, 1e-9);
        }
    }
    {
        sequence::circle_n<1, 0, 0, 1, 4> e(rnd, nothing);
        std::vector<vec<2>> v;
        for (int i=0; i<4; ++i) {
            EXPECT_FALSE(e.empty());
            v.push_back(e(rnd, nothing));
            EXPECT_NEAR(norm(v[i]), 1, 1e-9);
        }
        EXPECT_TRUE(e.empty());
        for (int i=0; i<4; ++i) {
            EXPECT_NEAR(v[i]*v[(i+1)%4], 0, 1e-9);
            EXPECT_NEAR(v[i]*v[(i+2)%4], -1, 1e-9);
        }
    }
    struct ct {};
    struct rt {};
    struct nt {};
    {
        sequence::circle_i<ct, rt, nt, 3> e(nullptr, common::make_tagged_tuple<ct,rt,nt>(make_vec(0,0,0), make_vec(1,1,1), 4));
        std::vector<vec<3>> v;
        for (int i=0; i<4; ++i) {
            EXPECT_FALSE(e.empty());
            v.push_back(e(rnd, nothing));
            EXPECT_NEAR(norm(v[i]), norm(make_vec(1,1,1)), 1e-9);
            EXPECT_NEAR(v[i]*make_vec(1,1,1), 0, 1e-9);
        }
        EXPECT_TRUE(e.empty());
        for (int i=0; i<4; ++i) {
            EXPECT_NEAR(v[i]*v[(i+1)%4], 0, 1e-9);
            EXPECT_NEAR(v[i]*v[(i+2)%4], -3, 1e-9);
        }
    }
    {
        sequence::circle_i<ct, rt, nt, 2> e(nullptr, common::make_tagged_tuple<ct,rt,nt>(make_vec(0,0), make_vec(1), 4));
        std::vector<vec<2>> v;
        for (int i=0; i<4; ++i) {
            EXPECT_FALSE(e.empty());
            v.push_back(e(rnd, nothing));
            EXPECT_NEAR(norm(v[i]), 1, 1e-9);
        }
        EXPECT_TRUE(e.empty());
        for (int i=0; i<4; ++i) {
            EXPECT_NEAR(v[i]*v[(i+1)%4], 0, 1e-9);
            EXPECT_NEAR(v[i]*v[(i+2)%4], -1, 1e-9);
        }
    }
}
