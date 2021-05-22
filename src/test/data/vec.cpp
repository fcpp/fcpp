// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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


TEST(VectorTest, Addition) {
    vec<3> t = gett();
    vec<3> u = getu();
    vec<3> v = getv();
    vec<3> w = getw();
    EXPECT_EQ(w,    u  +   v  );
    EXPECT_EQ(w,    u  +getv());
    EXPECT_EQ(w, getu()+   v  );
    EXPECT_EQ(w, getu()+getv());
    EXPECT_EQ(t,    v  +   3  );
    EXPECT_EQ(t, getv()+   3  );
    EXPECT_EQ(t,    3  +   v  );
    EXPECT_EQ(t,    3  +getv());
    u += v;
    EXPECT_EQ(w, u);
    v += 3;
    EXPECT_EQ(t, v);
}

TEST(VectorTest, Subtraction) {
    vec<3> t = gett();
    vec<3> u = getu();
    vec<3> v = getv();
    vec<3> w = getw();
    EXPECT_EQ(v,    w  -   u  );
    EXPECT_EQ(v,    w  -getu());
    EXPECT_EQ(v, getw()-   u  );
    EXPECT_EQ(v, getw()-getu());
    EXPECT_EQ(t,    v  +   3  );
    EXPECT_EQ(t, getv()+   3  );
    EXPECT_EQ(t,    3  +   v  );
    EXPECT_EQ(t,    3  +getv());
    w -= u;
    EXPECT_EQ(v, w);
    t -= 3;
    EXPECT_EQ(v, t);
}

TEST(VectorTest, Multiplication) {
    vec<2> t = {1,2};
    vec<2> u = {2,4};
    vec<2> v = {3,1};
    vec<2> w = {3,4};
    EXPECT_EQ(u,  t * 2);
    EXPECT_EQ(u,  2 * t);
    EXPECT_EQ(10, u * v);
    EXPECT_EQ(5,  norm(w));
    t *= 2;
    EXPECT_EQ(u, t);
}
