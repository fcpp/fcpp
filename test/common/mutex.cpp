// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <mutex>
#include <thread>

#include "gtest/gtest.h"

#include "lib/common/algorithm.hpp"
#include "lib/common/mutex.hpp"

#define TRIES 10000

// slow computation
int workhard(int n=15) {
    if (n <= 1) return 1;
    return (workhard(n-1) + workhard(n-2))/2;
}

// sequential for using locks
template <typename T, bool enabled>
int work_lock(T&& ex, fcpp::mutex<enabled>&& m) {
    int acc = 0;
    std::vector<int> v(TRIES);
    fcpp::parallel_for(ex, v, [&acc,&m] (int) {
        fcpp::lock_guard<enabled> lock(m);
        int tmp = acc;
        acc = tmp + workhard();
    });
    return acc;
}

// sequential for using locks
template <typename T, bool enabled>
int work_trylock(T&& ex, fcpp::mutex<enabled>&& m) {
    int acc = 0;
    std::vector<int> v(TRIES);
    fcpp::parallel_for(ex, v, [&acc,&m] (int) {
        while (not m.try_lock());
        fcpp::lock_guard<enabled> lock(m, fcpp::adopt_lock);
        int tmp = acc;
        acc = tmp + workhard();
    });
    return acc;
}


TEST(MutexTest, Sequential) {
    int res;
    res = work_lock(fcpp::sequential_execution, fcpp::mutex<false>());
    EXPECT_EQ(TRIES, res);
    res = work_lock(fcpp::sequential_execution, fcpp::mutex<true>());
    EXPECT_EQ(TRIES, res);
    res = work_trylock(fcpp::sequential_execution, fcpp::mutex<false>());
    EXPECT_EQ(TRIES, res);
    res = work_trylock(fcpp::sequential_execution, fcpp::mutex<true>());
    EXPECT_EQ(TRIES, res);
}

TEST(MutexTest, Parallel) {
    int res;
    res = work_lock(fcpp::parallel_execution<4>, fcpp::mutex<false>());
    EXPECT_NE(TRIES, res);
    res = work_lock(fcpp::parallel_execution<4>, fcpp::mutex<true>());
    EXPECT_EQ(TRIES, res);
}

TEST(MutexTest, Trying) {
    int res;
    res = work_trylock(fcpp::parallel_execution<4>, fcpp::mutex<false>());
    EXPECT_NE(TRIES, res);
    res = work_trylock(fcpp::parallel_execution<4>, fcpp::mutex<true>());
    EXPECT_EQ(TRIES, res);
}
