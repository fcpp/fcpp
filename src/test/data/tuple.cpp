// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/data/tuple.hpp"

using namespace fcpp;


TEST(TupleTest, Constructors) {
    tuple<int,bool> t{2, false};
    tuple<int,bool> u{t};
    tuple<double,int> s{t};
    s = u;
    u = std::move(t);
    t = u;
    swap(t, u);
    EXPECT_EQ(get<0>(t), get<0>(u));
    EXPECT_EQ(get<1>(t), get<1>(u));
}

TEST(TupleTest, Makers) {
    tuple<int,bool> t = make_tuple(42, true);
    tuple<int,bool> u = forward_as_tuple(get<0>(t), get<1>(t));
    int  i;
    bool b;
    tie(i, b) = u;
    EXPECT_EQ(42, i);
    EXPECT_EQ(true, b);
}

TEST(TupleTest, TupleCat) {
    tuple<int,double> t{10,4.5};
    tuple<bool,char> u{false,'x'};
    tuple<> w = make_tuple();
    tuple<int,double,bool,char> s = tuple_cat(t, u, w);
    EXPECT_EQ(10,    get<0>(s));
    EXPECT_EQ(4.5,   get<1>(s));
    EXPECT_EQ(false, get<2>(s));
    EXPECT_EQ('x',   get<3>(s));
}

TEST(TupleTest, Operators) {
    tuple<int,double> t{5,6};
    tuple<int,double> s = -t;
    s = -tuple<int,double>{t};
    EXPECT_EQ(-5,   get<0>(s));
    EXPECT_EQ(-6.0, get<1>(s));
    s += t;
    s += tuple<int,double>{t};
    EXPECT_EQ(5,   get<0>(s));
    EXPECT_EQ(6.0, get<1>(s));
    tuple<int,double> u;
    u = s + t;
    u = s + tuple<int,double>{t};
    u = tuple<int,double>{s} + t;
    u = tuple<int,double>{s} + tuple<int,double>{t};
    EXPECT_EQ(10,   get<0>(u));
    EXPECT_EQ(12.0, get<1>(u));
    auto x = (s <= u);
    EXPECT_EQ(true, get<0>(x));
    EXPECT_EQ(true, get<1>(x));
    EXPECT_TRUE(x);
}

TEST(TupleTest, NestedTuples) {
    tuple<tuple<int,char>,bool> s{{2,1},false}, t{{3,4},true}, u, x;
    u = {{1,5},true};
    x = make_tuple(tuple<int,char>{2,'a'}, true);
    x = s & t;
    EXPECT_EQ(2,        get<0>(get<0>(x)));
    EXPECT_EQ(char(0),  get<1>(get<0>(x)));
    EXPECT_EQ(false,    get<1>(x));
    x |= u;
    EXPECT_EQ(3,        get<0>(get<0>(x)));
    EXPECT_EQ(char(5),  get<1>(get<0>(x)));
    EXPECT_EQ(true,     get<1>(x));
    x = t ^ std::move(u);
    EXPECT_EQ(2,        get<0>(get<0>(x)));
    EXPECT_EQ(char(1),  get<1>(get<0>(x)));
    EXPECT_EQ(false,    get<1>(x));
    x = !x;
    EXPECT_EQ(char(0),  get<0>(get<0>(x)));
    EXPECT_EQ(char(0),  get<1>(get<0>(x)));
    EXPECT_EQ(true,     get<1>(x));
    x = make_tuple(~make_tuple(7,5), false);
    EXPECT_EQ(char(250),get<1>(get<0>(x)));
    EXPECT_EQ(false,    get<1>(x));
}

TEST(TupleTest, UnaryTuples) {
    tuple<int> t{3};
    tuple<tuple<int>> u{{2}};
    tuple<tuple<double>> s{{3}};
    u = tuple<tuple<long long>>{s};
    EXPECT_EQ(3, get<0>(get<0>(u)));
    u = t;
    EXPECT_EQ(3, get<0>(get<0>(u)));
}
