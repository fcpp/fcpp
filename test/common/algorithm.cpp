// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <queue>
#include <random>
#include <vector>

#include "gtest/gtest.h"

#include "lib/common/algorithm.hpp"

// slow computation
int workhard(int n=15) {
    if (n <= 1) return 1;
    return (workhard(n-1) + workhard(n-2))/2;
}


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
        
        fcpp::nth_elements(ev.begin(), ev.end(), iv.begin(), iv.end());
        for (int i : iv)
            EXPECT_EQ(i, ev[i]);
    }
}

TEST(AlgorithmTest, ParallelFor) {
    const int N = 10000;
    std::vector<int> v(N);
    int acc;
    for (size_t i=0; i<v.size(); ++i) v[i] = i;
    auto worker = [&acc](int) {
        int tmp = acc;
        acc = tmp + workhard();
    };
    acc = 0;
    fcpp::parallel_for(fcpp::sequential_execution, N, worker);
    EXPECT_EQ(N, acc);
    acc = 0;
    fcpp::parallel_for(fcpp::general_execution<1>, N, worker);
    EXPECT_EQ(N, acc);
    acc = 0;
    fcpp::parallel_for(fcpp::parallel_execution<4>, N, worker);
    EXPECT_NE(N, acc);
    acc = 0;
    fcpp::parallel_for(fcpp::general_execution<4>, N, worker);
    EXPECT_NE(N, acc);
    fcpp::parallel_for(fcpp::parallel_execution<4>, N, [&v](size_t i) { ++v[i]; });
    for (size_t i=0; i<N; ++i)
        EXPECT_EQ(int(i+1), v[i]);
}

TEST(AlgorithmTest, ParallelWhile) {
    std::mt19937 rnd(42);
    auto make_queue = [&rnd] (int N) {
        std::priority_queue<int> q;
        for (int i=0; i<N; ++i) q.push(i);
        return q;
    };
    std::priority_queue<int> q;
    int acc;
    auto popper = [&q,&acc] () {
        if (q.empty()) return false;
        q.pop();
        int tmp = acc;
        acc = tmp + workhard();
        return true;
    };
    q = make_queue(10000);
    acc = 0;
    fcpp::parallel_while(fcpp::sequential_execution, popper);
    EXPECT_EQ(10000, acc);
    q = make_queue(10000);
    acc = 0;
    fcpp::parallel_while(fcpp::parallel_execution<8>, popper);
    EXPECT_NE(10000, acc);
}
