// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/algorithm.hpp"
#include "lib/common/mutex.hpp"

#define TRIES 10000

using namespace fcpp;


// slow computation
int workhard(int& t, int n=15) {
    if (n <= 1) return 1;
    t += 1;
    int r = (workhard(t, n-1) + workhard(t, n-2))/2;
    t -= 1;
    return r;
}

// sequential for using locks
template <typename T, bool enabled>
int work_lock(T&& ex, common::mutex<enabled>&& m) {
    int acc = 0;
    common::parallel_for(ex, TRIES, [&acc,&m] (int,int) {
        common::lock_guard<enabled> lock(m);
        int tmp = acc;
        acc = tmp + workhard(tmp);
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
        acc = tmp + workhard(tmp);
    });
    return acc;
}

#ifdef FCPP_DISABLE_THREADS
#define EXPECT_NEQ(a, b)    EXPECT_EQ(a, b)
#define EXPECT_LEQ(a, b)    EXPECT_EQ(a, -1)
#else
#define EXPECT_NEQ(a, b)    EXPECT_NE(a, b)
#define EXPECT_LEQ(a, b)    EXPECT_EQ(a, b)
#endif


TEST(MutexTest, Sequential) {
    int res;
    res = work_lock(common::tags::sequential_execution(), common::mutex<false>());
    EXPECT_EQ(TRIES, res);
    res = work_lock(common::tags::sequential_execution(), common::mutex<true>());
    EXPECT_EQ(TRIES, res);
    res = work_trylock(common::tags::sequential_execution(), common::mutex<false>());
    EXPECT_EQ(TRIES, res);
    res = work_trylock(common::tags::sequential_execution(), common::mutex<true>());
    EXPECT_EQ(TRIES, res);
}

TEST(MutexTest, Parallel) {
    int res;
    res = work_lock(common::tags::parallel_execution(4), common::mutex<false>());
    EXPECT_NEQ(TRIES, res);
    res = work_lock(common::tags::parallel_execution(4), common::mutex<true>());
    EXPECT_EQ(TRIES, res);
    res = work_trylock(common::tags::parallel_execution(4), common::mutex<false>());
    EXPECT_NEQ(TRIES, res);
    res = work_trylock(common::tags::parallel_execution(4), common::mutex<true>());
    EXPECT_EQ(TRIES, res);
}

TEST(MutexTest, Locking) {
    {
        constexpr bool enabled = false;
        common::mutex<enabled> m1, m2;
        common::lock_guard<enabled> l(m1);
        EXPECT_EQ(common::try_lock(m2, m1), -1);
        {
            common::unlock_guard<enabled> u(m1);
            common::lock(m1, m2);
            EXPECT_EQ(common::try_lock(m1, m2), -1);
            EXPECT_EQ(common::try_lock(m2, m1), -1);
            m1.unlock();
            m2.unlock();
        }
        EXPECT_EQ(try_lock(m2, m1), -1);
        {
            common::unlock_guard<enabled> u(m1);
            EXPECT_EQ(common::try_lock(m1, m2), -1);
            m1.unlock();
            m2.unlock();
        }
    }
    {
        constexpr bool enabled = true;
        common::mutex<enabled> m1, m2;
        common::lock_guard<enabled> l(m1);
        EXPECT_LEQ(common::try_lock(m2, m1), 1);
        {
            common::unlock_guard<enabled> u(m1);
            common::lock(m1, m2);
            EXPECT_LEQ(common::try_lock(m1, m2), 0);
            EXPECT_LEQ(common::try_lock(m2, m1), 0);
            m1.unlock();
            m2.unlock();
        }
        EXPECT_LEQ(common::try_lock(m2, m1), 1);
        {
            common::unlock_guard<enabled> u(m1);
            EXPECT_LEQ(common::try_lock(m1, m2), -1);
            m1.unlock();
            m2.unlock();
        }
    }
}
