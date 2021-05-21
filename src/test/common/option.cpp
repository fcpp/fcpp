// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/option.hpp"

using namespace fcpp;


TEST(OptionTest, True) {
    constexpr bool enable = true;
    common::option<int, enable> x(42), y, z;
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
    EXPECT_EQ(x.front(), 42);
    x.front() = 10;
    EXPECT_EQ(x.front(), 10);
}

TEST(OptionTest, False) {
    constexpr bool enable = false;
    common::option<int, enable> x(42), y, z;
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
    EXPECT_EQ(x.front(), 0);
}
