// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include <array>
#include <string>
#include <tuple>
#include <typeinfo>

#include "gtest/gtest.h"

#include "lib/common/traits.hpp"


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

TEST(TraitsTest, BoolPack) {
    bool val;
    val = fcpp::all_true<>;
    EXPECT_TRUE(val);
    val = fcpp::all_true<true, true, true>;
    EXPECT_TRUE(val);
    val = fcpp::all_true<true, false>;
    EXPECT_FALSE(val);
    val = fcpp::all_false<>;
    EXPECT_TRUE(val);
    val = fcpp::all_false<false, false, false>;
    EXPECT_TRUE(val);
    val = fcpp::all_false<true, false>;
    EXPECT_FALSE(val);
    val = fcpp::some_true<>;
    EXPECT_FALSE(val);
    val = fcpp::some_true<false, false, false>;
    EXPECT_FALSE(val);
    val = fcpp::some_true<true, false>;
    EXPECT_TRUE(val);
    val = fcpp::some_false<>;
    EXPECT_FALSE(val);
    val = fcpp::some_false<true, true, true>;
    EXPECT_FALSE(val);
    val = fcpp::some_false<true, false>;
    EXPECT_TRUE(val);
}

template<class T> struct proxy {};

TEST(TraitsTest, HasTemplate) {
    bool b;
    b = fcpp::has_template<proxy, proxy<double>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, int>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, std::array<proxy<double>,4>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, std::array<int,4>>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, std::tuple<proxy<double>,char>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, std::tuple<int,char>>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, std::array<std::tuple<std::array<double,3>,char>,4>>;
    EXPECT_FALSE(b);
}

TEST(TraitsTest, DelTemplate) {
    std::string ex, res;
    ex  = typeid(double).name();
    res = typeid(fcpp::del_template<proxy, double>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::del_template<proxy, proxy<double>>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(std::array<double,4>).name();
    res = typeid(fcpp::del_template<proxy, std::array<proxy<double>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::del_template<proxy, proxy<std::array<double,4>>>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(std::tuple<double,char>).name();
    res = typeid(fcpp::del_template<proxy, std::tuple<proxy<double>,char>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::del_template<proxy, proxy<std::tuple<double,char>>>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(std::array<std::tuple<std::array<double,3>,char>,4>).name();
    res = typeid(fcpp::del_template<proxy, std::array<std::tuple<std::array<double,3>,char>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::del_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::del_template<proxy, std::array<std::tuple<proxy<std::array<double,3>>,char>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::del_template<proxy, std::array<proxy<std::tuple<std::array<double,3>,char>>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::del_template<proxy, proxy<std::array<std::tuple<std::array<double,3>,char>,4>>>).name();
    EXPECT_EQ(ex, res);
}

TEST(TraitsTest, AddTemplate) {
    std::string ex, res;
    ex  = typeid(proxy<std::array<std::tuple<std::array<double,3>,char>,4>>).name();
    res = typeid(fcpp::add_template<proxy, std::array<std::tuple<std::array<double,3>,char>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::add_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::add_template<proxy, std::array<std::tuple<proxy<std::array<double,3>>,char>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::add_template<proxy, std::array<proxy<std::tuple<std::array<double,3>,char>>,4>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::add_template<proxy, proxy<std::array<std::tuple<std::array<double,3>,char>,4>>>).name();
    EXPECT_EQ(ex, res);
}

template<class T> struct other {};

TEST(TraitsTest, NestedTemplate) {
    bool b;
    b = fcpp::nested_template<proxy, proxy<int>>;
    EXPECT_TRUE(b);
    b = fcpp::nested_template<proxy, int>;
    EXPECT_FALSE(b);
    b = fcpp::nested_template<proxy, other<proxy<int>>>;
    EXPECT_TRUE(b);
    b = fcpp::nested_template<proxy, other<int>>;
    EXPECT_FALSE(b);
    b = fcpp::nested_template<proxy, other<other<proxy<int>>>>;
    EXPECT_TRUE(b);
    b = fcpp::nested_template<proxy, other<other<int>>>;
    EXPECT_FALSE(b);
}

TEST(TraitsTest, NestTemplate) {
    std::string ex, res;
    ex  = typeid(proxy<int>).name();
    res = typeid(fcpp::nest_template<int, proxy>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::nest_template<proxy<int>, proxy>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(proxy<other<int>>).name();
    res = typeid(fcpp::nest_template<proxy<other<int>>, proxy>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::nest_template<proxy<other<int>>, other>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::nest_template<other<int>, proxy>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::nest_template<proxy<other<int>>>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::nest_template<proxy<other<int>>, proxy, other>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::nest_template<other<int>, proxy, other>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::nest_template<int, proxy, other>).name();
    EXPECT_EQ(ex, res);
}
