// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include <unordered_map>
#include <utility>

#include "gtest/gtest.h"

#include "lib/common/immutable_map.hpp"

using namespace fcpp;


TEST(ImmutableMapTest, Operators) {
    common::immutable_map<int, std::string> x{{4, "foo"}, {2, "bar"}}, y, z, a, b;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
    EXPECT_EQ(a, b);
    swap(z, a);
    EXPECT_EQ(x, a);
    EXPECT_EQ(z, b);
    b = {{4, "foo"}, {2, "bar"}};
    EXPECT_EQ(x, b);
}

TEST(ImmutableMapTest, Insert) {
    common::immutable_map<int, std::string> x, y{{3, "baz"}}, z, w;
    x.emplace(4, "foo");
    x.insert({2, "bar"});
    x.insert(y.begin(), y.end());
    for (auto& t : x)
        EXPECT_LT(t.first, 5);
    z = {{2, "bar"}, {3, "baz"}, {4, "foo"}};
    EXPECT_NE(x, z);
    x.freeze();
    EXPECT_EQ(x, z);
    EXPECT_NE(x, w);
    x.clear();
    EXPECT_EQ(x, w);
}

TEST(ImmutableMapTest, Query) {
    common::immutable_map<int, std::string> x = {{2, "bar"}, {3, "baz"}, {4, "foo"}};
    x.freeze();
    EXPECT_EQ(x.count(1), 0ULL);
    EXPECT_EQ(x.count(3), 1ULL);
    EXPECT_EQ(x.count(5), 0ULL);
    auto it = x.find(3);
    EXPECT_EQ(it->first, 3);
    EXPECT_EQ(it->second, "baz");
    EXPECT_EQ(x.at(4), "foo");
    x.at(4) = "oof";
    EXPECT_EQ(x.at(4), "oof");
}
