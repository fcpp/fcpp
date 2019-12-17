// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <string>
#include <typeinfo>

#include "gtest/gtest.h"

#include "lib/device/component.hpp"


struct tag {};
struct gat {};
struct oth {};
struct hto {};

template <typename T>
struct expose_get : public T {
    using T::T;
    using T::get;
};

template <typename T>
using comp1 = fcpp::storage_component<fcpp::tagged_tuple<T,int>>;

using csmall = expose_get<comp1<tag>>;

using comp3 = fcpp::multi_component<comp1<tag>, comp1<gat>, comp1<oth>>;

using comp4 = fcpp::storage_component<fcpp::tagged_tuple<hto,char>, comp3>;

using cbig = expose_get<comp4>;


TEST(ComponentTest, StorageOperators) {
    csmall  x;
    csmall y(x);
    csmall z(fcpp::make_tagged_tuple<tag,hto,void>(3,'v',2.5));
    int i;
    i = z.get<tag>();
    EXPECT_EQ(3, i);
    x.get<tag>() = 5;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
    i = z.get<tag>();
    EXPECT_EQ(5, i);
}

TEST(ComponentTest, MultiOperators) {
    cbig  x;
    cbig y(x);
    cbig z(fcpp::make_tagged_tuple<tag,hto,void>(3,'v',2.5));
    char c;
    c = z.get<hto>();
    EXPECT_EQ('v', c);
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(x, z);
}

TEST(ComponentTest, Manager) {
    cbig::manager m;
    times_t t, inf = std::numeric_limits<times_t>::max();
    t = m.next();
    EXPECT_EQ(inf, t);
    t = m.update();
    EXPECT_EQ(inf, t);
    t = m.update();
    EXPECT_EQ(inf, t);
    std::string ex, res;
    ex  = typeid(fcpp::tagged_tuple<>).name();
    res = typeid(cbig::message_t).name();
    EXPECT_EQ(ex, res);
}

TEST(ComponentTest, Functions) {
    csmall x;
    csmall::manager m;
    x.round_start(m);
    csmall::message_t f = x.round_end(m);
    x.insert(m, f);
    cbig y;
    cbig::manager n;
    y.round_start(n);
    cbig::message_t g = y.round_end(n);
    y.insert(n, g);
}
