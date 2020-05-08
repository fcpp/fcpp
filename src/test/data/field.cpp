// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "test/helper.hpp"

#include "lib/data/field.hpp"
#include "lib/data/tuple.hpp"


using namespace fcpp;


class FieldTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
        fi1 = details::make_field(2, {{1,1},{3,-1}});
        fi2 = details::make_field(1, {{1,4},{2,3}});
        fd  = details::make_field(0.5, {{2,3.25}});
        fb1 = details::make_field(true, {{2,false},{3,true}});
        fb2 = details::make_field(false,{{1,true}, {2,true}});
    }
    
    template <typename T>
    const T& constify(T& x) {
        return static_cast<const T&>(x);
    }
    
    field<int> fi1, fi2;
    field<double> fd;
    field<bool> fb1, fb2;
};


TEST_F(FieldTest, Constructors) {
    field<double> x(fd), y;
    y = x;
    EXPECT_EQ(details::other(fd), details::other(y));
    EXPECT_EQ(details::self(fd, 1), details::self(y, 1));
    EXPECT_EQ(details::self(fd, 2), details::self(y, 2));
    EXPECT_EQ(details::self(fd, 3), details::self(y, 3));
}

TEST_F(FieldTest, Access) {
    EXPECT_EQ(2, details::other(fi1));
    details::other(fi1) = 3;
    EXPECT_EQ(3, details::other(fi1));
    EXPECT_EQ(5, details::other(5));
    EXPECT_EQ(-1, details::self(fi1, 3));
    details::self(fi1, 3) = -2;
    EXPECT_EQ(-2, details::self(fi1, 3));
    EXPECT_EQ(-3, details::self(-3, 999));
    field<int> r = details::align(fi2, {2,4});
    EXPECT_EQ(1, details::self(r, 1));
    EXPECT_EQ(3, details::self(r, 2));
}

TEST_F(FieldTest, TupleAccess) {
    field<tuple<int,double>> t1{make_tuple(42,2.5)};
    tuple<field<int>, field<double>> t2{fi1, fd};
    tuple<field<int>, double> t3{fi1, 2.5};
    tuple<tuple<field<int>,int>, field<double>> t4{{fi1, 42},fd};
    EXPECT_SAME(field<tuple<int,double>>&&,         decltype(std::move(t1)));
    EXPECT_SAME(const field<tuple<int,double>>&,    decltype(constify(t1)));
    EXPECT_SAME(const field<tuple<int,double>>&&,   decltype(std::move(constify(t1))));
    EXPECT_SAME(tuple<int, double>&,                decltype(details::other(t1)));
    EXPECT_SAME(tuple<int, double>&&,               decltype(details::other(std::move(t1))));
    EXPECT_SAME(const tuple<int, double>&,          decltype(details::other(constify(t1))));
    EXPECT_SAME(const tuple<int, double>&&,         decltype(details::other(std::move(constify(t1)))));
    EXPECT_SAME(tuple<int&, double&>,               decltype(details::other(t2)));
    EXPECT_SAME(tuple<int&&, double&&>,             decltype(details::other(std::move(t2))));
    EXPECT_SAME(tuple<const int&, const double&>,   decltype(details::other(constify(t2))));
    EXPECT_SAME(tuple<const int&&, const double&&>, decltype(details::other(std::move(constify(t2)))));
    EXPECT_SAME(tuple<int&, const double&>,         decltype(details::other(t3)));
    EXPECT_SAME(tuple<int&&, const double&&>,       decltype(details::other(std::move(t3))));
    EXPECT_SAME(tuple<const int&, const double&>,   decltype(details::other(constify(t3))));
    EXPECT_SAME(tuple<const int&&, const double&&>, decltype(details::other(std::move(constify(t3)))));
    EXPECT_SAME(tuple<tuple<int&, const int&>, double&>,                decltype(details::other(t4)));
    EXPECT_SAME(tuple<tuple<int&&, const int&&>, double&&>,             decltype(details::other(std::move(t4))));
    EXPECT_SAME(tuple<tuple<const int&, const int&>, const double&>,    decltype(details::other(constify(t4))));
    EXPECT_SAME(tuple<tuple<const int&&, const int&&>, const double&&>,
                decltype(details::other(std::move(constify(t4)))));
    auto x1 = details::other(t1);
    EXPECT_EQ(42,  get<0>(x1));
    EXPECT_EQ(2.5, get<1>(x1));
    details::other(t1) = make_tuple(10, 0.0);
    x1 = details::other(std::move(t1));
    EXPECT_EQ(10,  get<0>(x1));
    EXPECT_EQ(0.0, get<1>(x1));
    auto x2 = details::other(t2);
    EXPECT_EQ(2,   details::other(get<0>(t2)));
    EXPECT_EQ(0.5, details::other(get<1>(t2)));
    EXPECT_EQ(2,   get<0>(x2));
    EXPECT_EQ(0.5, get<1>(x2));
    details::other(t2) = make_tuple(5, 0.0);
    EXPECT_EQ(5,   details::other(get<0>(t2)));
    EXPECT_EQ(0.0, details::other(get<1>(t2)));
    x2 = details::other(std::move(t2));
    EXPECT_EQ(5,   get<0>(x2));
    EXPECT_EQ(0.0, get<1>(x2));
    auto x3 = details::other(t3);
    EXPECT_EQ(2,   details::other(get<0>(t3)));
    EXPECT_EQ(2.5, details::other(get<1>(t3)));
    EXPECT_EQ(2,   get<0>(x3));
    EXPECT_EQ(2.5, get<1>(x3));
    get<0>(details::other(t3)) = 5;
    EXPECT_EQ(5,   details::other(get<0>(t3)));
    EXPECT_EQ(2.5, details::other(get<1>(t3)));
    x2 = details::other(std::move(t3));
    EXPECT_EQ(5,   get<0>(x3));
    EXPECT_EQ(2.5, get<1>(x3));
    auto x4 = details::other(t4);
    EXPECT_EQ(2,   get<0>(get<0>(x4)));
    EXPECT_EQ(42,  get<1>(get<0>(x4)));
    EXPECT_EQ(0.5, get<1>(x4));
    get<1>(details::self(t4, 24)) = 9.0;
    get<1>(details::other(t4)) = 2.5;
    get<0>(get<0>(details::other(t4))) = 12;
    get<1>(x4) *= 2;
    EXPECT_EQ(12,  details::other(get<0>(get<0>(t4))));
    EXPECT_EQ(42,  get<1>(get<0>(t4)));
    EXPECT_EQ(5.0, details::other(get<1>(t4)));
    EXPECT_EQ(12,  get<0>(get<0>(x4)));
    EXPECT_EQ(42,  get<1>(get<0>(x4)));
    EXPECT_EQ(5.0, get<1>(x4));
    EXPECT_EQ(9.0, details::self(get<1>(t4), 24));
    auto x5 = details::self(t4, 24);
    EXPECT_EQ(2,   get<0>(get<0>(x5)));
    EXPECT_EQ(42,  get<1>(get<0>(x5)));
    EXPECT_EQ(9.0, get<1>(x5));
    std::unordered_set<device_t> ex, res;
    ex  = {1, 2, 3, 24};
    res = details::joined_domain(t1, t2, t3, t4);
    EXPECT_EQ(ex, res);
    auto f4 = details::align(t4, {1,2});
    ex  = {1, 2, 3};
    res = details::joined_domain(t1,t2,t3,t4);
    EXPECT_EQ(ex, res);
    ex  = {1, 2};
    res = details::joined_domain(t1,t4);
    EXPECT_EQ(ex, res);
    EXPECT_EQ(1,   details::self(get<0>(get<0>(t4)), 1));
    EXPECT_EQ(12,  details::self(get<0>(get<0>(t4)), 2));
    EXPECT_EQ(12,  details::self(get<0>(get<0>(t4)), 3));
    EXPECT_EQ(12,  details::self(get<0>(get<0>(t4)), 24));
    EXPECT_EQ(1,   details::self(get<0>(get<0>(f4)), 1));
    EXPECT_EQ(12,  details::self(get<0>(get<0>(f4)), 2));
    EXPECT_EQ(12,  details::self(get<0>(get<0>(f4)), 3));
    EXPECT_EQ(12,  details::self(get<0>(get<0>(f4)), 24));
    EXPECT_EQ(42,  get<1>(get<0>(t4)));
    EXPECT_EQ(42,  get<1>(get<0>(f4)));
    EXPECT_EQ(5.0, details::self(get<1>(t4), 1));
    EXPECT_EQ(3.25,details::self(get<1>(t4), 2));
    EXPECT_EQ(5.0, details::self(get<1>(t4), 3));
    EXPECT_EQ(5.0, details::self(get<1>(t4), 24));
    EXPECT_EQ(5.0, details::self(get<1>(f4), 1));
    EXPECT_EQ(3.25,details::self(get<1>(f4), 2));
    EXPECT_EQ(5.0, details::self(get<1>(f4), 3));
    EXPECT_EQ(5.0, details::self(get<1>(f4), 24));
}

TEST_F(FieldTest, MapReduce) {
    field<bool> eq;
    field<int> x = details::map_hood([] (int i) {return i%2;}, fi2);
    eq = details::map_hood([] (int i, int j) {return i==j;}, x, details::make_field(1, {{1,0},{2,1}}));
    EXPECT_TRUE(eq);
    details::mod_hood([] (int i, int j) {return i+j;}, x, fi1);
    eq = details::map_hood([] (int i, int j) {return i==j;}, x, details::make_field(3, {{1,1},{2,3},{3,0}}));
    EXPECT_TRUE(eq);
    double sum = details::fold_hood([] (double i, double j) {return i+j;}, fd, {0,1,2});
    EXPECT_DOUBLE_EQ(4.25, sum);
    sum = details::fold_hood([] (double i, double j) {return i+j;}, fi1, {0,1,2});
    EXPECT_DOUBLE_EQ(5, sum);
    sum = details::fold_hood([] (double i, double j) {return i+j;}, 1, {0,1,2});
    EXPECT_DOUBLE_EQ(3, sum);
    tuple<field<int>, int> y{{1}, 2};
    details::self(get<0>(y), 0) = 5;
    details::self(get<0>(y), 1) = 6;
    field<tuple<int,int>> z{{3,4}};
    details::self(z, 1) = make_tuple(7,8);
    details::self(z, 2) = make_tuple(9,0);
    field<tuple<int,int>> w = details::map_hood([] (tuple<int,int> a, tuple<int,int> b, tuple<int,int> c) {
        return b * c - a;
    }, y, z, y);
    EXPECT_EQ(std::unordered_set<device_t>({0,1,2}), details::joined_domain(w));
    tuple<int,int> ex, res;
    ex  = {10,6};
    res = details::self(w, 0);
    EXPECT_EQ(ex, res);
    ex  = {36,14};
    res = details::self(w, 1);
    EXPECT_EQ(ex, res);
    ex  = {8,-2};
    res = details::self(w, 2);
    EXPECT_EQ(ex, res);
    ex  = {2,6};
    res = details::other(w);
    EXPECT_EQ(ex, res);
    tuple<field<int>, field<int>> f{{1}, {2}};
    details::self(f, 1) = make_tuple(3,4);
    details::mod_hood([] (tuple<int,int> a, tuple<int,int> b, tuple<int,int> c) {
        return a + b + c;
    }, f, z, y);
    EXPECT_EQ(std::unordered_set<device_t>({0,1,2}), details::joined_domain(f));
    EXPECT_EQ(details::self(f, 0), make_tuple(9,8));
    EXPECT_EQ(details::self(f, 1), make_tuple(16,14));
    EXPECT_EQ(details::self(f, 2), make_tuple(11,4));
    EXPECT_EQ(details::other(f),   make_tuple(5,8));
    tuple<int,int> g = details::fold_hood([] (tuple<int,int> a, tuple<int,int> b) {
        return a + b;
    }, y, {1,2});
    EXPECT_EQ(make_tuple(7,4), g);
    g = details::fold_hood([] (tuple<int,int> a, tuple<int,int> b) {
           return a < b ? a : b;
    }, f, {1,2,3});
    EXPECT_EQ(make_tuple(5,8), g);
}

TEST_F(FieldTest, Conversion) {
    fi1 = fb1;
    fd = fi1;
    EXPECT_EQ(fd, fb1);
    fb1 = fd;
    EXPECT_EQ(fd, fb1);
    field<char> fc = fi2;
    EXPECT_EQ(fc, fi2);
}

TEST_F(FieldTest, BasicFunctions) {
    field<int> x;
    x = mux(true, fi1, fi2);
    EXPECT_EQ(x, fi1);
    x = mux(false, field<int>(fi1), field<int>(fi2));
    EXPECT_EQ(x, fi2);
    x = mux(fb1, fi1, fi2);
    field<int> y = details::make_field(2, {{1,1}, {2,3}, {3,-1}});
    EXPECT_EQ(x, y);
    tuple<field<int>, int> a{fi1, 1};
    tuple<field<int>, int> b{fi2, 2};
    field<tuple<int,int>> c = mux(fb2, a, b);
    field<tuple<int,int>> d = details::make_field(make_tuple(1, 2), {
        {1, make_tuple(1, 1)},
        {2, make_tuple(2, 1)},
        {3, make_tuple(1, 2)}
    });
    EXPECT_EQ(c, d);
    c = max(a, b);
    d = details::make_field(make_tuple(2, 1), {
        {1, make_tuple(4, 2)},
        {2, make_tuple(3, 2)},
        {3, make_tuple(1, 2)}
    });
    EXPECT_EQ(c, d);
    c = min(a, b);
    d = details::make_field(make_tuple(1, 2), {
        {1, make_tuple( 1, 1)},
        {2, make_tuple( 2, 1)},
        {3, make_tuple(-1, 1)}
    });
    EXPECT_EQ(c, d);
    x = get<0>(d);
    y = details::make_field(1, {{1,1}, {2,2}, {3,-1}});
    EXPECT_EQ(x,y);
    x = get<1>(d);
    y = details::make_field(2, {{1,1}, {2,1}, {3,1}});
    EXPECT_EQ(x,y);
}

TEST_F(FieldTest, UnaryOperators) {
    field<bool> eq = !fb1;
    EXPECT_FALSE(details::other(eq));
    EXPECT_FALSE(details::self(eq,1));
    EXPECT_TRUE(details::self(eq,2));
    EXPECT_FALSE(details::self(eq,3));
    eq = details::map_hood([] (int i, int j) {return i==j;}, fi1, +fi1);
    EXPECT_TRUE(eq);
    eq = details::map_hood([] (int i, int j) {return i==j;}, -fi1, details::make_field(-2, {{1,-1},{3,1}}));
    EXPECT_TRUE(eq);
    field<char> fc = details::make_field<char>(15, {{1,22}});
    eq = details::map_hood([] (int i, int j) {return i==j;}, ~fc, details::make_field<char>(-16, {{1,-23}}));
    EXPECT_TRUE(eq);
    tuple<field<bool>, bool> x{{true}, false};
    details::self(get<0>(x), 2) = false;
    x = !x;
    EXPECT_EQ(std::unordered_set<device_t>({2}), details::joined_domain(x));
    EXPECT_EQ(make_tuple(true,true), details::self(x, 2));
    EXPECT_EQ(make_tuple(false,true), details::other(x));
}

TEST_F(FieldTest, BinaryOperators) {
    field<bool> eq;
    eq = (fi1 + fi2) == details::make_field(3, {{1,5},{2,5},{3,0}});
    EXPECT_TRUE(eq);
    eq = (fi1 * 2) == details::make_field(4, {{1,2},{3,-2}});
    EXPECT_TRUE(eq);
    eq = (2 * fi1) == details::make_field(4, {{1,2},{3,-2}});
    EXPECT_TRUE(eq);
    eq = (1 << fi2) == details::make_field(2, {{1,16},{2,8}});
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
    eq = (fi2 % 2) == details::make_field(1, {{1,0},{2,1}});
    EXPECT_TRUE(eq);
    double d = details::fold_hood([] (double i, double j) {return i+j;}, fd / fi1, {0,1,2,3});
    EXPECT_DOUBLE_EQ(1.875, d);
    tuple<field<int>, double> x{{1}, 2.5};
    tuple<field<int>, field<double>> y{{2}, {3.0}};
    field<tuple<int,double>> z{{3, 3.5}};
    details::self(get<0>(x), 0) = 0;
    details::self(y, 1) = make_tuple(-1, 2.0);
    details::self(z, 2) = make_tuple(4, 4.0);
    EXPECT_SAME(decltype(y), decltype(x + y));
    EXPECT_SAME(decltype(z), decltype(x + z));
    EXPECT_SAME(decltype(z), decltype(y + z));
    y = x + y;
    z = x + z;
    z = z - y;
    EXPECT_EQ(std::unordered_set<device_t>({0,1,2}), details::joined_domain(z));
    EXPECT_EQ(make_tuple(1,0.5), details::self(z, 0));
    EXPECT_EQ(make_tuple(4,1.5), details::self(z, 1));
    EXPECT_EQ(make_tuple(2,1.0), details::self(z, 2));
    EXPECT_EQ(make_tuple(1,0.5), details::other(z));
}

TEST_F(FieldTest, InfixOperators) {
    field<int> f = fi2;
    fi2 <<= 2;
    EXPECT_NE(fi2, f);
    fi2 /= 4;
    EXPECT_EQ(fi2, f);
    fi2 *= 4;
    EXPECT_NE(fi2, f);
    fi2 >>= 2;
    EXPECT_EQ(fi2, f);
    fi2 += fi1;
    EXPECT_NE(fi2, f);
    fi2 -= fi1;
    EXPECT_EQ(fi2, f);
    fi2 %= 2;
    EXPECT_EQ(fi2, details::make_field(1, {{1,0},{2,1}}));
    fi1 ^= fi1;
    EXPECT_EQ(fi1, 0);
    f = fb1;
    fb1 |= true;
    EXPECT_TRUE(fb1);
    fb1 &= fb2;
    EXPECT_EQ(fb1, fb2);
    tuple<field<int>, double> x{{1}, 2.5};
    tuple<field<int>, field<double>> y{{2}, {3.0}};
    field<tuple<int,double>> z{{3, 3.5}};
    details::self(get<0>(x), 0) = 0;
    details::self(y, 1) = make_tuple(-1, 2.0);
    details::self(z, 2) = make_tuple(4, 4.0);
    EXPECT_SAME(decltype(y), decltype(x + y));
    EXPECT_SAME(decltype(z), decltype(x + z));
    EXPECT_SAME(decltype(z), decltype(y + z));
    y += x;
    z += x;
    z -= y;
    EXPECT_EQ(std::unordered_set<device_t>({0,1,2}), details::joined_domain(z));
    EXPECT_EQ(make_tuple(1,0.5), details::self(z, 0));
    EXPECT_EQ(make_tuple(4,1.5), details::self(z, 1));
    EXPECT_EQ(make_tuple(2,1.0), details::self(z, 2));
    EXPECT_EQ(make_tuple(1,0.5), details::other(z));
}
