// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/data/vec.hpp"
#include "lib/option/metric.hpp"

using namespace fcpp;

struct tag {};

struct mock_node {
    mock_node(device_t id, times_t c, times_t n, vec<2> p) : uid(id), ct(c), nt(n), pos(p) {}

    vec<2> position(times_t) const {
        return pos;
    }
    times_t next_time() const {
        return nt;
    }
    times_t current_time() const {
        return ct;
    }

    device_t uid;
    times_t ct, nt;
    vec<2> pos;
};


TEST(MetricTest, Once) {
    using metric_type = metric::once;
    mock_node n(0, 0, 0.5f, {1.5f, 2.5f});
    metric_type m{common::make_tagged_tuple<>()};
    metric_type::result_type x = m.build(), y, z;
    y = m.build(n, 0.2f, 1, common::make_tagged_tuple<>());
    z = m.build(n, 0.2f, 0, common::make_tagged_tuple<>());
    EXPECT_LE(y, x);
    EXPECT_LE(z, x);
    y = m.update(y, n);
    z = m.update(z, n);
    EXPECT_GT(y, x);
    EXPECT_LE(z, x);
}

TEST(MetricTest, Retain) {
    using metric_type = metric::retain<>;
    mock_node n(0, 0, 0.5f, {1.5f, 2.5f});
    metric_type m{common::make_tagged_tuple<>()};
    metric_type::result_type x = m.build(), y, z;
    y = m.build(n, 0.2f, 1, common::make_tagged_tuple<>());
    z = m.build(n, 0.2f, 0, common::make_tagged_tuple<>());
    EXPECT_LE(y, x);
    EXPECT_LE(z, x);
    y = m.update(y, n);
    z = m.update(z, n);
    EXPECT_LE(y, x);
    EXPECT_LE(z, x);
    n.ct = n.nt;
    n.nt += 1;
    y = m.update(y, n);
    z = m.update(z, n);
    EXPECT_GT(y, x);
    EXPECT_LE(z, x);
}

TEST(MetricTest, VariableRetain) {
    using metric_type = metric::retain<1, 10, tag>;
    mock_node n(0, 0, 0.5f, {1.5f, 2.5f});
    metric_type m{common::make_tagged_tuple<tag>(1)};
    metric_type::result_type x = m.build(), y, z;
    y = m.build(n, 0.2f, 1, common::make_tagged_tuple<>());
    z = m.build(n, 0.2f, 0, common::make_tagged_tuple<>());
    EXPECT_LE(y, x);
    EXPECT_LE(z, x);
    y = m.update(y, n);
    z = m.update(z, n);
    EXPECT_LE(y, x);
    EXPECT_LE(z, x);
    n.ct = n.nt;
    n.nt += 1;
    y = m.update(y, n);
    z = m.update(z, n);
    EXPECT_GT(y, x);
    EXPECT_LE(z, x);
}

TEST(MetricTest, Minkowski) {
    using metric_type = metric::minkowski<tag>;
    mock_node n(0, 0, 0.5f, {1.5f, 2.5f});
    metric_type m{common::make_tagged_tuple<>()};
    metric_type::result_type x = m.build(), y, z;
    y = m.build(n, 0.2f, 1, common::make_tagged_tuple<tag>(make_vec(2, 2)));
    z = m.build(n, 0.2f, 0, common::make_tagged_tuple<tag>(make_vec(1.5f, 2.5f)));
    EXPECT_LE(y, x);
    EXPECT_LE(z, x);
    y = m.update(y, n);
    z = m.update(z, n);
    EXPECT_LE(y, x);
    EXPECT_LE(z, x);
    n.ct = n.nt;
    n.nt += 1;
    y = m.update(y, n);
    z = m.update(z, n);
    EXPECT_GT(y, x);
    EXPECT_LE(z, x);
}
