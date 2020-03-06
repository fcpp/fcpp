// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <unordered_set>
#include <utility>

#include "gtest/gtest.h"

#include "lib/common/multitype_map.hpp"
#include "lib/data/context.hpp"

using namespace fcpp;


class ContextTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
        m.insert(7, 'a');
        m.insert(42,'+');
        m.insert(3,  fcpp::details::make_field(1,{{0,3}, {6,4}}));
        m.insert(18, fcpp::details::make_field(9,{{1,2}, {9,2}}));
        m.insert(8);
        data.insert(1, m, 0.5);
    }
    
    common::multitype_map<trace_t, fcpp::field<int>, char> m;
    data::context<double, fcpp::field<int>, char> data{0};
};


TEST_F(ContextTest, Operators) {
    data::context<double, fcpp::field<int>, char> x(data), y(-1), z(-1);
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
}

TEST_F(ContextTest, InsertErase) {
    data::context<double, fcpp::field<int>, char> x(data);
    x.insert(2, m, 0.3);
    EXPECT_EQ(device_t(0), x.self());
    EXPECT_EQ(size_t(3), x.size());
    EXPECT_EQ(0.5, x.top());
    for (const auto& p : x.data())
        EXPECT_EQ(*(p.second), m);
    x.insert(2, 1.0);
    EXPECT_EQ(size_t(3), x.size());
    EXPECT_EQ(1.0, x.top());
    x.pop();
    EXPECT_EQ(data, x);
    EXPECT_EQ(size_t(2), x.size());
    EXPECT_EQ(0.5, x.top());
    x.insert(0, m, 2.0);
    EXPECT_EQ(size_t(2), x.size());
    EXPECT_EQ(2.0, x.top());
}

TEST_F(ContextTest, Align) {
    m.insert(9);
    data.insert(2, m, 1.0);
    std::unordered_set<device_t> ex, res;
    ex = std::unordered_set<device_t>{0,1,2};
    res = data.align(8);
    EXPECT_EQ(ex, res);
    ex = std::unordered_set<device_t>{0,2};
    res = data.align(9);
    EXPECT_EQ(ex, res);
}

TEST_F(ContextTest, Old) {
    char c;
    c = data.old(7, 'c');
    EXPECT_EQ('c', c);
    data.insert(0, m, 1.0);
    c = data.old(7, 'c');
    EXPECT_EQ('a', c);
}

TEST_F(ContextTest, Nbr) {
    m.insert(42, '-');
    m.insert(3,  fcpp::details::make_field(1, {{0,2}, {5,9}}));
    m.insert(18, fcpp::details::make_field(1, {{0,3}, {5,7}}));
    data.insert(2, m, 1.0);
    fcpp::field<char> fcr, fce;
    fcr = data.template nbr<char>(42, '*');
    fce = fcpp::details::make_field('*', {{1,'+'}, {2,'-'}});
    EXPECT_EQ(fce, fcr);
    fcpp::field<int> fir, fie;
    fir = data.template nbr<fcpp::field<int>>(18, -1);
    fie = fcpp::details::make_field(-1, {{1,9}, {2,3}});
    EXPECT_EQ(fie, fir);
    fir = data.template nbr<fcpp::field<int>>(3, 7);
    fie = fcpp::details::make_field(7, {{1,3}, {2,2}});
    EXPECT_EQ(fie, fir);
}
