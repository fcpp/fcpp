// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include <cassert>
#include <cmath>

#include <random>
#include <tuple>
#include <unordered_set>

#include "gtest/gtest.h"

#include "lib/data/bloom.hpp"

using namespace fcpp;

std::mt19937_64 gen(42);
std::uniform_int_distribution<size_t> rnd;

template <size_t m, size_t b>
std::tuple<bloom_filter<m,b>, std::unordered_set<size_t>> make(size_t n) {
    bloom_filter<m,b> c;
    std::unordered_set<size_t> s;
    for (; n>0; --n) {
        size_t x = rnd(gen);
        c.insert(x);
        s.insert(x);
    }
    return {c, s};
}

template <size_t m, size_t b>
std::tuple<double,double,double> stats(size_t n, size_t k, size_t t) {
    double sum = 0, sqsum = 0;
    for (size_t i=0; i<k; ++i) {
        bloom_filter<m,b> f;
        std::unordered_set<size_t> s;
        std::tie(f, s) = make<m,b>(n);
        size_t err = 0;
        for (size_t j=0; j<t; ++j) {
            size_t x = rnd(gen);
            while (s.count(x) > 0) x = rnd(gen);
            if (f.count(x) > 0) ++err;
        }
        double e = err * 1.0 / t;
        sum += e;
        sqsum += e*e;
    }
    sum /= k;
    sqsum = (sqsum - k*sum*sum) / (k-1);
    return {sum, sqrt(sqsum), bloom_error(m, b, n)};
}


TEST(BloomTest, Math) {
    for (int i=1; i<10; ++i)
        EXPECT_DOUBLE_EQ(details::exp(i), exp(i));
    for (int i=1; i<1000; ++i) {
        EXPECT_DOUBLE_EQ(details::log(i), log(i));
        EXPECT_DOUBLE_EQ(details::log(1.0/i), log(1.0/i));
    }
    EXPECT_EQ(required_bloom_bits(0.1, 500), 2396);
    constexpr size_t m = optimal_bloom_hashes(2048, 500);
    EXPECT_EQ(m, 3);
    EXPECT_DOUBLE_EQ(bloom_error(4, 2048, 25),   0.0000051624412052370923);
    EXPECT_DOUBLE_EQ(bloom_error(4, 2048, 50),   0.000075001697659215198);
    EXPECT_DOUBLE_EQ(bloom_error(4, 2048, 100),  0.00099178671312478354);
    EXPECT_DOUBLE_EQ(bloom_error(4, 2048, 200),  0.010942711920210209);
    EXPECT_DOUBLE_EQ(bloom_error(4, 2048, 400),  0.086459158287382168);
    EXPECT_DOUBLE_EQ(bloom_error(4, 2048, 600),  0.22706975426473885);
    EXPECT_DOUBLE_EQ(bloom_error(4, 2048, 800),  0.39042575333011748);
    EXPECT_DOUBLE_EQ(bloom_error(4, 2048, 1000), 0.542537672430774);
}

TEST(BloomTest, Operators) {
    bloom_filter<4,2048> x{1, 3, 13, 17, 42};
    bloom_filter<4,2048> y(x);
    bloom_filter<4,2048> z = std::move(x);
    EXPECT_EQ(z, y);
    x = {};
    EXPECT_TRUE(x.empty());
    x.insert(1);
    x.insert(3);
    x.insert(13);
    x.insert(17);
    x.insert(42);
    EXPECT_EQ(x, z);
}

TEST(BloomTest, CountError) {
    double m, d, e;
    for (size_t k=25; k<1000; k*=2) {
        std::tie(m, d, e) = stats<4,2048>(k, 25000/k, 100);
        EXPECT_NEAR(m, e, d + 0.00001);
        //std::cerr << k << ": " << m << " " << d << " " << e << std::endl;
    }
    for (size_t k=5; k<200; k*=2) {
        std::tie(m, d, e) = stats<5,512>(k, 25000/k, 100);
        EXPECT_NEAR(m, e, d + 0.00001);
        //std::cerr << k << ": " << m << " " << d << " " << e << std::endl;
    }
}

TEST(BloomTest, Clear) {
    bloom_filter<4,2048> x;
    EXPECT_TRUE(x.empty());
    x.insert(1);
    EXPECT_FALSE(x.empty());
    x.clear();
    EXPECT_TRUE(x.empty());
}
