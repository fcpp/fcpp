// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "test/helper.hpp"

#include "lib/common/type_sequence.hpp"

using namespace fcpp;


TEST(TypeSequenceTest, QueueOp) {
    EXPECT_SAME(short, common::type_sequence<short,int,double,double,char>::front);
    EXPECT_SAME(short, common::type_sequence<char,int,double,double,short>::back);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int,double,double,char>::push_back<>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int,double,double>::push_back<char>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int,double>::push_back<double,char>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int>::push_back<double,double,char>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<>::push_back<int,double,double,char>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<double,char>::push_front<int,double>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<short,int,double,double,char>::pop_front);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int,double,double,char,short>::pop_back);
}

TEST(TypeSequenceTest, ArrayOp) {
    int i;
    i = common::type_sequence<int,double,double,char>::size;
    EXPECT_EQ(4, i);
    i = common::type_sequence<>::size;
    EXPECT_EQ(0, i);
    EXPECT_SAME(common::type_sequence<double,char>,
                common::type_sequence<int,double,double,char>::slice<1,-1,2>);
    EXPECT_SAME(common::type_sequence<int,char>,
                common::type_sequence<int,double,double,char>::slice<0,-1,3>);
    EXPECT_SAME(common::type_sequence<double>,
                common::type_sequence<int,double,double,char>::slice<2,-1,5>);
    EXPECT_SAME(common::type_sequence<double,double>,
                common::type_sequence<int,double,double,char>::slice<1,3,1>);
    EXPECT_SAME(double, common::type_sequence<int,int,double,char>::get<2>);
}

TEST(TypeSequenceTest, SetOp) {
    EXPECT_SAME(common::type_sequence<int,char>,
                common::type_sequence<int,double,double,char>::intersect<short,char,int>);
    EXPECT_SAME(common::type_sequence<double,double>,
                common::type_sequence<int,double,double,char>::intersect<double>);
    EXPECT_SAME(common::type_sequence<double,double,char,float,int>,
                common::type_sequence<double,double,char>::unite<double,float,int,float>);
    EXPECT_SAME(common::type_sequence<void,char>,
                common::type_sequence<void,double,double,char>::subtract<double,float,int,float>);
    EXPECT_SAME(common::type_sequence<double>,
                common::type_sequence<int,double,double,char>::repeated);
    EXPECT_SAME(common::type_sequence<int,double,char>,
                common::type_sequence<int,double,double,char>::uniq);
    EXPECT_SAME(common::type_cat<common::type_sequence<int,double,char>, common::type_sequence<float>, common::type_sequence<bool,void>>,
                common::type_sequence<int,double,char,float,bool,void>::uniq);
    EXPECT_SAME(common::type_product< common::type_sequence<common::type_sequence<double,float>, common::type_sequence<char>>, common::type_sequence<common::type_sequence<>, common::type_sequence<bool>>, common::type_sequence<common::type_sequence<void>> >,
                common::type_sequence<common::type_sequence<double,float,void>, common::type_sequence<char,void>, common::type_sequence<double,float,bool,void>, common::type_sequence<char,bool,void>>);
    int i;
    i = common::type_sequence<int,double,double,char>::repeated::size;
    EXPECT_EQ(1, i);
    i = common::type_sequence<int,double,char>::repeated::size;
    EXPECT_EQ(0, i);
}

TEST(TypeSequenceTest, SearchOp) {
    int t;
    t = common::type_sequence<int,double,char>::find<int>;
    EXPECT_EQ(0, t);
    t = common::type_sequence<int,char,double>::find<double>;
    EXPECT_EQ(2, t);
    t = common::type_sequence<int,char,double>::count<double>;
    EXPECT_EQ(1, t);
    t = common::type_sequence<double,char>::count<int>;
    EXPECT_EQ(0, t);
    t = common::type_sequence<int,char,int>::count<int>;
    EXPECT_EQ(2, t);
}
