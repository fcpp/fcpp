// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <unordered_map>
#include <vector>

#include "gtest/gtest.h"

#include "lib/common/random_access_map.hpp"

using namespace fcpp;


TEST(RandomAccessMapTest, Constructors) {
    std::unordered_map<int, double> m = {{1,3}, {2,4}};
    common::random_access_map<int, double> k;
    common::random_access_map<int, double> x = {{1,3}, {2,4}};
    common::random_access_map<int, double> y(x);
    EXPECT_EQ(x, y);
    common::random_access_map<int, double> z(std::move(y));
    EXPECT_EQ(x, z);
    common::random_access_map<int, double> w(m.begin(), m.end());
    EXPECT_EQ(x, w);
    y = w;
    EXPECT_EQ(x, y);
    y = {{2,2}, {5,7}};
    EXPECT_NE(x, y);
    m = {{2,2}, {5,7}};
    common::random_access_map<int, double> l(m.begin(), m.end());
    EXPECT_EQ(l, y);
    w = std::move(y);
    EXPECT_EQ(l, w);
}

TEST(RandomAccessMapTest, Access) {
    common::random_access_map<int, double> x = {{1,3}, {2,4}}, y;
    EXPECT_TRUE(y.empty());
    EXPECT_FALSE(x.empty());
    EXPECT_EQ(2, (int)x.size());
    EXPECT_EQ(0, (int)y.size());
    EXPECT_EQ(3.0, x[1]);
    EXPECT_EQ(4.0, x[2]);
    EXPECT_EQ(0.0, x[7]);
    EXPECT_EQ(3, (int)x.size());
    EXPECT_EQ(3.0, x.at(1));
    EXPECT_EQ(4.0, x.at(2));
    EXPECT_EQ(0.0, x.at(7));
    EXPECT_EQ(1, (int)x.count(7));
    EXPECT_EQ(0, (int)x.count(42));
    y = x;
    x.clear();
    EXPECT_EQ(0, (int)x.size());
    EXPECT_EQ(0, (int)x.count(7));
    x.swap(y);
    EXPECT_EQ(3, (int)x.size());
    EXPECT_EQ(4.0, x.at(2));
}

TEST(RandomAccessMapTest, Modify) {
    common::random_access_map<int, double> x = {{1,3}, {2,4}, {3,8}, {11,42}};
    auto p = x.emplace(9,4.5);
    EXPECT_TRUE(p.second);
    EXPECT_EQ(9, p.first->first);
    EXPECT_EQ(4.5, p.first->second);
    p = x.emplace(2,5);
    EXPECT_FALSE(p.second);
    EXPECT_EQ(2, p.first->first);
    EXPECT_EQ(4.0, p.first->second);
    p = x.insert({23,4.5});
    EXPECT_TRUE(p.second);
    EXPECT_EQ(23, p.first->first);
    EXPECT_EQ(4.5, p.first->second);
    auto arg = std::make_pair(2,5.0);
    p = x.insert(arg);
    EXPECT_FALSE(p.second);
    EXPECT_EQ(2, p.first->first);
    EXPECT_EQ(4.0, p.first->second);
    EXPECT_EQ(6, (int)x.size());
    common::random_access_map<int, double> y = {{2,2}, {5,7}};
    x.insert(y.begin(), y.end());
    EXPECT_EQ(7, (int)x.size());
    EXPECT_EQ(0, (int)x.erase(92));
    EXPECT_EQ(7, (int)x.size());
    EXPECT_EQ(1, (int)x.erase(11));
    EXPECT_EQ(6, (int)x.size());
    auto it = x.erase(x.find(3));
    EXPECT_TRUE(it == x.end() or it->first != 3);
    EXPECT_EQ(5, (int)x.size());
}

TEST(RandomAccessMapTest, Iterators) {
    common::random_access_map<int, double> x = {{1,3}, {2,4}, {3,8}, {11,42}};
    common::random_access_map<int, double> y(x.begin(), x.end());
    EXPECT_EQ(x, y);
    auto it = x.find(2);
    EXPECT_EQ(2, it->first);
    EXPECT_EQ(4.0, it->second);
    it = x.find(9);
    EXPECT_EQ(x.end(), it);
    EXPECT_EQ(x.size(), (size_t)(x.end()-x.begin()));
    std::vector<int> v;
    it = x.begin();
    for (size_t i=0; i<x.size(); ++i) {
        v.push_back(it[i].first);
        v.push_back((int)it[i].second);
    }
    std::sort(v.begin(), v.end());
    std::vector<int> w = {1,2,3,3,4,8,11,42};
    EXPECT_EQ(w, v);
}
