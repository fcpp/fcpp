// Copyright © 2017 Giorgio Audrito. All Rights Reserved.

#include <vector>

#include "gtest/gtest.h"

#include "lib/util/traits.hpp"


TEST(TraitsTest, Index) {
    size_t t;
    t = fcpp::type_index<int,int,double,char>;
    EXPECT_EQ(t, 0);
    t = fcpp::type_index<double,int,char,double>;
    EXPECT_EQ(t, 2);
}

TEST(TraitsTest, Contains) {
    bool b;
    b = fcpp::type_contains<double,int,char,double>;
    EXPECT_TRUE(b);
    b = fcpp::type_contains<int,double,char>;
    EXPECT_FALSE(b);
}

TEST(TraitsTest, Repeated) {
    bool b;
    b = fcpp::type_repeated<int,double,char,int>;
    EXPECT_TRUE(b);
    b = fcpp::type_repeated<void*,int,double,char>;
    EXPECT_FALSE(b);
}

TEST(TraitsTest, Template) {
    bool b;
    b = fcpp::is_template<std::vector, std::vector<double>>;
    EXPECT_TRUE(b);
    b = fcpp::is_template<std::vector, int>;
    EXPECT_FALSE(b);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
