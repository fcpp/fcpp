// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <mutex>
#include <thread>

#include "gtest/gtest.h"

#include "lib/common/algorithm.hpp"
#include "lib/common/mutex.hpp"

#define TRIES 10000

using namespace fcpp;


// slow computation
int workhard(int n=15) {
    if (n <= 1) return 1;
    return (workhard(n-1) + workhard(n-2))/2;
}

// sequential for using locks
template <typename T, bool enabled>
int work_lock(T&& ex, common::mutex<enabled>&& m) {
    int acc = 0;
    common::parallel_for(ex, TRIES, [&acc,&m] (int,int) {
        common::lock_guard<enabled> lock(m);
        int tmp = acc;
        acc = tmp + workhard();
    });
    return acc;
}

// sequential for using locks
template <typename T, bool enabled>
int work_trylock(T&& ex, common::mutex<enabled>&& m) {
    int acc = 0;
    common::parallel_for(ex, TRIES, [&acc,&m] (int,int) {
        while (not m.try_lock());
        common::lock_guard<enabled> lock(m, std::adopt_lock);
        int tmp = acc;
        acc = tmp + workhard();
    });
    return acc;
}


TEST(MutexTest, Sequential) {
    int res;
    res = work_lock(common::sequential_execution, common::mutex<false>());
    EXPECT_EQ(TRIES, res);
    res = work_lock(common::sequential_execution, common::mutex<true>());
    EXPECT_EQ(TRIES, res);
    res = work_trylock(common::sequential_execution, common::mutex<false>());
    EXPECT_EQ(TRIES, res);
    res = work_trylock(common::sequential_execution, common::mutex<true>());
    EXPECT_EQ(TRIES, res);
}

TEST(MutexTest, Parallel) {
    int res;
    res = work_lock(common::parallel_execution<4>, common::mutex<false>());
    EXPECT_NE(TRIES, res);
    res = work_lock(common::parallel_execution<4>, common::mutex<true>());
    EXPECT_EQ(TRIES, res);
}

TEST(MutexTest, Trying) {
    int res;
    res = work_trylock(common::parallel_execution<4>, common::mutex<false>());
    EXPECT_NE(TRIES, res);
    res = work_trylock(common::parallel_execution<4>, common::mutex<true>());
    EXPECT_EQ(TRIES, res);
}
