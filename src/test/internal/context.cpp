// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <utility>

#include "gtest/gtest.h"

#include "lib/common/multitype_map.hpp"
#include "lib/internal/context.hpp"

using namespace fcpp;


// mock metric class returning a given value and never updating
struct metric {
    metric() = default;
    
    metric(double v) : val(v) {}
    
    template <typename... Ts>
    double build(Ts const&...) const {
        return val;
    }
    
    template <typename... Ts>
    double update(const double& r, Ts const&...) const {
        return val == 0.0 ? r : val;
    }
    
  private:
    double val;
};

class ContextTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
        m.insert(7, 'a');
        m.insert(42,'+');
        m.insert(3,  details::make_field({0,6}, std::vector<int>{1,3,4}));
        m.insert(18, details::make_field({1,9}, std::vector<int>{9,2,2}));
        m.insert(8);
        data.insert(1, m, 0.5, 1.5, 9);
    }
    
    common::multitype_map<trace_t, fcpp::field<int>, char> m;
    internal::context<FCPP_ONLINE_DROP, FCPP_EXPORT_PTR, double, fcpp::field<int>, char> data;
};


TEST_F(ContextTest, Operators) {
    internal::context<FCPP_ONLINE_DROP, FCPP_EXPORT_PTR, double, fcpp::field<int>, char> x(data), y, z;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
}

#if FCPP_ONLINE_DROP
TEST_F(ContextTest, InsertErase) {
    internal::context<FCPP_ONLINE_DROP, FCPP_EXPORT_PTR, double, fcpp::field<int>, char> x(data);
    x.insert(2, m, 0.3, 1.5, 9);
    x.insert(3, m, 0.4, 1.5, 9);
    EXPECT_EQ(size_t(3), x.size(1));
    EXPECT_EQ(size_t(4), x.size(0));
    EXPECT_EQ(device_t(1), x.top());
    x.pop();
    EXPECT_EQ(device_t(3), x.top());
    x.pop();
    EXPECT_EQ(device_t(2), x.top());
    x.unfreeze(0, metric{0.5}, 1.0);
    EXPECT_EQ(device_t(2), x.top());
    x.insert(3, m, 0.4, 1.5, 9);
    EXPECT_EQ(device_t(2), x.top());
    x.pop();
    EXPECT_EQ(device_t(3), x.top());
    x.unfreeze(0, metric{1.0}, 0.5);
    EXPECT_EQ(size_t(1), x.size(9));
}
#endif

TEST_F(ContextTest, Align) {
    m.insert(9);
    data.insert(2, m, 1.0, 1.5, 9);
    data.freeze(9, 0);
    std::vector<device_t> ex, res;
    ex = std::vector<device_t>{0,1,2};
    res = data.align(8, 0);
    EXPECT_EQ(ex, res);
    ex = std::vector<device_t>{0,2};
    res = data.align(9, 0);
    EXPECT_EQ(ex, res);
    data.unfreeze(0, metric{}, 1.5);
}

TEST_F(ContextTest, Old) {
    char c;
    data.freeze(9, 0);
    c = data.old(7, 'c', 0);
    EXPECT_EQ('c', c);
    data.unfreeze(0, metric{}, 1.5);
    data.insert(0, m, 1.0, 1.5, 9);
    data.freeze(9, 0);
    c = data.old(7, 'c', 0);
    EXPECT_EQ('a', c);
    data.unfreeze(0, metric{}, 1.5);
}

TEST_F(ContextTest, Nbr) {
    m.insert(42, '-');
    m.insert(3,  details::make_field({0,5}, std::vector<int>{1,2,9}));
    m.insert(18, details::make_field({0,5}, std::vector<int>{1,3,7}));
    data.insert(2, m, 1.0, 1.5, 9);
    data.freeze(9, 0);
    fcpp::field<char> fcr, fce;
    fcr = data.nbr(42, '*', 0);
    fce = details::make_field({1,2}, std::vector<char>{'*', '+', '-'});
    EXPECT_EQ(fce, fcr);
    fcpp::field<int> fir, fie;
    fir = data.nbr(18, field<int>{-1}, 0);
    fie = details::make_field({1,2}, std::vector<int>{-1,9,3});
    EXPECT_EQ(fie, fir);
    fir = data.nbr(3, field<int>{7}, 0);
    fie = details::make_field({1,2}, std::vector<int>{7,3,2});
    EXPECT_EQ(fie, fir);
    data.unfreeze(0, metric{}, 1.5);
}
