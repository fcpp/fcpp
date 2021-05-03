// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include <utility>

#include "gtest/gtest.h"

#include "lib/common/multitype_map.hpp"

using namespace fcpp;


class MultitypeMapTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
        data.insert(7, 'a');
        data.insert<char>(7, 'b');
        data.insert<char>(42, '+');
        data.insert<int>(18, 31);
        data.insert(18, 999);
        data.insert(2);
        data.insert(3);
        data.insert(3);
    }

    common::multitype_map<short, int, double, char> data;
};


TEST_F(MultitypeMapTest, Operators) {
    common::multitype_map<short, int, double, char> x(data), y, z;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
}

TEST_F(MultitypeMapTest, Points) {
    EXPECT_TRUE(data.contains(2));
    EXPECT_TRUE(data.contains(3));
    data.remove(3);
    EXPECT_FALSE(data.contains(3));
    EXPECT_FALSE(data.contains(0));
    EXPECT_FALSE(data.contains(999));
}

TEST_F(MultitypeMapTest, Values) {
    EXPECT_TRUE(data.count<char>(42));
    data.erase<char>(42);
    EXPECT_FALSE(data.count<char>(42));
    EXPECT_FALSE(data.count<double>(42));
    EXPECT_EQ(999, data.at<int>(18));
    EXPECT_EQ('b', data.at<char>(7));
}
