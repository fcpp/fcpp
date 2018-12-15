// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include <vector>

#include "gtest/gtest.h"

#include "lib/data/trace.hpp"


TEST(TraceTest, Hash) {
	EXPECT_EQ((fcpp::trace_t)0, fcpp::thread_trace.hash<0>());
	EXPECT_EQ((fcpp::trace_t)12L<<fcpp::k_hash_len, fcpp::thread_trace.hash<12>());
}

TEST(TraceTest, PushPop) {
    std::vector<fcpp::trace_t> stack;
    stack.push_back(fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.push<15>();
    stack.push_back(fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.push<120>();
    stack.push_back(fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.push<48>();
    stack.push_back(fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.push<20>();
    stack.push_back(fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.push<50>();
    EXPECT_EQ(false, fcpp::thread_trace.empty());

    fcpp::thread_trace.pop();
    EXPECT_EQ(stack[4], fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.pop();
    EXPECT_EQ(stack[3], fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.pop();
    EXPECT_EQ(stack[2], fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.pop();
    EXPECT_EQ(stack[1], fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.pop();
    EXPECT_EQ(stack[0], fcpp::thread_trace.hash<0>());
    EXPECT_EQ(true, fcpp::thread_trace.empty());

    EXPECT_EQ((fcpp::trace_t)0, stack[0]);
    EXPECT_EQ((fcpp::trace_t)15, stack[1]);
}

TEST(TraceTest, PushPopCycle) {
    fcpp::thread_trace.push<15>();
    fcpp::thread_trace.push_cycle<120>();
    fcpp::thread_trace.push<48>();
    fcpp::thread_trace.push<20>();
    fcpp::thread_trace.push<50>();
    fcpp::thread_trace.pop_cycle();
    EXPECT_EQ((fcpp::trace_t)15, fcpp::thread_trace.hash<0>());
    fcpp::thread_trace.pop();
    EXPECT_EQ(true, fcpp::thread_trace.empty());
}
