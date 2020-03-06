// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <vector>

#include "gtest/gtest.h"

#include "lib/data/trace.hpp"

using namespace fcpp;


TEST(TraceTest, Hash) {
	EXPECT_EQ((trace_t)0, data::thread_trace.hash(0));
	EXPECT_EQ((trace_t)12L<<k_hash_len, data::thread_trace.hash(12));
}

TEST(TraceTest, PushPop) {
    std::vector<trace_t> stack;
    stack.push_back(data::thread_trace.hash(0));
    data::thread_trace.push(15);
    stack.push_back(data::thread_trace.hash(0));
    data::thread_trace.push(120);
    stack.push_back(data::thread_trace.hash(0));
    data::thread_trace.push(48);
    stack.push_back(data::thread_trace.hash(0));
    data::thread_trace.push(20);
    stack.push_back(data::thread_trace.hash(0));
    data::thread_trace.push(50);
    EXPECT_EQ(false, data::thread_trace.empty());

    data::thread_trace.pop();
    EXPECT_EQ(stack[4], data::thread_trace.hash(0));
    data::thread_trace.pop();
    EXPECT_EQ(stack[3], data::thread_trace.hash(0));
    data::thread_trace.pop();
    EXPECT_EQ(stack[2], data::thread_trace.hash(0));
    data::thread_trace.pop();
    EXPECT_EQ(stack[1], data::thread_trace.hash(0));
    data::thread_trace.pop();
    EXPECT_EQ(stack[0], data::thread_trace.hash(0));
    EXPECT_EQ(true, data::thread_trace.empty());

    EXPECT_EQ((trace_t)0, stack[0]);
    EXPECT_EQ((trace_t)15, stack[1]);
}

TEST(TraceTest, TraceCall) {
    std::vector<trace_t> stack;
    stack.push_back(data::thread_trace.hash(0));
    {
        data::trace_call _(15);
        stack.push_back(data::thread_trace.hash(0));
        {
            data::trace_call _(120);
            stack.push_back(data::thread_trace.hash(0));
            {
                data::trace_call _(48);
            }
            EXPECT_EQ(stack[2], data::thread_trace.hash(0));
        }
        EXPECT_EQ(stack[1], data::thread_trace.hash(0));
    }
    EXPECT_EQ(stack[0], data::thread_trace.hash(0));
}

TEST(TraceTest, TraceCycle) {
    std::vector<trace_t> stack;
    {
        data::trace_cycle _;
        for (int i=0; i<10; ++i) {
            stack.push_back(data::thread_trace.hash(0));
            ++_;
        }
    }
    for (data::trace_cycle i = 0; i<10; ++i) {
        EXPECT_EQ(stack[i], data::thread_trace.hash(0));
    }
}
