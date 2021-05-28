// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <sstream>

#include "gtest/gtest.h"

#include "lib/common/quaternion.hpp"

using namespace fcpp;
using namespace fcpp::common;

quaternion gets() {
    return {3,6,0,0};
}

quaternion gett() {
    return {4,2,0,0};
}

quaternion getu() {
    return {1,2,0,0};
}

quaternion getv() {
    return {0,1,1,0};
}

quaternion getw() {
    return {1,3,1,0};
}


TEST(QuaternionTest, Construction) {
    real_t v[3] = {3,2,1};
    quaternion a;
    quaternion b(2.5);
    quaternion c(1, 2, 3, 4);
    quaternion d(30, {1,2,3});
    quaternion e(a);
    quaternion f(std::move(b));
    quaternion g(60, v);
    quaternion h({1,2,3});
    quaternion j(v);
    a = e;
    b = std::move(f);
    std::stringstream ss;
    ss << 4.2 << c;
    EXPECT_EQ(ss.str(), std::string("4.21 + 2i + 3j + 4k"));
}

TEST(QuaternionTest, Comparison) {
    quaternion a(2, 4, 5, 6);
    EXPECT_EQ(abs(a), 81);
    EXPECT_EQ(norm(a), 9);
    EXPECT_LT(a, 10);
    EXPECT_LE(a, 9);
    EXPECT_GE(a, 9);
    EXPECT_GT(a, 8);
}

TEST(QuaternionTest, Unary) {
    quaternion q(1,0,-1,0);
    quaternion u(-1,0,1,0);
    quaternion v(1,0,1,0);
    quaternion w(0.5,0,0.5,0);
    EXPECT_EQ(q, +q);
    EXPECT_EQ(u, -q);
    EXPECT_EQ(v, ~q);
    EXPECT_EQ(w, !q);
}

TEST(QuaternionTest, Addition) {
    quaternion t = gett();
    quaternion u = getu();
    quaternion v = getv();
    quaternion w = getw();
    EXPECT_EQ(w,    u  +   v  );
    EXPECT_EQ(w,    u  +getv());
    EXPECT_EQ(w, getu()+   v  );
    EXPECT_EQ(w, getu()+getv());
    EXPECT_EQ(t,    u  +   3  );
    EXPECT_EQ(t,    3  +   u  );
    EXPECT_EQ(t, getu()+   3  );
    EXPECT_EQ(t,    3  +getu());
    u += v;
    EXPECT_EQ(w, u);
}

TEST(QuaternionTest, Subtraction) {
    quaternion t = gett();
    quaternion u = getu();
    quaternion v = getv();
    quaternion w = getw();
    EXPECT_EQ(v,    w  -   u  );
    EXPECT_EQ(v,    w  -getu());
    EXPECT_EQ(v, getw()-   u  );
    EXPECT_EQ(v, getw()-getu());
    EXPECT_EQ(u,    t  -   3  );
    EXPECT_EQ(-u,   3  -   t  );
    EXPECT_EQ(u, gett()-   3  );
    EXPECT_EQ(-u,   3  -gett());
    w -= u;
    EXPECT_EQ(v, w);
}

TEST(QuaternionTest, Multiplication) {
    quaternion s = gets();
    quaternion u = getu();
    EXPECT_EQ(s,    u  *   3  );
    EXPECT_EQ(s,    3  *   u  );
    EXPECT_EQ(s, getu()*   3  );
    EXPECT_EQ(s,    3  *getu());
    quaternion a(1,2,3,4);
    quaternion b(4,3,2,1);
    quaternion c(-12,6,24,12);
    EXPECT_EQ(c, a * b);
    EXPECT_EQ(c, quaternion(a) * b);
    EXPECT_EQ(c, a * quaternion(b));
    EXPECT_EQ(c, quaternion(a) * quaternion(b));
}

TEST(QuaternionTest, Division) {
    quaternion s = gets();
    quaternion u = getu();
    EXPECT_EQ(u,    s  /   3  );
    EXPECT_EQ(!u,   3  /   s  );
    EXPECT_EQ(u, gets()/   3  );
    EXPECT_EQ(!u,   3  /gets());
    quaternion a(1,2,3,4);
    quaternion b(4,3,2,1);
    quaternion c(-12,6,24,12);
    EXPECT_EQ(a, c / b);
}

TEST(QuaternionTest, Rotation) {
    real_t pi = acos(-1);
    quaternion p({1,1,0});
    quaternion r(-pi/2, {-1,1,1});
    quaternion q = r*p*~r;
    EXPECT_NEAR(q[0], 0, 1e-9);
    real_t e = 0;
    for (size_t i=1; i<4; ++i) e += p[i]*q[i];
    EXPECT_NEAR(e, 0, 1e-9);
    r *= r;
    q = r*p*~r;
    EXPECT_LT(p+q, 1e-9);
    r *= r;
    q = r*p*~r;
    EXPECT_LT(p-q, 1e-9);
}
