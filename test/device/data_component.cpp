// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <random>
#include <string>
#include <typeinfo>

#include "gtest/gtest.h"

#include "lib/common/sequence.hpp"
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

using cbig = fcpp::exporter_component<tag, void, void, csmall>;


TEST(ComponentTest, StorageOperators) {
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
    std::mt19937 rnd(42);
    csmall::manager m(rnd);
    fcpp::times_t t, inf = std::numeric_limits<fcpp::times_t>::max();
    t = m.next(rnd);
    EXPECT_EQ(inf, t);
    t = m.update(rnd);
    EXPECT_EQ(inf, t);
    t = m.update(rnd);
    EXPECT_EQ(inf, t);
    std::string ex, res;
    ex  = typeid(fcpp::tagged_tuple<>).name();
    res = typeid(csmall::message_t).name();
    EXPECT_EQ(ex, res);
}

TEST(ComponentTest, StorageFunctions) {
    std::mt19937 rnd(42);
    csmall x;
    csmall::manager m(rnd);
    x.round_start(m);
    csmall::message_t f = x.round_end(m);
    x.insert(m, f);
}

TEST(ComponentTest, ExporterOperators) {
    cbig  x;
    cbig y(x);
    cbig z(fcpp::make_tagged_tuple<tag,hto,void>(3,'v',2.5));
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
