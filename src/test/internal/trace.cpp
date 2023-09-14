// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#define FCPP_WARNING_TRACE false

#include <string>
#include <vector>

#include "gtest/gtest.h"

#include "lib/internal/trace.hpp"

using namespace fcpp;

struct public_trace : public internal::trace {
    using internal::trace::trace;
    using internal::trace::clear;
    using internal::trace::push;
    using internal::trace::pop;
};

public_trace test_trace;

template <typename T>
size_t dohash(T const& x) {
    std::hash<T> h;
    return h(x);
}

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
        internal::trace_call _(test_trace, 15);
        stack.push_back(test_trace.hash(0));
        {
            internal::trace_call _(test_trace, 120);
            stack.push_back(test_trace.hash(0));
            {
                internal::trace_call _(test_trace, 48);
            }
            EXPECT_EQ(stack[2], test_trace.hash(0));
        }
        EXPECT_EQ(stack[1], test_trace.hash(0));
    }
    EXPECT_EQ(stack[0], test_trace.hash(0));
}

TEST(TraceTest, TraceKey) {
    std::vector<trace_t> stack;
    stack.push_back(test_trace.hash(0));
    {
        internal::trace_key _(test_trace, dohash(std::string("foo")));
        stack.push_back(test_trace.hash(0));
        {
            internal::trace_key _(test_trace, dohash(120));
            stack.push_back(test_trace.hash(0));
            {
                internal::trace_key _(test_trace, dohash(std::string("bar")));
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
        internal::trace_cycle _(test_trace);
        for (int i=0; i<10; ++i) {
            stack.push_back(test_trace.hash(0));
            ++_;
        }
    }
    for (internal::trace_cycle i{test_trace, 0}; i<10; ++i) {
        EXPECT_EQ(stack[i], test_trace.hash(0));
    }
}
