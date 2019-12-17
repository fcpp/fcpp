// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <string>
#include <typeinfo>

#include "gtest/gtest.h"

#include "lib/device/data_component.hpp"


struct tag {};
struct gat {};
struct oth {};
struct hto {};

template <typename T>
struct expose_storage : public T {
    using T::T;
    using T::storage;
};

template <typename T>
using comp1 = fcpp::storage_component<fcpp::tagged_tuple<T,int>>;

using csmall = expose_storage<comp1<tag>>;


TEST(ComponentTest, Operators) {
    csmall  x;
    csmall y(x);
    csmall z(fcpp::make_tagged_tuple<tag,hto,void>(3,'v',2.5));
    int i;
    i = z.storage<tag>();
    EXPECT_EQ(3, i);
    x.storage<tag>() = 5;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
    i = z.storage<tag>();
    EXPECT_EQ(5, i);
}

TEST(ComponentTest, StorageManager) {
    csmall::manager m;
    fcpp::times_t t, inf = std::numeric_limits<fcpp::times_t>::max();
    t = m.next();
    EXPECT_EQ(inf, t);
    t = m.update();
    EXPECT_EQ(inf, t);
    t = m.update();
    EXPECT_EQ(inf, t);
    std::string ex, res;
    ex  = typeid(fcpp::tagged_tuple<>).name();
    res = typeid(csmall::message_t).name();
    EXPECT_EQ(ex, res);
}

TEST(ComponentTest, Functions) {
    csmall x;
    csmall::manager m;
    x.round_start(m);
    csmall::message_t f = x.round_end(m);
    x.insert(m, f);
}
