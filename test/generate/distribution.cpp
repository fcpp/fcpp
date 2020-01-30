// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <random>

#include "gtest/gtest.h"

#include "lib/generate/distribution.hpp"

CONSTANT_DISTRIBUTION(d5, double, 5.0);
CONSTANT_DISTRIBUTION(d1, double, 1.0);


TEST(DistributionTest, Constant) {
    std::mt19937 rnd(42);
    d5 distr(rnd);
    double d;
    d = distr(rnd);
    EXPECT_DOUBLE_EQ(5.0, d);
    d = distr(rnd);
    EXPECT_DOUBLE_EQ(5.0, d);
    fcpp::constant_distribution<int, 4> dint(rnd);
    int i;
    i = dint(rnd);
    EXPECT_EQ(4, i);
    i = dint(rnd);
    EXPECT_EQ(4, i);
    fcpp::constant_distribution<double, 52, 10> ddouble(rnd);
    d = ddouble(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
    d = ddouble(rnd);
    EXPECT_DOUBLE_EQ(5.2, d);
}

TEST(DistributionTest, Uniform) {
    std::mt19937 rnd(42);
    fcpp::uniform_distribution<d5, d1> distr(rnd);
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
    fcpp::uniform_d<double, 5, 1> dratio(rnd);
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
}

TEST(DistributionTest, Normal) {
    std::mt19937 rnd(42);
    fcpp::normal_distribution<d5, d1> distr(rnd);
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
    fcpp::normal_d<double, 5, 1> dratio(rnd);
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
}

TEST(DistributionTest, Exponential) {
    std::mt19937 rnd(42);
    fcpp::exponential_distribution<d5, d5> distr(rnd);
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
    fcpp::exponential_d<double, 5, 5> dratio(rnd);
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
}

TEST(DistributionTest, Weibull) {
    std::mt19937 rnd(42);
    fcpp::weibull_distribution<d5, d1> distr(rnd);
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
    fcpp::weibull_d<double, 5, 1> dratio(rnd);
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
}

TEST(DistributionTest, Positive) {
    std::mt19937 rnd(42);
    fcpp::make_positive<fcpp::uniform_distribution<d1, d5>> distr(rnd);
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
