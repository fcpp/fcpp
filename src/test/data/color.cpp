// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/data/color.hpp"

using namespace fcpp;

void print(color c) {
    std::cerr << "(" << c.red() << ", " << c.green() << ", " << c.blue() << ", " << c.alpha() << ")\n";
}

TEST(ColorTest, Packed) {
    packed_color c1, c2;

    c1 = packed_rgba(229, 229, 45);
    c2 = packed_hsva(60, 80, 90);
    EXPECT_EQ(c1, c2);
    c1 = packed_rgba(91, 122, 153, 127);
    c2 = packed_hsva(210, 40, 60, 50);
    EXPECT_EQ(c1, c2);
}

TEST(ColorTest, Color) {
    color c1, c2, c3, c4;

    c1 = color(229, 229, 45);
    c2 = color::hsva(60, 0.80, 0.90);
    c3 = color(0.9, 0.9, 0.18);
    c4 = color(0xE5E52DFF);
    EXPECT_EQ(c1, c2);
    EXPECT_EQ(c2, c3);
    EXPECT_EQ(c3, c4);
    c1 = color(91, 122, 153, 127);
    c2 = color::hsva(210, 0.40, 0.60, 0.50);
    c3 = color(0.36, 0.48, 0.6, 0.5);
    c4 = color(0x5B7A997F);
    EXPECT_EQ(c1, c2);
    EXPECT_EQ(c2, c3);
    EXPECT_EQ(c3, c4);
}
