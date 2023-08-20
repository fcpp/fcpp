// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/data/vec.hpp"

using namespace fcpp;

vec<3> gett() {
    return {3,4,4};
}

vec<3> getu() {
    return {1,2,0};
}

vec<3> getv() {
    return {0,1,1};
}

vec<3> getw() {
    return {1,3,1};
}


TEST(VecTest, Addition) {
    vec<3> u = getu();
    vec<3> v = getv();
    vec<3> w = getw();
    EXPECT_EQ(w,    u  +   v  );
    EXPECT_EQ(w,    u  +getv());
    EXPECT_EQ(w, getu()+   v  );
    EXPECT_EQ(w, getu()+getv());
    u += v;
    EXPECT_EQ(w, u);
}

TEST(VecTest, Subtraction) {
    vec<3> u = getu();
    vec<3> v = getv();
    vec<3> w = getw();
    EXPECT_EQ(v,    w  -   u  );
    EXPECT_EQ(v,    w  -getu());
    EXPECT_EQ(v, getw()-   u  );
    EXPECT_EQ(v, getw()-getu());
    w -= u;
    EXPECT_EQ(v, w);
}

TEST(VecTest, Multiplication) {
    vec<2> t = {1,2};
    vec<2> u = {2,4};
    vec<2> v = {3,1};
    vec<2> w = {3,4};
    EXPECT_EQ(u,  t * 2);
    EXPECT_EQ(u,  2 * t);
    EXPECT_EQ(10, u * v);
    EXPECT_EQ(25,  abs(w));
    EXPECT_EQ(5,  norm(w));
    t *= 2;
    EXPECT_EQ(u, t);
}
