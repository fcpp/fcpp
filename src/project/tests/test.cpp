#include "gtest/gtest.h"
#include "project/tests/library.hpp"

TEST(TagTupleTest, Print) {
    push_back<void,int,bool>();
    push_back<>();
    push_back<char,char>();
    EXPECT_EQ(2, pop_back());
    EXPECT_EQ(0, pop_back());
    EXPECT_EQ(3, pop_back());
}
