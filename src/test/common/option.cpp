// Copyright © 2025 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/option.hpp"

using namespace fcpp;


TEST(OptionTest, True) {
    common::option<int, true> x(42), y, z;
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
    EXPECT_EQ(x.front(), 42);
    x.front() = 10;
    EXPECT_EQ(x.front(), 10);
    int c = 0;
    for (int& i : x) {
        EXPECT_EQ(i, 10);
        ++c;
    }
    EXPECT_EQ(c, 1);
    EXPECT_EQ((int)x.size(), 1);
    EXPECT_FALSE(x.empty());
    common::option<std::vector<int>, true> w;
    EXPECT_EQ(w.size(), 1);
    EXPECT_EQ(w.front().size(), 0);
    w = std::vector<int>{1,2,3,4};
    EXPECT_EQ(w.front().size(), 4);
    common::option<double, true> k = x;
}

TEST(OptionTest, False) {
    common::option<int, false> x(42), y, z;
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
    EXPECT_EQ(x.front(), 0);
    int c = 0;
    for (int& i : x) ++c, (void)i;
    EXPECT_EQ(c, 0);
    EXPECT_EQ((int)x.size(), 0);
    EXPECT_TRUE(x.empty());
    common::option<std::vector<int>, false> w;
    EXPECT_EQ(w.size(), 0);
    common::option<double, false> k = x;
}

TEST(OptionTest, Default) {
    common::option<int> x(42), y, z;
    EXPECT_EQ((int)y.size(), 0);
    EXPECT_TRUE(y.empty());
    int c = 0;
    for (int& i : y) ++c, (void)i;
    EXPECT_EQ(c, 0);
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
    EXPECT_EQ(x.front(), 42);
    x.front() = 10;
    EXPECT_EQ(x.front(), 10);
    c = 0;
    for (int& i : x) {
        EXPECT_EQ(i, 10);
        ++c;
    }
    EXPECT_EQ(c, 1);
    EXPECT_EQ((int)x.size(), 1);
    EXPECT_FALSE(x.empty());
    x.clear();
    c = 0;
    for (int& i : x) ++c, (void)i;
    EXPECT_EQ(c, 0);
    x.emplace(11);
    c = 0;
    for (int& i : x) {
        EXPECT_EQ(i, 11);
        ++c;
    }
    EXPECT_EQ(c, 1);
    common::option<std::vector<int>> w, wx;
    EXPECT_EQ(w.size(), 0);
    wx = w;
    EXPECT_EQ(wx.size(), 0);
    w = std::vector<int>{1,2,3,4};
    EXPECT_EQ(w.front().size(), 4);
    wx = w;
    EXPECT_EQ(w, wx);
    x = {};
    common::option<double> k = x;
    EXPECT_EQ(x.size(), 0);
    x = 42;
    k = x;
    EXPECT_EQ(x.size(), 1);
    EXPECT_EQ(x.front(), 42.0);
}
