// Copyright © 2022 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/option/distribution.hpp"
#include "lib/option/functor.hpp"

using namespace fcpp;

struct tag {};
struct gat {};
struct oth {};

TEST(FunctorTest, Get) {
    std::mt19937 rnd(42);
    common::tagged_tuple_t<tag, int, gat, int> t;
    functor::get<tag> f1(rnd, t);
    t = {1, 2};
    EXPECT_EQ(1.0, f1(rnd, t));
    t = {3, 1};
    EXPECT_EQ(3.0, f1(rnd, t));
}

TEST(FunctorTest, Arithmetics) {
    std::mt19937 rnd(42);
    common::tagged_tuple_t<tag, int, gat, int> t;
    functor::sub<tag, gat> f1(rnd, t);
    functor::add<distribution::constant_n<int, 2>, gat> f2(rnd, t);
    functor::mul<functor::add<tag, distribution::constant_n<int, 2>>, gat> f3(rnd, t);
    functor::div<tag, distribution::constant_n<double, 2>> f4(rnd, t);
    functor::pow<tag, distribution::constant_n<int, 2>> f5(rnd, t);
    t = {1, 2};
    EXPECT_EQ(-1.0, f1(rnd, t));
    EXPECT_EQ(+4.0, f2(rnd, t));
    EXPECT_EQ(+6.0, f3(rnd, t));
    EXPECT_EQ(+0.5, f4(rnd, t));
    EXPECT_EQ(+1.0, f5(rnd, t));
    t = {3, 1};
    EXPECT_EQ(+2.0, f1(rnd, t));
    EXPECT_EQ(+3.0, f2(rnd, t));
    EXPECT_EQ(+5.0, f3(rnd, t));
    EXPECT_EQ(+1.5, f4(rnd, t));
    EXPECT_EQ(+9.0, f5(rnd, t));
}

TEST(FunctorTest, Maths) {
    std::mt19937 rnd(42);
    common::tagged_tuple_t<tag, int> t;
    functor::log<functor::exp<tag>> f1(rnd, t);
    t = {1};
    EXPECT_DOUBLE_EQ(1.0, f1(rnd, t));
    t = {2};
    EXPECT_DOUBLE_EQ(2.0, f1(rnd, t));
    t = {3};
    EXPECT_DOUBLE_EQ(3.0, f1(rnd, t));
    t = {4};
    EXPECT_DOUBLE_EQ(4.0, f1(rnd, t));
}

TEST(FunctorTest, Analitics) {
    std::mt19937 rnd(42);
    common::tagged_tuple_t<tag, int> t;
    functor::acc<tag> f1(rnd, t);
    functor::diff<tag> f2(rnd, t);
    functor::diff<functor::acc<tag>> f3(rnd, t);
    t = {1};
    EXPECT_EQ(+1.0, f1(rnd, t));
    EXPECT_EQ(+1.0, f2(rnd, t));
    EXPECT_EQ(+1.0, f3(rnd, t));
    t = {2};
    EXPECT_EQ(+3.0, f1(rnd, t));
    EXPECT_EQ(+1.0, f2(rnd, t));
    EXPECT_EQ(+2.0, f3(rnd, t));
    t = {3};
    EXPECT_EQ(+6.0, f1(rnd, t));
    EXPECT_EQ(+1.0, f2(rnd, t));
    EXPECT_EQ(+3.0, f3(rnd, t));
    t = {4};
    EXPECT_EQ(10.0, f1(rnd, t));
    EXPECT_EQ(+1.0, f2(rnd, t));
    EXPECT_EQ(+4.0, f3(rnd, t));
}
