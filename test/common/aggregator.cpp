// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <sstream>

#include "gtest/gtest.h"

#include "lib/common/aggregator.hpp"


TEST(AggregatorTest, Count) {
    std::stringstream ss;
    fcpp::count_aggregator<bool> v(ss, "tag");
    EXPECT_EQ("count(tag) ", ss.str());

    EXPECT_EQ(0ULL, v.result());
    v.insert(false);
    EXPECT_EQ(0ULL, v.result());
    v.insert(true);
    EXPECT_EQ(1ULL, v.result());
    v.insert(true);
    EXPECT_EQ(2ULL, v.result());
    v.erase(true);
    EXPECT_EQ(1ULL, v.result());
    v.erase(false);
    EXPECT_EQ(1ULL, v.result());
}

TEST(AggregatorTest, Sum) {
    std::stringstream ss;
    fcpp::sum_aggregator<int> v(ss, "tag");
    EXPECT_EQ("sum(tag) ", ss.str());

    EXPECT_EQ(0, v.result());
    v.insert(3);
    EXPECT_EQ(3, v.result());
    v.insert(3);
    EXPECT_EQ(6, v.result());
    v.insert(2);
    EXPECT_EQ(8, v.result());
    v.erase(3);
    EXPECT_EQ(5, v.result());
    v.erase(2);
    EXPECT_EQ(3, v.result());
    
    fcpp::sum_aggregator<double> vd(ss, "tag");
    vd.insert(1.0/0.0);
    vd.insert(0.0/0.0);
    vd.insert(-1.0/0.0);
    EXPECT_EQ(0.0, vd.result());
    fcpp::sum_aggregator<double,false> vi(ss, "tag");
    vi.insert(1.0/0.0);
    EXPECT_EQ(1.0/0.0, vi.result());
}

TEST(AggregatorTest, Mean) {
    std::stringstream ss;
    fcpp::mean_aggregator<int> v(ss, "tag");
    EXPECT_EQ("mean(tag) ", ss.str());

    v.insert(3);
    EXPECT_EQ(3, v.result());
    v.insert(3);
    EXPECT_EQ(3, v.result());
    v.insert(6);
    EXPECT_EQ(4, v.result());
    v.erase(3);
    EXPECT_EQ(4, v.result());
    v.erase(3);
    EXPECT_EQ(6, v.result());
    
    fcpp::mean_aggregator<double> vd(ss, "tag");
    vd.insert(1.0/0.0);
    vd.insert(1.0/1.0);
    vd.insert(0.0/0.0);
    vd.insert(-1.0/0.0);
    EXPECT_EQ(1.0, vd.result());
    fcpp::mean_aggregator<double,false> vi(ss, "tag");
    vi.insert(1.0/0.0);
    EXPECT_EQ(1.0/0.0, vi.result());
}

TEST(AggregatorTest, Moment) {
    std::stringstream ss;
    fcpp::moment_aggregator<int,2> v(ss, "tag");
    EXPECT_EQ("dev(tag) ", ss.str());

    v.insert(3);
    EXPECT_EQ(3, v.result());
    v.insert(3);
    EXPECT_EQ(3, v.result());
    v.insert(6);
    EXPECT_EQ(4, v.result());
    v.erase(3);
    EXPECT_EQ(4, v.result());
    v.erase(3);
    EXPECT_EQ(6, v.result());
    
    fcpp::moment_aggregator<double,2> vd(ss, "tag");
    vd.insert(1.0/0.0);
    vd.insert(1.0/1.0);
    vd.insert(0.0/0.0);
    vd.insert(-1.0/0.0);
    EXPECT_EQ(1.0, vd.result());
    fcpp::moment_aggregator<double,2,false> vi(ss, "tag");
    vi.insert(1.0/0.0);
    EXPECT_EQ(1.0/0.0, vi.result());
}

TEST(AggregatorTest, Dev) {
    std::stringstream ss;
    fcpp::dev_aggregator<int> v(ss, "tag");
    EXPECT_EQ("mean(tag) dev(tag) ", ss.str());

    v.insert(3);
    EXPECT_EQ(std::make_tuple(3,0), v.result());
    v.insert(3);
    EXPECT_EQ(std::make_tuple(3,0), v.result());
    v.insert(6);
    EXPECT_EQ(std::make_tuple(4,1), v.result());
    v.erase(3);
    EXPECT_EQ(std::make_tuple(4,1), v.result());
    v.erase(3);
    EXPECT_EQ(std::make_tuple(6,0), v.result());
    
    fcpp::dev_aggregator<double> vd(ss, "tag");
    vd.insert(1.0/0.0);
    vd.insert(1.0/1.0);
    vd.insert(0.0/0.0);
    vd.insert(-1.0/0.0);
    EXPECT_EQ(std::make_tuple(1.0, 0.0), vd.result());
    fcpp::dev_aggregator<double,false> vi(ss, "tag");
    vi.insert(1.0/0.0);
    double m = std::get<0>(vi.result());
    double d = std::get<1>(vi.result());
    EXPECT_EQ(1.0/0.0, m);
    EXPECT_TRUE(std::isnan(d));
}

TEST(AggregatorTest, Min) {
    std::stringstream ss;
    fcpp::min_aggregator<int> v(ss, "tag");
    EXPECT_EQ("min(tag) ", ss.str());

    v.insert(3);
    EXPECT_EQ(3, v.result()[0]);
    v.insert(6);
    EXPECT_EQ(3, v.result()[0]);
    v.insert(2);
    EXPECT_EQ(2, v.result()[0]);
    v.erase(3);
    EXPECT_EQ(2, v.result()[0]);
    v.erase(2);
    EXPECT_EQ(6, v.result()[0]);
    
    fcpp::min_aggregator<double> vd(ss, "tag");
    vd.insert(1.0/0.0);
    vd.insert(1.0/1.0);
    vd.insert(0.0/0.0);
    vd.insert(-1.0/0.0);
    EXPECT_EQ(1.0, vd.result()[0]);
    fcpp::min_aggregator<double,false> vi(ss, "tag");
    vi.insert(1.0/0.0);
    EXPECT_EQ(1.0/0.0, vi.result()[0]);
}

TEST(AggregatorTest, Max) {
    std::stringstream ss;
    fcpp::max_aggregator<int> v(ss, "tag");
    EXPECT_EQ("max(tag) ", ss.str());

    v.insert(3);
    EXPECT_EQ(3, v.result()[0]);
    v.insert(6);
    EXPECT_EQ(6, v.result()[0]);
    v.insert(2);
    EXPECT_EQ(6, v.result()[0]);
    v.erase(3);
    EXPECT_EQ(6, v.result()[0]);
    v.erase(6);
    EXPECT_EQ(2, v.result()[0]);
    
    fcpp::max_aggregator<double> vd(ss, "tag");
    vd.insert(1.0/0.0);
    vd.insert(1.0/1.0);
    vd.insert(0.0/0.0);
    vd.insert(-1.0/0.0);
    EXPECT_EQ(1.0, vd.result()[0]);
    fcpp::max_aggregator<double,false> vi(ss, "tag");
    vi.insert(1.0/0.0);
    EXPECT_EQ(1.0/0.0, vi.result()[0]);
}

TEST(AggregatorTest, Quantile) {
    std::stringstream ss;
    fcpp::quantile_aggregator<double, false, 33, 66, 100> v(ss, "tag");
    EXPECT_EQ("q33(tag) q66(tag) max(tag) ", ss.str());

    v.insert(3);
    EXPECT_NEAR(3.000, v.result()[0], 0.001);
    EXPECT_NEAR(3.000, v.result()[1], 0.001);
    v.insert(4);
    EXPECT_NEAR(3.330, v.result()[0], 0.001);
    EXPECT_NEAR(3.660, v.result()[1], 0.001);
    v.insert(7);
    EXPECT_EQ(7.0, v.result()[2]);
    v.insert(8);
    EXPECT_NEAR(4.00, v.result()[0], 0.04);
    EXPECT_NEAR(7.00, v.result()[1], 0.07);
    v.erase(3);
    EXPECT_EQ(8.0, v.result()[2]);
    v.erase(4);
    EXPECT_NEAR(7.330, v.result()[0], 0.001);
    EXPECT_NEAR(7.660, v.result()[1], 0.001);
}

TEST(AggregatorTest, Multi) {
    std::stringstream ss;
    fcpp::multi_aggregator<fcpp::count_aggregator<int>, fcpp::mean_aggregator<int>> v(ss, "tag");
    EXPECT_EQ("count(tag) mean(tag) ", ss.str());
    
    int c, m;
    v.insert(3);
    c = std::get<0>(v.result());
    m = std::get<1>(v.result());
    EXPECT_EQ(1, c);
    EXPECT_EQ(3, m);
    v.insert(0);
    c = std::get<0>(v.result());
    m = std::get<1>(v.result());
    EXPECT_EQ(1, c);
    EXPECT_EQ(1, m);
    v.insert(6);
    c = std::get<0>(v.result());
    m = std::get<1>(v.result());
    EXPECT_EQ(2, c);
    EXPECT_EQ(3, m);
    v.erase(3);
    c = std::get<0>(v.result());
    m = std::get<1>(v.result());
    EXPECT_EQ(1, c);
    EXPECT_EQ(3, m);
    v.erase(6);
    c = std::get<0>(v.result());
    m = std::get<1>(v.result());
    EXPECT_EQ(0, c);
    EXPECT_EQ(0, m);
}
