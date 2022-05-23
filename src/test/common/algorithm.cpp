// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <queue>
#include <random>

#include "gtest/gtest.h"

#include "lib/common/algorithm.hpp"

using namespace fcpp;


// slow computation
int workhard(int& t, int n=15) {
    if (n <= 1) return 1;
    t += 1;
    int r = (workhard(t, n-1) + workhard(t, n-2))/2;
    t -= 1;
    return r;
}

#ifdef FCPP_DISABLE_THREADS
#define EXPECT_NEQ(a, b)    EXPECT_EQ(a, b)
#else
#define EXPECT_NEQ(a, b)    EXPECT_NE(a, b)
#endif


TEST(AlgorithmTest, NthElements) {
    std::mt19937 rnd(42);
    std::vector<int> ev, iv, inum = {1, 3, 10, 30, 100, 300, 1000};

    for (int i=0; i<10000; ++i) ev.push_back(i);
    for (int n : inum) {
        std::shuffle(ev.begin(), ev.end(), rnd);
        std::uniform_int_distribution<int> distr(0, ev.size()-n);
        iv.clear();
        for (int i=0; i<n; ++i) iv.push_back(distr(rnd));
        std::sort(iv.begin(), iv.end());
        for (int i=0; i<n; ++i) iv[i] += i;

        common::nth_elements(ev.begin(), ev.end(), iv.begin(), iv.end());
        for (int i : iv)
            EXPECT_EQ(i, ev[i]);
    }
}

TEST(AlgorithmTest, ParallelFor) {
    int const N = 10000;
    std::vector<int> v(N);
    int acc;
    for (size_t i=0; i<v.size(); ++i) v[i] = i;
    auto worker = [&acc](size_t,size_t) {
        int tmp = acc;
        acc = tmp + workhard(tmp);
    };
    acc = 0;
    common::parallel_for(common::tags::sequential_execution(), N, worker);
    EXPECT_EQ(N, acc);
    acc = 0;
    common::parallel_for(common::tags::general_execution<false>(), N, worker);
    EXPECT_EQ(N, acc);
    acc = 0;
    common::parallel_for(common::tags::parallel_execution(4), N, worker);
    EXPECT_NEQ(N, acc);
    acc = 0;
    common::parallel_for(common::tags::general_execution<true>(4), N, worker);
    EXPECT_NEQ(N, acc);
    acc = 0;
    common::parallel_for(common::tags::dynamic_execution(4), N, worker);
    EXPECT_NEQ(N, acc);
    acc = 0;
    std::mutex m;
    common::parallel_for(common::tags::general_execution<true>(4), N, [&acc,&m](size_t,size_t) {
        std::lock_guard<std::mutex> l(m);
        int tmp = acc;
        acc = tmp + workhard(tmp);
    });
    EXPECT_EQ(N, acc);
    acc = 0;
    int multiacc[4] = {0,0,0,0};
    common::parallel_for(common::tags::dynamic_execution(4,5), N, [&multiacc](size_t,size_t t) {
        int tmp = multiacc[t];
        multiacc[t] = tmp + workhard(tmp);
    });
    for (int i=0; i<4; ++i) acc += multiacc[i];
    EXPECT_EQ(N, acc);
    common::parallel_for(common::tags::parallel_execution(4), N, [&v](size_t i, size_t) { ++v[i]; });
    for (size_t i=0; i<N; ++i)
        EXPECT_EQ(int(i+1), v[i]);
}

TEST(AlgorithmTest, ParallelWhile) {
    std::mt19937 rnd(42);
    auto make_queue = [] (int N) {
        std::priority_queue<int> q;
        for (int i=0; i<N; ++i) q.push(i);
        return q;
    };
    std::priority_queue<int> q;
    std::mutex m;
    int acc, N = 10000;
    q = make_queue(N);
    acc = 0;
    common::parallel_while(common::tags::sequential_execution(), [&q,&acc] (size_t,size_t) {
        if (q.empty()) return false;
        q.pop();
        int tmp = acc;
        acc = tmp + workhard(tmp);
        return true;
    });
    EXPECT_EQ(N, acc);
    q = make_queue(N);
    acc = 0;
    common::parallel_while(common::tags::parallel_execution(8), [&q,&m,&acc] (size_t,size_t) {
        {
            std::lock_guard<std::mutex> l(m);
            if (q.empty()) return false;
            q.pop();
        }
        int tmp = acc;
        acc = tmp + workhard(tmp);
        return true;
    });
    EXPECT_NEQ(N, acc);
    q = make_queue(N);
    acc = 0;
    common::parallel_while(common::tags::parallel_execution(8), [&q,&m,&acc] (size_t,size_t) {
        std::lock_guard<std::mutex> l(m);
        if (q.empty()) return false;
        q.pop();
        int tmp = acc;
        acc = tmp + workhard(tmp);
        return true;
    });
    EXPECT_EQ(N, acc);
    q = make_queue(N);
    acc = 0;
    common::parallel_while(common::tags::dynamic_execution(8,5), [&q,&m,&acc] (size_t,size_t) {
        {
            std::lock_guard<std::mutex> l(m);
            if (q.empty()) return false;
            q.pop();
        }
        int tmp = acc;
        acc = tmp + workhard(tmp);
        return true;
    });
    EXPECT_NEQ(N, acc);
    q = make_queue(N);
    acc = 0;
    common::parallel_while(common::tags::dynamic_execution(8,5), [&q,&m,&acc] (size_t,size_t) {
        std::lock_guard<std::mutex> ll(m);
        if (q.empty()) return false;
        q.pop();
        int tmp = acc;
        acc = tmp + workhard(tmp);
        return true;
    });
    EXPECT_EQ(N, acc);
}

TEST(AlgorithmTest, UniformInsert) {
    std::vector<int> v;
    common::uniform_insert(v, 42);
    std::set<int> s;
    common::uniform_insert(s, 42);
    std::multiset<int> m;
    common::uniform_insert(m, 42);
}
