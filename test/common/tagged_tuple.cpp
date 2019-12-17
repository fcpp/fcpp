// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/tagged_tuple.hpp"


struct tag {};
struct gat {};
struct oth {};
struct hto {};


class TagTupleTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
    }
    
    fcpp::tagged_tuple<tag, int, gat, bool> t{2, true};
};


TEST_F(TagTupleTest, Operators) {
    fcpp::tagged_tuple<tag, int, gat, bool> x(t), y, z;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(t, z);
    y = fcpp::tagged_tuple<tag, int, gat, bool>{1, false};
    EXPECT_NE(t, y);
    y = fcpp::make_tagged_tuple<tag,gat>(2, true);
    EXPECT_EQ(t, y);
    fcpp::tagged_tuple<> e(y);
    fcpp::tagged_tuple<> f = fcpp::make_tagged_tuple<>();
    EXPECT_EQ(e, f);
}

TEST_F(TagTupleTest, Get) {
    int i;
    i = fcpp::get<tag>(t);
    EXPECT_EQ(2, i);
    fcpp::get<tag>(t) = 3;
    i = fcpp::get<tag>(t);
    EXPECT_EQ(3, i);
    bool b;
    b = fcpp::get<gat>(t);
    EXPECT_EQ(true, b);
    i = std::get<0>(t);
    EXPECT_EQ(3, i);
}

TEST_F(TagTupleTest, Call) {
    auto x = fcpp::make_tagged_tuple<tag,gat>([](int x){ return x+1; }, [](int x){ return 2*x; });
    auto y = x(3);
    auto z = fcpp::make_tagged_tuple<tag,gat>(4, 6);
    EXPECT_EQ(y, z);
}

TEST_F(TagTupleTest, Assignment) {
    int i;
    i = fcpp::get<tag>(t);
    EXPECT_EQ(2, i);
    t = fcpp::make_tagged_tuple<oth,tag>("stuff",5);
    i = fcpp::get<tag>(t);
    EXPECT_EQ(5, i);
    bool b;
    b = fcpp::get<gat>(t);
    EXPECT_EQ(true, b);
    t = fcpp::make_tagged_tuple<gat,tag>(false,3);
    i = fcpp::get<tag>(t);
    EXPECT_EQ(3, i);
    b = fcpp::get<gat>(t);
    EXPECT_EQ(false, b);
    fcpp::tagged_tuple<oth, double, tag, int> x(t);
    i = fcpp::get<tag>(x);
    EXPECT_EQ(3, i);
}

TEST_F(TagTupleTest, Types) {
    std::string ex, res;
    ex  = typeid(char).name();
    res = typeid(fcpp::tagged_tuple<tag,int,gat,char,oth,bool>::tag_type<gat>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::type_sequence<tag,gat,oth>).name();
    res = typeid(fcpp::tagged_tuple<tag,int,gat,char,oth,bool>::tags).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::type_sequence<int,char,bool>).name();
    res = typeid(fcpp::tagged_tuple<tag,int,gat,char,oth,bool>::types).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::tagged_tuple<tag,int,gat,char,oth,bool>).name();
    res = typeid(fcpp::tagged_tuple<gat,char,oth,bool>::push_front<tag,int>).name();
    EXPECT_EQ(ex, res);
    res = typeid(fcpp::tagged_tuple<tag,int,gat,char>::push_back<oth,bool>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(fcpp::tagged_tuple<tag,int,gat,char,oth,bool,hto,double>).name();
    res = typeid(fcpp::tagged_tuple_cat<
        fcpp::tagged_tuple<tag,int>,
        fcpp::tagged_tuple<gat,char,oth,bool>,
        fcpp::tagged_tuple<hto,double>
    >).name();
    EXPECT_EQ(ex, res);
}
