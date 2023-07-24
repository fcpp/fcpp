// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include <vector>

#include "lib/data/ordered.hpp"

using namespace fcpp;


TEST(OrderedTest, Assignments) {
    ordered<int> x{};
    EXPECT_EQ(int(x), 0);
    ordered<int> y{x}, z = x;
    x = 42;
    y = make_ordered(42);
    z = y;
    EXPECT_EQ(int(x), int(y));
}

TEST(OrderedTest, Ordering) {
    ordered<int> x(1), y(2);
    EXPECT_NE(int(x), int(y));
    EXPECT_EQ(x, y);
    ordered<std::vector<int>> v({1,2,3,4}), u({5,6});
    EXPECT_EQ(x, y);
    EXPECT_LE(x, y);
    EXPECT_GE(x, y);
    auto t = make_tuple(4, make_ordered(v.data));
    auto s = make_tuple(2, make_ordered(u.data));
    EXPECT_LT(s, t);
}
