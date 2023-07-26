// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/option/filter.hpp"

using namespace fcpp;

TEST(FilterTest, Filters) {
    {
        filter::finite f;
        EXPECT_EQ(f(05), true);
        EXPECT_EQ(f(15), true);
        EXPECT_EQ(f(1.0/0.0), false);
        EXPECT_EQ(f(0.0/0.0), false);
    }
    {
        filter::within<10, 20> f;
        EXPECT_EQ(f(05), false);
        EXPECT_EQ(f(15), true);
        EXPECT_EQ(f(25), false);
    }
    {
        filter::neg<filter::within<10, 20>> f;
        EXPECT_EQ(f(05), true);
        EXPECT_EQ(f(15), false);
        EXPECT_EQ(f(25), true);
    }
    {
        filter::vee<filter::below<10>, filter::above<20>> f;
        EXPECT_EQ(f(05), true);
        EXPECT_EQ(f(15), false);
        EXPECT_EQ(f(25), true);
    }
    {
        filter::neg<filter::wedge<filter::above<10>, filter::below<20>>> f;
        EXPECT_EQ(f(05), true);
        EXPECT_EQ(f(15), false);
        EXPECT_EQ(f(25), true);
    }
}
