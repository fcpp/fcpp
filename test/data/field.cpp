// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/data/field.hpp"


class FieldTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        fi1 = fcpp::details::make_field(2, {{1,1},{3,-1}});
        fi2 = fcpp::details::make_field(1, {{1,4},{2,3}});
        fd  = fcpp::details::make_field(0.5, {{2,3.25}});
        fb1 = fcpp::details::make_field(true, {{2,false},{3,true}});
        fb2 = fcpp::details::make_field(false,{{1,true}, {2,true}});
    }
    
    fcpp::field<int> fi1, fi2;
    fcpp::field<double> fd;
    fcpp::field<bool> fb1, fb2;
};


TEST_F(FieldTest, Constructors) {
    fcpp::field<double> x(fd), y;
    y = x;
    EXPECT_EQ(fcpp::other(fd), fcpp::other(y));
    EXPECT_EQ(fcpp::details::self(fd, 1), fcpp::details::self(y, 1));
    EXPECT_EQ(fcpp::details::self(fd, 2), fcpp::details::self(y, 2));
    EXPECT_EQ(fcpp::details::self(fd, 3), fcpp::details::self(y, 3));
}

TEST_F(FieldTest, Access) {
    EXPECT_EQ(2, fcpp::other(fi1));
    fcpp::other(fi1) = 3;
    EXPECT_EQ(3, fcpp::other(fi1));
    EXPECT_EQ(5, fcpp::other(5));
    EXPECT_EQ(-1, fcpp::details::self(fi1, 3));
    fcpp::details::self(fi1, 3) = -2;
    EXPECT_EQ(-2, fcpp::details::self(fi1, 3));
    EXPECT_EQ(-3, fcpp::details::self(-3, 999));
    fcpp::field<int> r = fcpp::details::align(fi2, {2,4});
    EXPECT_EQ(1, fcpp::details::self(r, 1));
    EXPECT_EQ(3, fcpp::details::self(r, 2));
}

TEST_F(FieldTest, MapReduce) {
    fcpp::field<bool> eq;
    fcpp::field<int> x = fcpp::map_hood([] (int i) {return i%2;}, fi2);
    eq = fcpp::map_hood([] (int i, int j) {return i==j;}, x, fcpp::details::make_field(1, {{1,0},{2,1}}));
    EXPECT_TRUE(eq);
    fcpp::mod_hood([] (int i, int j) {return i+j;}, x, fi1);
    eq = fcpp::map_hood([] (int i, int j) {return i==j;}, x, fcpp::details::make_field(3, {{1,1},{2,3},{3,0}}));
    EXPECT_TRUE(eq);
    double sum = fcpp::details::fold_hood([] (double i, double j) {return i+j;}, fd, {0,1,2});
    EXPECT_DOUBLE_EQ(4.25, sum);
    sum = fcpp::details::fold_hood([] (double i, double j) {return i+j;}, fi1, {0,1,2});
    EXPECT_DOUBLE_EQ(5, sum);
    sum = fcpp::details::fold_hood([] (double i, double j) {return i+j;}, 1, {0,1,2});
    EXPECT_DOUBLE_EQ(3, sum);
}

TEST_F(FieldTest, Conversion) {
    fi1 = fb1;
    fd = fi1;
    EXPECT_EQ(fd, fb1);
    fb1 = fd;
    EXPECT_EQ(fd, fb1);
    fcpp::field<char> fc = fi2;
    EXPECT_EQ(fc, fi2);
}

TEST_F(FieldTest, UnaryOperators) {
    fcpp::field<bool> eq = !fb1;
    EXPECT_FALSE(fcpp::other(eq));
    EXPECT_FALSE(fcpp::details::self(eq,1));
    EXPECT_TRUE(fcpp::details::self(eq,2));
    EXPECT_FALSE(fcpp::details::self(eq,3));
    eq = fcpp::map_hood([] (int i, int j) {return i==j;}, fi1, +fi1);
    EXPECT_TRUE(eq);
    eq = fcpp::map_hood([] (int i, int j) {return i==j;}, -fi1, fcpp::details::make_field(-2, {{1,-1},{3,1}}));
    EXPECT_TRUE(eq);
    fcpp::field<char> fc = fcpp::details::make_field<char>(15, {{1,22}});
    eq = fcpp::map_hood([] (int i, int j) {return i==j;}, ~fc, fcpp::details::make_field<char>(-16, {{1,-23}}));
    EXPECT_TRUE(eq);
}

TEST_F(FieldTest, BinaryOperators) {
    fcpp::field<bool> eq;
    eq = (fi1 + fi2) == fcpp::details::make_field(3, {{1,5},{2,5},{3,0}});
    EXPECT_TRUE(eq);
    eq = (fi1 * 2) == fcpp::details::make_field(4, {{1,2},{3,-2}});
    EXPECT_TRUE(eq);
    eq = (2 * fi1) == fcpp::details::make_field(4, {{1,2},{3,-2}});
    EXPECT_TRUE(eq);
    eq = (1 << fi2) == fcpp::details::make_field(2, {{1,16},{2,8}});
    EXPECT_TRUE(eq);
    eq = fi2 >= (fi2 >> 1);
    EXPECT_TRUE(eq);
    eq = (fi1 <= fi2) || (fi1 > fi2);
    EXPECT_TRUE(eq);
    eq = (fi1 != fi2) && (fi1 == fi2);
    EXPECT_FALSE(eq);
    EXPECT_TRUE(!eq);
    eq = (fi2 ^ fi2 ^ fi2) == fi2;
    EXPECT_TRUE(eq);
    eq = ((fi1 + fi2) - fi1) == fi2;
    EXPECT_TRUE(eq);
    eq = (fi2 % 2) == fcpp::details::make_field(1, {{1,0},{2,1}});
    EXPECT_TRUE(eq);
    double d = fcpp::details::fold_hood([] (double i, double j) {return i+j;}, fd / fi1, {0,1,2,3});
    EXPECT_DOUBLE_EQ(1.875, d);
}

TEST_F(FieldTest, InfixOperators) {
    fcpp::field<int> x = fi1;
    fi1 <<= 2;
    EXPECT_NE(fi1, x);
    fi1 /= 4;
    EXPECT_EQ(fi1, x);
    fi1 *= 4;
    fi1 >>= 2;
    EXPECT_EQ(fi1, x);
    fi1 += fi2;
    fi1 -= fi2;
    EXPECT_EQ(fi1, x);
    fi2 %= 2;
    fcpp::field<int> y = fcpp::details::make_field(1, {{1,0},{2,1}});
    EXPECT_EQ(fi2, y);
    fi1 ^= fi1;
    EXPECT_EQ(fi1, 0);
    x = fb1;
    fb1 |= true;
    EXPECT_TRUE(fb1);
    fb1 &= fb2;
    EXPECT_EQ(fb1, fb2);
}
