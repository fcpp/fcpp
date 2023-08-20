// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include <sstream>

#include "gtest/gtest.h"

#include "lib/option/aggregator.hpp"
#include "test/helper.hpp"

using namespace fcpp;

struct tag {};


TEST(AggregatorTest, Count) {
    using aggr_t = aggregator::count<bool>;
    using res_t = aggr_t::result_type<tag>;
    using tag_t = res_t::tags::front;
    std::stringstream ss;
    aggr_t v;
    v.header(ss, "tag");
    EXPECT_EQ("count(tag) ", ss.str());

    EXPECT_EQ(0ULL, common::get<tag_t>(v.result<tag>()));
    v.insert(false);
    EXPECT_EQ(0ULL, common::get<tag_t>(v.result<tag>()));
    v.insert(true);
    EXPECT_EQ(1ULL, common::get<tag_t>(v.result<tag>()));
    v.insert(true);
    EXPECT_EQ(2ULL, common::get<tag_t>(v.result<tag>()));
    v.erase(true);
    EXPECT_EQ(1ULL, common::get<tag_t>(v.result<tag>()));
    v.erase(false);
    EXPECT_EQ(1ULL, common::get<tag_t>(v.result<tag>()));
}

TEST(AggregatorTest, Distinct) {
    using aggr_t = aggregator::distinct<std::string>;
    using res_t = aggr_t::result_type<tag>;
    using tag_t = res_t::tags::front;
    std::stringstream ss;
    aggr_t v;
    v.header(ss, "tag");
    EXPECT_EQ("distinct(tag) ", ss.str());

    EXPECT_EQ(0ULL, common::get<tag_t>(v.result<tag>()));
    v.insert("hello");
    EXPECT_EQ(1ULL, common::get<tag_t>(v.result<tag>()));
    v.insert("world");
    EXPECT_EQ(2ULL, common::get<tag_t>(v.result<tag>()));
    v.insert("world");
    EXPECT_EQ(2ULL, common::get<tag_t>(v.result<tag>()));
    v.erase("hello");
    EXPECT_EQ(1ULL, common::get<tag_t>(v.result<tag>()));
    v.erase("world");
    EXPECT_EQ(1ULL, common::get<tag_t>(v.result<tag>()));
    v.erase("world");
    EXPECT_EQ(0ULL, common::get<tag_t>(v.result<tag>()));
}

TEST(AggregatorTest, List) {
    using aggr_t = aggregator::list<int>;
    using res_t = aggr_t::result_type<tag>;
    using tag_t = res_t::tags::front;
    std::stringstream ss;
    aggr_t v;
    v.header(ss, "tag");
    EXPECT_EQ("list(tag) ", ss.str());

    aggr_t vs[4];
    for (int t = 0; t < 4; ++t)
        for (int i = t; i < 7; i += 4)
            vs[t].insert(i*i);
    for (int t = 0; t < 4; ++t)
        v += vs[t];
    std::vector<int> r = {0, 1, 4, 9, 16, 25, 36};
    EXPECT_EQ(r, common::get<tag_t>(v.result<tag>()));
}

TEST(AggregatorTest, Sum) {
    {
        using aggr_t = aggregator::sum<int>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("sum(tag) ", ss.str());

        EXPECT_EQ(0, common::get<tag_t>(v.result<tag>()));
        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(3);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
        v.insert(2);
        EXPECT_EQ(8, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(5, common::get<tag_t>(v.result<tag>()));
        v.erase(2);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::sum<double>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(0.0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::sum<double>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        EXPECT_EQ(INF, common::get<tag_t>(v.result<tag>()));
    }
}

TEST(AggregatorTest, Mean) {
    {
        using aggr_t = aggregator::mean<int>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("mean(tag) ", ss.str());

        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(6);
        EXPECT_EQ(4, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(4, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::mean<double>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        v.insert(1.0);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(1.0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::mean<double>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        EXPECT_EQ(INF, common::get<tag_t>(v.result<tag>()));
    }
}

TEST(AggregatorTest, Moment) {
    {
        using aggr_t = aggregator::moment<int,2>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("moment2(tag) ", ss.str());

        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(6);
        EXPECT_EQ(4, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(4, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::moment<double,2>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        v.insert(1.0);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(1.0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::moment<double,2>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        EXPECT_EQ(INF, common::get<tag_t>(v.result<tag>()));
    }
}

TEST(AggregatorTest, Dev) {
    {
        using aggr_t = aggregator::deviation<int>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("dev(tag) ", ss.str());

        v.insert(3);
        EXPECT_EQ(0, common::get<tag_t>(v.result<tag>()));
        v.insert(3);
        EXPECT_EQ(0, common::get<tag_t>(v.result<tag>()));
        v.insert(6);
        EXPECT_EQ(1, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(1, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::deviation<double>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        v.insert(1.0);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(0.0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::deviation<double>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        EXPECT_TRUE(std::isnan(common::get<tag_t>(v.result<tag>())));
    }
}

TEST(AggregatorTest, Stats) {
    {
        using aggr_t = aggregator::stats<int>;
        using res_t = aggr_t::result_type<tag>;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("mean(tag) dev(tag) ", ss.str());

        v.insert(3);
        EXPECT_EQ(res_t(3,0), v.result<tag>());
        v.insert(3);
        EXPECT_EQ(res_t(3,0), v.result<tag>());
        v.insert(6);
        EXPECT_EQ(res_t(4,1), v.result<tag>());
        v.erase(3);
        EXPECT_EQ(res_t(4,1), v.result<tag>());
        v.erase(3);
        EXPECT_EQ(res_t(6,0), v.result<tag>());
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::stats<double>>;
        using res_t = aggr_t::result_type<tag>;
        aggr_t v;
        v.insert(INF);
        v.insert(1.0);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(res_t(1.0, 0.0), v.result<tag>());
    }
    {
        using aggr_t = aggregator::stats<double>;
        aggr_t v;
        v.insert(INF);
        double m = std::get<0>(v.result<tag>());
        double d = std::get<1>(v.result<tag>());
        EXPECT_EQ(INF, m);
        EXPECT_TRUE(std::isnan(d));
    }
}

TEST(AggregatorTest, Min) {
    {
        using aggr_t = aggregator::min<int>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("min(tag) ", ss.str());

        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(6);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(2);
        EXPECT_EQ(2, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::min<double>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        v.insert(1.0);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(1.0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::min<double>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        EXPECT_EQ(INF, common::get<tag_t>(v.result<tag>()));
    }
}

TEST(AggregatorTest, Max) {
    {
        using aggr_t = aggregator::max<int>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("max(tag) ", ss.str());

        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(6);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
        v.insert(2);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::max<double>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        v.insert(1.0);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(1.0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::max<double>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        EXPECT_EQ(INF, common::get<tag_t>(v.result<tag>()));
    }
}

TEST(AggregatorTest, Minimum) {
    {
        using aggr_t = aggregator::minimum<int>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("min(tag) ", ss.str());

        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(6);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(2);
        EXPECT_EQ(2, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(2, common::get<tag_t>(v.result<tag>()));
        v.erase(2);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::minimum<double>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        v.insert(1.0);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(1.0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::minimum<double>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        EXPECT_EQ(INF, common::get<tag_t>(v.result<tag>()));
    }
}

TEST(AggregatorTest, Maximum) {
    {
        using aggr_t = aggregator::maximum<int>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("max(tag) ", ss.str());

        v.insert(3);
        EXPECT_EQ(3, common::get<tag_t>(v.result<tag>()));
        v.insert(6);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
        v.insert(2);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
        v.erase(3);
        EXPECT_EQ(6, common::get<tag_t>(v.result<tag>()));
        v.erase(6);
        EXPECT_EQ(2, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::maximum<double>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        v.insert(1.0);
        v.insert(NAN);
        v.insert(-INF);
        EXPECT_EQ(1.0, common::get<tag_t>(v.result<tag>()));
    }
    {
        using aggr_t = aggregator::maximum<double>;
        using res_t = aggr_t::result_type<tag>;
        using tag_t = res_t::tags::front;
        aggr_t v;
        v.insert(INF);
        EXPECT_EQ(INF, common::get<tag_t>(v.result<tag>()));
    }
}

TEST(AggregatorTest, Quantile) {
    {
        using aggr_t = aggregator::quantile<double, false, 33, 66, 100>;
        using res_t = aggr_t::result_type<tag>;
        using tag_33 = res_t::tags::get<0>;
        using tag_66 = res_t::tags::get<1>;
        using tag_00 = res_t::tags::get<2>;
        EXPECT_SAME(tag_00, aggregator::quantile<tag, false, 100>);
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("q33(tag) q66(tag) max(tag) ", ss.str());

        v.insert(3);
        EXPECT_NEAR(3.000, common::get<tag_33>(v.result<tag>()), 0.001);
        EXPECT_NEAR(3.000, common::get<tag_66>(v.result<tag>()), 0.001);
        v.insert(4);
        EXPECT_NEAR(3.330, common::get<tag_33>(v.result<tag>()), 0.001);
        EXPECT_NEAR(3.660, common::get<tag_66>(v.result<tag>()), 0.001);
        v.insert(7);
        EXPECT_EQ(7.0, common::get<tag_00>(v.result<tag>()));
        v.insert(8);
        EXPECT_NEAR(4.00, common::get<tag_33>(v.result<tag>()), 0.04);
        EXPECT_NEAR(7.00, common::get<tag_66>(v.result<tag>()), 0.07);
        v.erase(3);
        EXPECT_EQ(8.0, common::get<tag_00>(v.result<tag>()));
        v.erase(4);
        EXPECT_NEAR(7.330, common::get<tag_33>(v.result<tag>()), 0.001);
        EXPECT_NEAR(7.660, common::get<tag_66>(v.result<tag>()), 0.001);
    }
    {
        using aggr_t = aggregator::only_finite<aggregator::quantile<double, true, 33, 66, 100>>;
        using res_t = aggr_t::result_type<tag>;
        using tag_33 = res_t::tags::get<0>;
        using tag_66 = res_t::tags::get<1>;
        using tag_00 = res_t::tags::get<2>;
        EXPECT_SAME(tag_00, aggregator::only_finite<aggregator::quantile<tag, true, 100>>);
        std::stringstream ss;
        aggr_t v;
        v.header(ss, "tag");
        EXPECT_EQ("q33(tag) q66(tag) max(tag) ", ss.str());

        v.insert(3);
        EXPECT_NEAR(3.000, common::get<tag_33>(v.result<tag>()), 0.001);
        EXPECT_NEAR(3.000, common::get<tag_66>(v.result<tag>()), 0.001);
        v.insert(4);
        EXPECT_NEAR(3.330, common::get<tag_33>(v.result<tag>()), 0.001);
        EXPECT_NEAR(3.660, common::get<tag_66>(v.result<tag>()), 0.001);
        v.insert(7);
        EXPECT_EQ(7.0, common::get<tag_00>(v.result<tag>()));
        v.insert(8);
        EXPECT_NEAR(4.00, common::get<tag_33>(v.result<tag>()), 0.04);
        EXPECT_NEAR(7.00, common::get<tag_66>(v.result<tag>()), 0.07);
        EXPECT_EQ(8.0, common::get<tag_00>(v.result<tag>()));
    }
}

TEST(AggregatorTest, Multi) {
    using aggr_t = aggregator::combine<aggregator::count<int>, aggregator::mean<int>>;
    using res_t = aggr_t::result_type<tag>;
    std::stringstream ss;
    aggr_t v, w;
    v.header(ss, "tag");
    EXPECT_EQ("count(tag) mean(tag) ", ss.str());

    v.insert(3);
    EXPECT_EQ(res_t(1,3), v.result<tag>());
    v.insert(0);
    EXPECT_EQ(res_t(1,1), v.result<tag>());
    v.insert(6);
    EXPECT_EQ(res_t(2,3), v.result<tag>());
    v.erase(3);
    EXPECT_EQ(res_t(1,3), v.result<tag>());
    v.erase(6);
    EXPECT_EQ(res_t(0,0), v.result<tag>());
    v += w;
}

TEST(AggregatorTest, Mapper) {
    struct plus10 {
        int operator()(int x) {
            return x+10;
        }
    };
    using aggr_t = aggregator::mapper<plus10, aggregator::sum<int>>;
    using res_t = aggr_t::result_type<tag>;
    using tag_t = res_t::tags::front;
    std::stringstream ss;
    aggr_t v;
    v.insert(3);
    EXPECT_EQ(13, common::get<tag_t>(v.result<tag>()));
    v.insert(6);
    EXPECT_EQ(29, common::get<tag_t>(v.result<tag>()));
    v.insert(2);
    EXPECT_EQ(41, common::get<tag_t>(v.result<tag>()));
}
