// Copyright © 2017 Roberto Poletti. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/data/trace.hpp"


class TraceTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }
    
    fcpp::trace data;
};


TEST_F(TraceTest, Hash) {
	EXPECT_EQ(0L, data.hash(0));
	EXPECT_EQ(12L<<48, data.hash(12));
}

TEST_F(TraceTest, PushPop) {
	data.push(15);
	EXPECT_EQ(15L, data.hash(0));
	data.push(120);
	EXPECT_EQ(225165L, data.hash(0));
	data.push(48);
	EXPECT_EQ(3378150543L, data.hash(0));
	data.push(20);
	EXPECT_EQ(50682392596649L, data.hash(0));
	data.push(50);
	EXPECT_EQ(124024032043141L, data.hash(0));

	data.pop();
	EXPECT_EQ(50682392596649L, data.hash(0));
	data.pop();
	EXPECT_EQ(3378150543L, data.hash(0));
	data.pop();
	EXPECT_EQ(225165L, data.hash(0));
	data.pop();
	EXPECT_EQ(15L, data.hash(0));
	data.pop();
	EXPECT_EQ(0L, data.hash(0));
}

TEST_F(TraceTest, PushPopCycle) {
	data.push(15);
	EXPECT_EQ(15L, data.hash(0));
	data.push_cycle(120);
	EXPECT_EQ(225165L, data.hash(0));
	data.push(48);
	EXPECT_EQ(3378150543L, data.hash(0));
	data.push(20);
	EXPECT_EQ(50682392596649L, data.hash(0));
	data.push(50);
	EXPECT_EQ(124024032043141L, data.hash(0));
	data.pop_cycle();
	EXPECT_EQ(15L, data.hash(0));
}
