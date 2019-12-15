// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <array>
#include <string>
#include <tuple>
#include <typeinfo>

#include "gtest/gtest.h"

#include "lib/common/traits.hpp"


TEST(TraitsTest, Sequence) {
    int i = fcpp::type_sequence<int,double,double,char>::size();
    EXPECT_EQ(4, i);
    std::string ex, res;
    ex  = typeid(fcpp::type_sequence<int,double,double,char>).name();
    res = typeid(fcpp::type_sequence<int,double,double,char>::append<>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::type_sequence<int,double,double>::append<char>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::type_sequence<int,double>::append<double,char>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::type_sequence<int>::append<double,double,char>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::type_sequence<>::append<int,double,double,char>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::type_sequence<double,char>::prepend<int,double>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::type_sequence<short,int,double,double,char>::tail).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(short).name();
    res = typeid(fcpp::type_sequence<short,int,double,double,char>::head).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::type_sequence<double,char>).name();
    res = typeid(fcpp::type_subseq<2,1,int,double,double,char>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::type_sequence<int,char>).name();
    res = typeid(fcpp::type_subseq<3,0,int,double,double,char>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::type_sequence<double>).name();
    res = typeid(fcpp::type_subseq<5,2,int,double,double,char>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(double).name();
    res = typeid(fcpp::nth_type<2,int,int,double,char>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::type_sequence<int,char>).name();
    res = typeid(fcpp::type_sequence<int,double,double,char>::intersect<short,char,int>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::type_sequence<double,double>).name();
    res = typeid(fcpp::type_sequence<int,double,double,char>::intersect<double>).name();
    EXPECT_EQ(ex, res);
}

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
    b = fcpp::type_repeated<>;
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
    b = fcpp::has_template<proxy, const proxy<double>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, proxy<double>&>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, proxy<double>&&>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, const proxy<double>&>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, const proxy<double>&&>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, int>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, const int>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, std::array<proxy<double>,4>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, const std::array<proxy<double>,4>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, std::array<int,4>>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, const std::array<int,4>>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, std::tuple<proxy<double>,char>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, const std::tuple<proxy<double>,char>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, std::tuple<int,char>>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, const std::tuple<int,char>>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, const std::array<std::tuple<std::array<proxy<double>,3>,char>,4>&>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>&&>;
    EXPECT_TRUE(b);
    b = fcpp::has_template<proxy, std::array<std::tuple<std::array<double,3>,char>,4>>;
    EXPECT_FALSE(b);
    b = fcpp::has_template<proxy, const std::array<std::tuple<std::array<double,3>,char>,4>&>;
    EXPECT_FALSE(b);
}

TEST(TraitsTest, DelTemplate) {
    std::string ex, res;
    ex  = typeid(double).name();
    res = typeid(fcpp::del_template<proxy, double>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::del_template<proxy, proxy<double>>).name();
    EXPECT_EQ(ex, res);
    bool b;
    b = std::is_same<const double, fcpp::del_template<proxy, const double>>::value;
    EXPECT_TRUE(b);
    b = std::is_same<const double, fcpp::del_template<proxy, const proxy<double>>>::value;
    EXPECT_TRUE(b);
    b = std::is_same<const double&, fcpp::del_template<proxy, const proxy<double>&>>::value;
    EXPECT_TRUE(b);
    b = std::is_same<double&&, fcpp::del_template<proxy, proxy<double>&&>>::value;
    EXPECT_TRUE(b);
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
