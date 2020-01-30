// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <vector>
#include <random>

#include "gtest/gtest.h"

#include "lib/common/algorithm.hpp"


TEST(AlgorithmTest, NthElements) {
    std::mt19937 rnd(42);
    std::vector<int> ev, iv, inum = {1, 3, 10, 30, 100, 300, 1000};
    
    for (int i=0; i<1000; ++i) ev.push_back(i);
    for (int n : inum) {
        std::shuffle(ev.begin(), ev.end(), rnd);
        std::uniform_int_distribution<int> distr(0, 1000-n);
        iv.clear();
        for (int i=0; i<n; ++i) iv.push_back(distr(rnd));
        std::sort(iv.begin(), iv.end());
        for (int i=0; i<n; ++i) iv[i] += i;
        
        fcpp::nth_elements(ev.begin(), ev.end(), iv.begin(), iv.end());
        for (int i : iv)
            EXPECT_EQ(i, ev[i]);
    }
}
