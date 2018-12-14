// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include <utility>

#include "gtest/gtest.h"

#include "lib/data/exports.hpp"


class ExportsTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        data.insert<char>(7, 'a');
        data.insert<char>(7, 'b');
        data.insert<char>(42, '+');
        data.insert<int>(18, 999);
        data.insert(2);
        data.insert(3);
        data.insert(3);
    }
    
    fcpp::exports<int, double, char> data;
};


TEST_F(ExportsTest, Values) {
    EXPECT_TRUE(data.count<char>(42));
    EXPECT_FALSE(data.count<double>(42));
    EXPECT_EQ(999, data.at<int>(18));
    EXPECT_EQ('b', data.at<char>(7));
}

TEST_F(ExportsTest, Points) {
    EXPECT_TRUE(data.contains(2));
    EXPECT_TRUE(data.contains(3));
    EXPECT_FALSE(data.contains(0));
    EXPECT_FALSE(data.contains(999));
}

TEST_F(ExportsTest, Operators) {
    fcpp::exports<int, double, char> x(data), y, z;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
}
