// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <random>

#include "gtest/gtest.h"

#include "lib/data/color.hpp"
#include "lib/option/distribution.hpp"
#include "test/helper.hpp"

using namespace fcpp;

common::tagged_tuple_t<> nothing{};

struct meantag {};
struct devtag {};
struct devtag2 {};

CONSTANT_DISTRIBUTION(d5, double, 5.0);
CONSTANT_DISTRIBUTION(d1, double, 1.0);

template<typename D, typename... Ts>
double tester(D distr, Ts&&... xs) {
    double d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(std::forward<Ts>(xs)...);
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
    d5 distr(rnd, nothing);
    double d;
    d = distr(rnd, nothing);
    EXPECT_DOUBLE_EQ(5.0, d);
    d = distr(rnd, nothing);
    EXPECT_DOUBLE_EQ(5.0, d);
    distribution::constant_n<int, 4> dint(rnd, nothing);
    int i;
    i = dint(rnd, nothing);
    EXPECT_EQ(4, i);
    i = dint(rnd, nothing);
    EXPECT_EQ(4, i);
    distribution::constant_n<double, 52, 10> ddouble(rnd, nothing);
    d = ddouble(rnd, nothing);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = ddouble(rnd, nothing);
    EXPECT_DOUBLE_EQ(5.2, d);
    color c;
    distribution::constant_n<color, TEAL> dcolor(rnd, nothing);
    c = dcolor(rnd, nothing);
    EXPECT_EQ(c, color(TEAL));
    distribution::constant_n<color, TEAL, 1, meantag> dcol1(rnd, nothing);
    c = dcol1(rnd, nothing);
    EXPECT_EQ(c, color(TEAL));
    distribution::constant_n<color, TEAL, 1, meantag> dcol2(rnd, common::make_tagged_tuple<meantag>(TAN));
    c = dcol2(rnd, nothing);
    EXPECT_EQ(c, color(TAN));
}

TEST(DistributionTest, Variable) {
    std::mt19937 rnd(42);
    distribution::variable_i<int, meantag> d(rnd, nothing);
    int i;
    i = d(rnd, nothing);
    EXPECT_EQ(0, i);
    i = d(rnd, common::make_tagged_tuple<meantag>(42));
    EXPECT_EQ(42, i);
    i = d(rnd, common::make_tagged_tuple<meantag>(7));
    EXPECT_EQ(7, i);
}

TEST(DistributionTest, Uniform) {
    std::mt19937 rnd(42);
    distribution::uniform<d5, d1> distr(rnd, nothing);
    double d;
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::uniform_n<double, 5, 1> dratio(rnd, nothing);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dratio(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::uniform<d1, d5, meantag, devtag> dtup(rnd, common::make_tagged_tuple<meantag,devtag>(5.0,1.0));
    d = dtup(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = dtup(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = dtup(rnd, nothing);
    EXPECT_NEAR(5.0, d, 1.74);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dtup(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Interval) {
    std::mt19937 rnd(42);
    distribution::interval_n<double, 1, 5> distr(rnd, nothing);
    double d, acc = 0;
    for (int i=0; i<10000; ++i) {
        d = distr(rnd, nothing);
        EXPECT_NEAR(3.0, d, 2.0);
        acc += d;
    }
    EXPECT_NEAR(30000, acc, 350);
}

TEST(DistributionTest, Normal) {
    std::mt19937 rnd(42);
    distribution::normal<d5, d1> distr(rnd, nothing);
    double d;
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::normal_n<double, 5, 1> dratio(rnd, nothing);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dratio(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::normal_n<double, 5, 3, 1, meantag, devtag> dtup(rnd, common::make_tagged_tuple<devtag>(1.0));
    d = dtup(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = dtup(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = dtup(rnd, nothing);
    EXPECT_NEAR(5.0, d, 3.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dtup(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Exponential) {
    std::mt19937 rnd(42);
    distribution::exponential<d5> distr(rnd, nothing);
    double d;
    d = distr(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 1500.0);
    distribution::exponential_n<double, 5> dratio(rnd, nothing);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dratio(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 1500.0);
    distribution::exponential<d1, meantag> dtup(rnd, common::make_tagged_tuple<meantag>(5.0));
    d = dtup(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = dtup(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = dtup(rnd, nothing);
    EXPECT_NEAR(10.0, d, 10.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dtup(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 1500.0);
}

TEST(DistributionTest, Weibull) {
    std::mt19937 rnd(42);
    distribution::weibull<d5, d1> distr(rnd, nothing);
    double d;
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::weibull_n<double, 5, 1> dratio(rnd, nothing);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = dratio(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dratio(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
    distribution::weibull_n<double, 3, 1, 1, meantag> dtag(rnd, common::make_tagged_tuple<meantag,double>(5.0,'a'));
    d = dtag(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = dtag(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = dtag(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += dtag(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Positive) {
    std::mt19937 rnd(42);
    distribution::positive<distribution::uniform<d1, d5>> distr(rnd, nothing);
    double d;
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd, nothing);
    EXPECT_NEAR(48300.0, d, 800.0);
}

TEST(DistributionTest, Combined) {
    std::mt19937 rnd(42);
    distribution::weibull<distribution::uniform<d5, d5, void, devtag>, distribution::uniform<d1, d5, void, devtag2>> distr(rnd, common::make_tagged_tuple<devtag,devtag2>(0.0,0.0));
    double d;
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = distr(rnd, nothing);
    EXPECT_NEAR(5.0, d, 5.0);
    d = 0;
    for (int i=0; i<10000; ++i)
        d += distr(rnd, nothing);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, Point) {
    std::mt19937 rnd(42);
    distribution::point<distribution::uniform_n<double, 5, 1>, distribution::uniform_n<double, 1, 5>> distr(rnd, nothing);
    vec<2> res{0,0};
    for (int i=0; i<10000; ++i)
        res += distr(rnd, nothing);
    EXPECT_NEAR(50000, res[0], 300);
    EXPECT_NEAR(10000, res[1], 1500);
}

TEST(DistributionTest, Rect) {
    std::mt19937 rnd(42);
    distribution::rect_n<1, 0, 0, 6, 10> distr(rnd, nothing);
    vec<2> res{0,0};
    for (int i=0; i<10000; ++i) {
        vec<2> r = distr(rnd, nothing);
        EXPECT_LE(0, r[0]);
        EXPECT_LE(r[0], 6);
        EXPECT_LE(0, r[1]);
        EXPECT_LE(r[1], 10);
        res += r;
    }
    EXPECT_NEAR(30000, res[0], 500);
    EXPECT_NEAR(50000, res[1], 900);
}
