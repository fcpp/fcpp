// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include <unordered_set>
#include <utility>

#include "gtest/gtest.h"

#include "lib/component/calculus.hpp"

using namespace fcpp;


constexpr fcpp::trace_t time_id = 42;

template <class... Ts>
struct time_metric {
    typedef double result_type;
    typedef common::multitype_map<fcpp::trace_t, Ts...> export_type;
    static result_type metric(const export_type& self, const export_type& other) {
        return self.template at<double>(time_id) - other.template at<double>(time_id);
    }
};

typedef fcpp::device<time_metric, fcpp::field<int>, double> device_type;


class DeviceTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
        m.insert(7, 0.2);
        m.insert(42,0.7);
        m.insert(3,  fcpp::details::make_field(1,{{0,3}, {6,4}}));
        m.insert(18, fcpp::details::make_field(9,{{1,2}, {9,2}}));
        m.insert(8);
        device_type::export_type start;
        start.second().insert(42, 1.0);
        data.round_start(start);
        data.insert(1, m);
    }
    
    device_type::context_type::value_type m;
    device_type data{0, 2, 3.0};
};


TEST_F(DeviceTest, Operators) {
    device_type x(data), y(1, 1, 1.0), z(1, 1, 1.0);
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(data, z);
}
