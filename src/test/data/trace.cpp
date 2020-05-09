// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <vector>

#include "gtest/gtest.h"

#include "lib/data/trace.hpp"

using namespace fcpp;

struct public_trace : public data::trace {
    using data::trace::trace;
    using data::trace::clear;
    using data::trace::push;
    using data::trace::pop;
};

public_trace test_trace;

TEST(TraceTest, Hash) {
	EXPECT_EQ((trace_t)0, test_trace.hash(0));
	EXPECT_EQ((trace_t)12L<<k_hash_len, test_trace.hash(12));
}

TEST(TraceTest, PushPop) {
    std::vector<trace_t> stack;
    stack.push_back(test_trace.hash(0));
    test_trace.push(15);
    stack.push_back(test_trace.hash(0));
    test_trace.push(120);
    stack.push_back(test_trace.hash(0));
    test_trace.push(48);
    stack.push_back(test_trace.hash(0));
    test_trace.push(20);
    stack.push_back(test_trace.hash(0));
    test_trace.push(50);
    EXPECT_EQ(false, test_trace.empty());

    test_trace.pop();
    EXPECT_EQ(stack[4], test_trace.hash(0));
    test_trace.pop();
    EXPECT_EQ(stack[3], test_trace.hash(0));
    test_trace.pop();
    EXPECT_EQ(stack[2], test_trace.hash(0));
    test_trace.pop();
    EXPECT_EQ(stack[1], test_trace.hash(0));
    test_trace.pop();
    EXPECT_EQ(stack[0], test_trace.hash(0));
    EXPECT_EQ(true, test_trace.empty());

    EXPECT_EQ((trace_t)0, stack[0]);
    EXPECT_EQ((trace_t)15, stack[1]);
}

TEST(TraceTest, TraceCall) {
    std::vector<trace_t> stack;
    stack.push_back(test_trace.hash(0));
    {
        data::trace_call _(test_trace, 15);
        stack.push_back(test_trace.hash(0));
        {
            data::trace_call _(test_trace, 120);
            stack.push_back(test_trace.hash(0));
            {
                data::trace_call _(test_trace, 48);
            }
            EXPECT_EQ(stack[2], test_trace.hash(0));
        }
        EXPECT_EQ(stack[1], test_trace.hash(0));
    }
    EXPECT_EQ(stack[0], test_trace.hash(0));
}

TEST(TraceTest, TraceCycle) {
    std::vector<trace_t> stack;
    {
        data::trace_cycle _(test_trace);
        for (int i=0; i<10; ++i) {
            stack.push_back(test_trace.hash(0));
            ++_;
        }
    }
    for (data::trace_cycle i{test_trace, 0}; i<10; ++i) {
        EXPECT_EQ(stack[i], test_trace.hash(0));
    }
}
