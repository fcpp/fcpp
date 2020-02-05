// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/lock.hpp"

// just there to force some time to wait
int workhard(int n=15) {
    if (n <= 1) return 1;
    return (workhard(n-1) + workhard(n-2))/2;
}

TEST(LockTest, Sequential) {
    fcpp::lock<false> l;
    int accumulate = 0;
    for (int i=0; i<10000; ++i) {
        l.set();
        int tmp = accumulate;
        accumulate = tmp + workhard();
        l.unset();
    }
    EXPECT_EQ(10000, accumulate);
    accumulate = 0;
    for (int i=0; i<10000; ++i) {
        while (not l.test());
        int tmp = accumulate;
        accumulate = tmp + workhard();
        l.unset();
    }
    EXPECT_EQ(10000, accumulate);
}

#ifdef _OPENMP
TEST(LockTest, FalseLock) {
    omp_set_num_threads(2);
    fcpp::lock<false> l;
    int accumulate = 0;
    #pragma omp parallel for
    for (int i=0; i<10000; ++i) {
        int tmp = accumulate;
        accumulate = tmp + workhard();
    }
    EXPECT_NE(10000, accumulate);
    accumulate = 0;
    #pragma omp parallel for
    for (int i=0; i<10000; ++i) {
        l.set();
        int tmp = accumulate;
        accumulate = tmp + workhard();
        l.unset();
    }
    EXPECT_NE(10000, accumulate);
    accumulate = 0;
    #pragma omp parallel for
    for (int i=0; i<10000; ++i) {
        while (not l.test());
        int tmp = accumulate;
        accumulate = tmp + workhard();
        l.unset();
    }
    EXPECT_NE(10000, accumulate);
}

TEST(LockTest, TrueLock) {
    omp_set_num_threads(2);
    fcpp::lock<true> l;
    int accumulate = 0;
    #pragma omp parallel for
    for (int i=0; i<10000; ++i) {
        l.set();
        int tmp = accumulate;
        accumulate = tmp + workhard();
        l.unset();
    }
    EXPECT_EQ(10000, accumulate);
    accumulate = 0;
    #pragma omp parallel for
    for (int i=0; i<10000; ++i) {
        while (not l.test());
        int tmp = accumulate;
        accumulate = tmp + workhard();
        l.unset();
    }
    EXPECT_EQ(10000, accumulate);
}
#endif
