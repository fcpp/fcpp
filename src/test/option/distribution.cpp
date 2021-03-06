// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <random>

#include "gtest/gtest.h"

#include "lib/option/distribution.hpp"
#include "test/helper.hpp"

using namespace fcpp;

struct meantag {};
struct devtag {};
struct devtag2 {};

CONSTANT_DISTRIBUTION(d5, double, 5.0);
CONSTANT_DISTRIBUTION(d1, double, 1.0);

template<typename D, typename G>
double tester(D distr, G& gen) {
    double d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(gen);
    return d;
}


TEST(DistributionTest, Maker) {
    std::mt19937 rnd(42);
    double d;
    d = tester(distribution::make<std::uniform_real_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = tester(distribution::make<std::normal_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = tester(distribution::make<std::exponential_distribution>(5.0, 5.0), rnd);
    EXPECT_NEAR(50000.0, d, 1500.0);
    d = tester(distribution::make<std::weibull_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, CRand) {
    crand rnd(42);
    double d;
    d = tester(distribution::make<std::uniform_real_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = tester(distribution::make<std::normal_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = tester(distribution::make<std::exponential_distribution>(5.0, 5.0), rnd);
    EXPECT_NEAR(50000.0, d, 1500.0);
    d = tester(distribution::make<std::weibull_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Constant) {
    std::mt19937 rnd(42);
    d5 distr(rnd);
    double d;
    d = distr(rnd);
    EXPECT_DOUBLE_EQ(5.0, d);
    d = distr(rnd);
    EXPECT_DOUBLE_EQ(5.0, d);
    distribution::constant_n<int, 4> dint(rnd);
    int i;
    i = dint(rnd);
    EXPECT_EQ(4, i);
    i = dint(rnd);
    EXPECT_EQ(4, i);
    distribution::constant_n<double, 52, 10> ddouble(rnd);
    d = ddouble(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = ddouble(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
}

TEST(DistributionTest, Uniform) {
    std::mt19937 rnd(42);
    distribution::uniform<d5, d1> distr(rnd);
    double d;
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::uniform_n<double, 5, 1> dratio(rnd);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dratio(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::uniform<d1, d5, meantag, devtag> dtup(rnd, common::make_tagged_tuple<meantag,devtag>(5.0,1.0));
    d = dtup(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = dtup(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = dtup(rnd);
    EXPECT_NEAR(5.0, d, 1.74);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dtup(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Interval) {
    std::mt19937 rnd(42);
    distribution::interval_n<double, 1, 5> distr{rnd};
    double d, acc = 0;
    for (int i=0; i<10000; ++i) {
        d = distr(rnd);
        EXPECT_NEAR(3.0, d, 2.0);
        acc += d;
    }
    EXPECT_NEAR(30000, acc, 350);
}

TEST(DistributionTest, Normal) {
    std::mt19937 rnd(42);
    distribution::normal<d5, d1> distr(rnd);
    double d;
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::normal_n<double, 5, 1> dratio(rnd);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dratio(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::normal_n<double, 5, 3, 1, meantag, devtag> dtup(rnd, common::make_tagged_tuple<devtag>(1.0));
    d = dtup(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = dtup(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = dtup(rnd);
    EXPECT_NEAR(5.0, d, 3.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dtup(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Exponential) {
    std::mt19937 rnd(42);
    distribution::exponential<d5> distr(rnd);
    double d;
    d = distr(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = distr(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = distr(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd);
    EXPECT_NEAR(50000.0, d, 1500.0);
    distribution::exponential_n<double, 5> dratio(rnd);
    d = dratio(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = dratio(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = dratio(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dratio(rnd);
    EXPECT_NEAR(50000.0, d, 1500.0);
    distribution::exponential<d1, meantag> dtup(rnd, common::make_tagged_tuple<meantag>(5.0));
    d = dtup(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = dtup(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = dtup(rnd);
    EXPECT_NEAR(10.0, d, 10.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dtup(rnd);
    EXPECT_NEAR(50000.0, d, 1500.0);
}

TEST(DistributionTest, Weibull) {
    std::mt19937 rnd(42);
    distribution::weibull<d5, d1> distr(rnd);
    double d;
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::weibull_n<double, 5, 1> dratio(rnd);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = dratio(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dratio(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::weibull_n<double, 3, 1, 1, meantag> dtag(rnd, common::make_tagged_tuple<meantag,double>(5.0,'a'));
    d = dtag(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = dtag(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = dtag(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dtag(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Positive) {
    std::mt19937 rnd(42);
    distribution::positive<distribution::uniform<d1, d5>> distr(rnd);
    double d;
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd);
    EXPECT_NEAR(48300.0, d, 800.0);
}

TEST(DistributionTest, Combined) {
    std::mt19937 rnd(42);
    distribution::weibull<distribution::uniform<d5, d5, void, devtag>, distribution::uniform<d1, d5, void, devtag2>> distr(rnd, common::make_tagged_tuple<devtag,devtag2>(0.0,0.0));
    double d;
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Point) {
    std::mt19937 rnd(42);
    distribution::point<distribution::uniform_n<double, 5, 1>, distribution::uniform_n<double, 1, 5>> distr{rnd};
    vec<2> res{0,0};
    for (int i=0; i<10000; ++i)
        res += distr(rnd);
    EXPECT_NEAR(50000, res[0], 300);
    EXPECT_NEAR(10000, res[1], 1500);
}

TEST(DistributionTest, Rect) {
    std::mt19937 rnd(42);
    distribution::rect_n<1, 0, 0, 6, 10> distr{rnd};
    vec<2> res{0,0};
    for (int i=0; i<10000; ++i) {
        vec<2> r = distr(rnd);
        EXPECT_LE(0, r[0]);
        EXPECT_LE(r[0], 6);
        EXPECT_LE(0, r[1]);
        EXPECT_LE(r[1], 10);
        res += r;
    }
    EXPECT_NEAR(30000, res[0], 500);
    EXPECT_NEAR(50000, res[1], 900);
}
