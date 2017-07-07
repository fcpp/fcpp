// Copyright © 2017 Giorgio Audrito. All Rights Reserved.

#include <string>
#include <typeinfo>
#include <vector>

#include "gtest/gtest.h"

#include "lib/util/traits.hpp"


TEST(TraitsTest, Index) {
    int t;
    t = fcpp::type_index<int,int,double,char>;
    EXPECT_EQ(0, t);
    t = fcpp::type_index<double,int,char,double>;
    EXPECT_EQ(2, t);
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

TEST(TraitsTest, IsTemplate) {
    bool b;
    b = fcpp::is_template<std::vector, std::vector<double>>;
    EXPECT_TRUE(b);
    b = fcpp::is_template<std::vector, int>;
    EXPECT_FALSE(b);
}

TEST(TraitsTest, ToTemplate) {
    std::string ex, res;
    ex  = typeid(double).name();
    res = typeid(typename fcpp::to_template<std::vector, std::vector<double>>::type::value_type).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(int).name();
    res = typeid(typename fcpp::to_template<std::vector, int>::type::value_type).name();
    EXPECT_EQ(ex, res);
}
