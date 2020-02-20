// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <random>

#include "gtest/gtest.h"

#include "lib/common/distribution.hpp"

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
    d = tester(random::make_distribution<std::uniform_real_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = tester(random::make_distribution<std::normal_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = tester(random::make_distribution<std::exponential_distribution>(5.0, 5.0), rnd);
    EXPECT_NEAR(50000.0, d, 1500.0);
    d = tester(random::make_distribution<std::weibull_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
}

TEST(DistributionTest, CRand) {
    random::crand rnd(42);
    double d;
    d = tester(random::make_distribution<std::uniform_real_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = tester(random::make_distribution<std::normal_distribution>(5.0, 1.0), rnd);
    EXPECT_NEAR(50000.0, d, 300.0);
    d = tester(random::make_distribution<std::exponential_distribution>(5.0, 5.0), rnd);
    EXPECT_NEAR(50000.0, d, 1500.0);
    d = tester(random::make_distribution<std::weibull_distribution>(5.0, 1.0), rnd);
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
    random::constant_distribution<int, 4> dint(rnd);
    int i;
    i = dint(rnd);
    EXPECT_EQ(4, i);
    i = dint(rnd);
    EXPECT_EQ(4, i);
    random::constant_distribution<double, 52, 10> ddouble(rnd);
    d = ddouble(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = ddouble(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
}

TEST(DistributionTest, Uniform) {
    std::mt19937 rnd(42);
    random::uniform_distribution<d5, d1> distr(rnd);
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
    random::uniform_d<double, 5, 1> dratio(rnd);
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
    random::uniform_distribution<d1, d5, meantag, devtag> dtup(rnd, fcpp::make_tagged_tuple<meantag,devtag>(5.0,1.0));
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

TEST(DistributionTest, Normal) {
    std::mt19937 rnd(42);
    random::normal_distribution<d5, d1> distr(rnd);
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
    random::normal_d<double, 5, 1> dratio(rnd);
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
    random::normal_d<double, 5, 3, 1, meantag, devtag> dtup(rnd, fcpp::make_tagged_tuple<devtag>(1.0));
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
    random::exponential_distribution<d5, d5> distr(rnd);
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
    random::exponential_d<double, 5, 5> dratio(rnd);
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
    random::exponential_distribution<d1, d1, meantag> dtup(rnd, fcpp::make_tagged_tuple<meantag>(5.0));
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
    random::weibull_distribution<d5, d1> distr(rnd);
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
    random::weibull_d<double, 5, 1> dratio(rnd);
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
    random::weibull_d<double, 3, 1, 1, meantag> dtag(rnd, fcpp::make_tagged_tuple<meantag,double>(5.0,'a'));
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
    random::make_positive<random::uniform_distribution<d1, d5>> distr(rnd);
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
    random::weibull_distribution<random::uniform_distribution<d5, d5, void, devtag>, random::uniform_distribution<d1, d5, void, devtag2>> distr(rnd, fcpp::make_tagged_tuple<devtag,devtag2>(0.0,0.0));
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
