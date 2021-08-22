// Copyright © 2021 Giorgio Audrito and Gianluca Torta. All Rights Reserved.

#include <cassert>
#include <cmath>

#include <random>
#include <tuple>

#include "gtest/gtest.h"

#include "lib/data/hyperloglog.hpp"

using namespace fcpp;

template <size_t m, size_t bits = 4, size_t seed = 0, typename T = size_t, typename H = std::hash<T>>
struct exposed_hll : public hyperloglog_counter<m,bits,seed,T,H> {
    using hyperloglog_counter<m,bits,seed,T,H>::hyperloglog_counter;
    using hyperloglog_counter<m,bits,seed,T,H>::getreg;
    using hyperloglog_counter<m,bits,seed,T,H>::maxreg;
};

std::mt19937_64 gen(42);
std::uniform_int_distribution<size_t> rnd;

template <size_t m, size_t b>
hyperloglog_counter<m,b> make(size_t n) {
    hyperloglog_counter<m,b> c;
    for (; n>0; --n) c.insert(rnd(gen));
    return c;
}

template <size_t m, size_t b>
std::tuple<real_t,real_t,real_t> stats(size_t n, size_t k) {
    real_t sum = 0, sqsum = 0;
    for (size_t i=0; i<k; ++i) {
        real_t e = make<m,b>(n).size();
        e /= n;
        sum += e;
        sqsum += e*e;
    }
    sum /= k;
    sqsum = (sqsum - k*sum*sum) / (k-1);
    return {sum, sqrt(sqsum), hyperloglog_counter<m,b>::error()};
}


TEST(HyperLogLogTest, Operators) {
    hyperloglog_counter<16,6> x;
    hyperloglog_counter<16,6> y(x);
    hyperloglog_counter<16,6> z = std::move(x);
    EXPECT_EQ(z, y);
}

TEST(HyperLogLogTest, CountError) {
    real_t m, d, e;
    for (size_t k=50; k<=50000; k*=10) {
        std::tie(m, d, e) = stats<80,6>(k, 2500000/k);
        EXPECT_NEAR(d, e, 0.4*e);
        EXPECT_NEAR(m, 1, 2*e);
        //std::cerr << k << ": " << m << " " << d << " " << e << std::endl;
    }
    for (size_t k=50; k<=50000; k*=10) {
        std::tie(m, d, e) = stats<100,5>(k, 2500000/k);
        EXPECT_NEAR(d, e, 0.4*e);
        EXPECT_NEAR(m, 1, 2*e);
        //std::cerr << k << ": " << m << " " << d << " " << e << std::endl;
    }
    for (size_t k=10; k<=10000; k*=10) {
        std::tie(m, d, e) = stats<128,4>(k, 100000/k);
        EXPECT_NEAR(d, e, 0.4*e);
        EXPECT_NEAR(m, 1, 2*e);
        //std::cerr << k << ": " << m << " " << d << " " << e << std::endl;
    }
}

TEST(HyperLogLogTest, Registers) {
    size_t v;

    exposed_hll<64,6> x;
    x.maxreg(1,10);
    v = x.getreg(1);
    EXPECT_EQ(v,10ULL);

    exposed_hll<100,6> y;
    y.maxreg(1,10);
    v = y.getreg(1);
    EXPECT_EQ(v,10ULL);
}

TEST(HyperLogLogTest, Clear) {
    exposed_hll<64,6> x;
    EXPECT_TRUE(x.empty());

    x.maxreg(1,10);
    EXPECT_FALSE(x.empty());

    x.clear();
    EXPECT_TRUE(x.empty());
}

TEST(HyperLogLogTest, Insert) {
    hyperloglog_counter<64,6> x, y;
    hyperloglog_counter<128,6> w, z;
    hyperloglog_counter<100,6> p, q;
    hyperloglog_counter<64,5> x5;
    real_t s;

    // test #1
    x = hyperloglog_counter<64,6>();

    x.insert(10);
    s = x.size();

    EXPECT_FLOAT_EQ(s, 1.0078948459609032);

    // test #2a
    x = hyperloglog_counter<64,6>();
    for (size_t i=1; i<1000000; i++)
        x.insert(i);
    s = x.size();

    EXPECT_FLOAT_EQ(s, 831946.400618537);

    // test #2b (128 registers)
    w = hyperloglog_counter<128,6>();
    for (size_t i=1; i<1000000; i++)
        w.insert(i);
    s = w.size();

    EXPECT_FLOAT_EQ(s, 856712.3439500469);

    // test #2c (100 registers)
    p = hyperloglog_counter<100,6>();
    for (size_t i=1; i<1000000; i++)
        p.insert(i);
    s = p.size();

    EXPECT_FLOAT_EQ(s, 925596.88);

    // test #2d (64 registers with 5 bits)
    x5 = hyperloglog_counter<64,5>();
    for (size_t i=1; i<1000000; i++)
        x5.insert(i);
    s = x5.size();

    EXPECT_FLOAT_EQ(s, 831946.400618537);

    // test #3
    x = hyperloglog_counter<64,6>();
    y = hyperloglog_counter<64,6>();

    for (size_t i=0; i<10; i++) {
        x.insert(i);
        y.insert(i);
    }

    x.insert(y);

    s = x.size();

    EXPECT_FLOAT_EQ(s, 9.699193480140856);

    // test #4a
    x = hyperloglog_counter<64,6>();
    y = hyperloglog_counter<64,6>();

    for (size_t i=0; i<10; i++) {
        x.insert(i);
        y.insert(i+10);
    }

    s = x.size();
    s = y.size();

    x.insert(y);

    s = x.size();

    EXPECT_FLOAT_EQ(s, 18.411652636913974);

    // test #4b (128 registers)
    w = hyperloglog_counter<128,6>();
    z = hyperloglog_counter<128,6>();

    for (size_t i=0; i<10; i++) {
        w.insert(i);
        z.insert(i+10);
    }

    s = w.size();
    s = z.size();

    w.insert(z);

    s = w.size();

    EXPECT_FLOAT_EQ(s, 18.240007);

    // test #4c (100 registers)
    p = hyperloglog_counter<100,6>();
    q = hyperloglog_counter<100,6>();

    for (size_t i=0; i<10; i++) {
        p.insert(i);
        q.insert(i+10);
    }

    s = p.size();
    s = q.size();

    p.insert(q);

    s = p.size();

    EXPECT_FLOAT_EQ(s, 19.845095);
}
