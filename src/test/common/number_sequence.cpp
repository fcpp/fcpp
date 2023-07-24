// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "test/helper.hpp"

#include "lib/common/number_sequence.hpp"

using namespace fcpp;

TEST(NumberSequenceTest, QueueOp) {
    int i;
    i = common::number_sequence<1,2,4,4,0>::front;
    EXPECT_EQ(1, i);
    i = common::number_sequence<0,2,4,4,1>::back;
    EXPECT_EQ(1, i);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2,4,4,0>::push_back<>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2,4,4>::push_back<0>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2,4>::push_back<4,0>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2>::push_back<4,4,0>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<>::push_back<2,4,4,0>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<4,0>::push_front<2,4>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<1,2,4,4,0>::pop_front);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2,4,4,0,1>::pop_back);
}

TEST(NumberSequenceTest, ArrayOp) {
    int i;
    i = common::number_sequence<2,4,4,0>::size;
    EXPECT_EQ(4, i);
    i = common::number_sequence<>::size;
    EXPECT_EQ(0, i);
    EXPECT_SAME(common::number_sequence<4,0>,
                common::number_sequence<2,4,4,0>::slice<1,-1,2>);
    EXPECT_SAME(common::number_sequence<2,0>,
                common::number_sequence<2,4,4,0>::slice<0,-1,3>);
    EXPECT_SAME(common::number_sequence<4>,
                common::number_sequence<2,4,4,0>::slice<2,-1,5>);
    EXPECT_SAME(common::number_sequence<4,4>,
                common::number_sequence<2,4,4,0>::slice<1,3,1>);
    i = common::number_sequence<2,2,4,0>::get<2>;
    EXPECT_EQ(4, i);
}

TEST(NumberSequenceTest, SetOp) {
    EXPECT_SAME(common::number_sequence<4,1>,
                common::number_sequence<4,8,8,1>::intersect<3,1,4>);
    EXPECT_SAME(common::number_sequence<8,8>,
                common::number_sequence<4,8,8,1>::intersect<8>);
    EXPECT_SAME(common::number_sequence<8,8,1,6,4>,
                common::number_sequence<8,8,1>::unite<8,6,4,6>);
    EXPECT_SAME(common::number_sequence<0,1>,
                common::number_sequence<0,8,8,1>::subtract<8,6,4,6>);
    EXPECT_SAME(common::number_sequence<8>,
                common::number_sequence<4,8,8,1>::repeated);
    EXPECT_SAME(common::number_sequence<4,8,1>,
                common::number_sequence<4,8,8,1>::uniq);
    EXPECT_SAME(common::number_cat<common::number_sequence<4,8,1>, common::number_sequence<6>, common::number_sequence<2,0>>,
                common::number_sequence<4,8,1,6,2,0>::uniq);
    int i;
    i = common::number_sequence<4,8,8,1>::repeated::size;
    EXPECT_EQ(1, i);
    i = common::number_sequence<4,8,1>::repeated::size;
    EXPECT_EQ(0, i);
}

TEST(NumberSequenceTest, SearchOp) {
    int i;
    i = common::number_sequence<2,4,0>::find<2>;
    EXPECT_EQ(0, i);
    i = common::number_sequence<2,0,4>::find<4>;
    EXPECT_EQ(2, i);
    i = common::number_sequence<2,0,4>::count<4>;
    EXPECT_EQ(1, i);
    i = common::number_sequence<4,0>::count<2>;
    EXPECT_EQ(0, i);
    i = common::number_sequence<2,0,2>::count<2>;
    EXPECT_EQ(2, i);
}

TEST(NumberSequenceTest, BoolOp) {
    bool val;
    val = common::number_sequence<>::all_true;
    EXPECT_TRUE(val);
    val = common::number_sequence<true, true, true>::all_true;
    EXPECT_TRUE(val);
    val = common::number_sequence<true, false>::all_true;
    EXPECT_FALSE(val);
    val = common::number_sequence<>::all_false;
    EXPECT_TRUE(val);
    val = common::number_sequence<false, false, false>::all_false;
    EXPECT_TRUE(val);
    val = common::number_sequence<true, false>::all_false;
    EXPECT_FALSE(val);
    val = common::number_sequence<>::some_true;
    EXPECT_FALSE(val);
    val = common::number_sequence<false, false, false>::some_true;
    EXPECT_FALSE(val);
    val = common::number_sequence<true, false>::some_true;
    EXPECT_TRUE(val);
    val = common::number_sequence<>::some_false;
    EXPECT_FALSE(val);
    val = common::number_sequence<true, true, true>::some_false;
    EXPECT_FALSE(val);
    val = common::number_sequence<true, false>::some_false;
    EXPECT_TRUE(val);
}
