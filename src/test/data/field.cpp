// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include <unordered_map>

#include "lib/data/field.hpp"

#include "test/helper.hpp"

#define FIELD_EQ(a, b)  EXPECT_EQ(a, b); EXPECT_EQ(joined_domain(a), joined_domain(b))


using namespace fcpp;


//! @brief Builds a field for testing purposes
template <typename A>
field<A> build_field(A def, std::unordered_map<device_t, A> data) {
    std::vector<device_t> ids;
    ids.reserve(data.size());
    for (auto const& x : data)
        ids.push_back(x.first);
    std::sort(ids.begin(), ids.end());
    std::vector<A> vals;
    vals.resize(data.size()+1);
    vals[0] = def;
    for (size_t i = 0; i < data.size(); ++i)
        vals[i+1] = data[ids[i]];
    return details::make_field(std::move(ids), std::move(vals));
}

//! @brief Joins the domain of a sequence of fields.
template <typename... A>
std::vector<device_t> joined_domain(A const&... a) {
    std::vector<device_t> res;
    for (details::field_iterator<tuple<A...> const> it(a...); not it.end(); ++it)
        res.push_back(it.id());
    return res;
}

class FieldTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
        fi1 = build_field(2, {{1,1},{3,-1}});
        fi2 = build_field(1, {{1,4},{2,3}});
        fd  = build_field(0.5, {{2,3.25}});
        fb1 = build_field(true, {{2,false},{3,true}});
        fb2 = build_field(false,{{1,true}, {2,true}});
    }

    template <typename T>
    T const& constify(T& x) {
        return static_cast<T const&>(x);
    }

    template <typename T>
    T copy(T const& x) {
        return x;
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

TEST_F(FieldTest, Conversion) {
    fi1 = fb1;
    fd = fi1;
    EXPECT_EQ(fd, fb1);
    fb1 = fd;
    EXPECT_EQ(fd, fb1);
    field<char> fc = fi2;
    EXPECT_EQ(fc, fi2);
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
    EXPECT_SAME(field<tuple<int,double>> const&,    decltype(constify(t1)));
    EXPECT_SAME(field<tuple<int,double>> const&&,   decltype(std::move(constify(t1))));
    EXPECT_SAME(tuple<int, double>&,                decltype(details::other(t1)));
    EXPECT_SAME(tuple<int, double>,                 decltype(details::other(std::move(t1))));
    EXPECT_SAME(tuple<int, double> const&,          decltype(details::other(constify(t1))));
    EXPECT_SAME(tuple<int, double>,                 decltype(details::other(std::move(constify(t1)))));
    EXPECT_SAME(tuple<int&, double&>,               decltype(details::other(t2)));
    EXPECT_SAME(tuple<int, double>,                 decltype(details::other(std::move(t2))));
    EXPECT_SAME(tuple<int const&, double const&>,   decltype(details::other(constify(t2))));
    EXPECT_SAME(tuple<int, double>,                 decltype(details::other(std::move(constify(t2)))));
    EXPECT_SAME(tuple<int&, double const&>,         decltype(details::other(t3)));
    EXPECT_SAME(tuple<int, double>,                 decltype(details::other(std::move(t3))));
    EXPECT_SAME(tuple<int const&, double const&>,   decltype(details::other(constify(t3))));
    EXPECT_SAME(tuple<int, double>,                 decltype(details::other(std::move(constify(t3)))));
    EXPECT_SAME(tuple<tuple<int&, int const&>, double&>,             decltype(details::other(t4)));
    EXPECT_SAME(tuple<tuple<int, int>, double>,                      decltype(details::other(std::move(t4))));
    EXPECT_SAME(tuple<tuple<int const&, int const&>, double const&>, decltype(details::other(constify(t4))));
    EXPECT_SAME(tuple<tuple<int, int>, double>,                      decltype(details::other(std::move(constify(t4)))));
    auto x1 = details::other(t1);
    EXPECT_EQ(42,  get<0>(x1));
    EXPECT_EQ(2.5, get<1>(x1));
    details::other(t1) = make_tuple(10, 0.0);
    x1 = details::other(copy(t1));
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
    x2 = details::other(copy(t2));
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
    x2 = details::other(copy(t3));
    EXPECT_EQ(5,   get<0>(x3));
    EXPECT_EQ(2.5, get<1>(x3));
    details::self(t4, 24);
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
    std::vector<device_t> ex, res;
    ex  = {1, 2, 3, 24};
    res = joined_domain(t1, t2, t3, t4);
    EXPECT_EQ(ex, res);
    auto f4 = details::align(t4, {1,2});
    ex  = {1, 2, 3};
    res = joined_domain(t1,t2,t3,t4);
    EXPECT_EQ(ex, res);
    ex  = {1, 2};
    res = joined_domain(t1,t4);
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

TEST_F(FieldTest, AlignInplace) {
    field<int> fex, fres;
    fex = build_field(2, {{1,1},{3,-1}});
    details::align_inplace(fex, {2,3,4});
    fres = build_field(2, {{2,2},{3,-1},{4,2}});
    FIELD_EQ(fex, fres);
    tuple<field<int>,field<int>> tex, tres;
    tex = {build_field(2, {{1,1},{3,-1}}), build_field(3, {{2,1},{3,-1},{5,7}})};
    details::align_inplace(tex, {2,3,4});
    tres = {build_field(2, {{2,2},{3,-1},{4,2}}), build_field(3, {{2,1},{3,-1},{4,3}})};
    FIELD_EQ(tex, tres);
    tuple<tuple<field<int>,field<int>>,field<int>> ttex, ttres;
    ttex = {{build_field(2, {{1,1},{3,-1}}), build_field(3, {{2,1},{3,-1},{5,7}})}, build_field(2, {{1,1},{3,-1}})};
    details::align_inplace(ttex, {2,3,4});
    ttres = {{build_field(2, {{2,2},{3,-1},{4,2}}), build_field(3, {{2,1},{3,-1},{4,3}})}, build_field(2, {{2,2},{3,-1},{4,2}})};
    FIELD_EQ(ttex, ttres);
}

TEST_F(FieldTest, ModOther) {
    field<int> fin, fex, fres;
    fin = build_field(2, {{1,1},{3,-1}});
    fres = details::mod_other(fin, 4, {2,3,4});
    fex = build_field(4, {{2,2},{3,-1},{4,2}});
    FIELD_EQ(fin, build_field(2, {{1,1},{3,-1}}));
    FIELD_EQ(fex, fres);
    tuple<tuple<field<int>,int>,int> tin = {{build_field(2, {{1,1},{3,-1}}), 4}, 2};
    tuple<tuple<int,int>,int> to = {{1,2},0};
    field<tuple<tuple<int,int>,int>> tex, tres;
    tres = details::mod_other(tin, to, {2,3,4});
    tex = build_field(to, {{2,{{2,4},2}},{3,{{-1,4},2}},{4,{{2,4},2}}});
    FIELD_EQ(tex, tres);
}

TEST_F(FieldTest, ModSelf) {
    field<int> fin, fex, fres;
    fin = build_field(2, {{1,1},{3,-1}});
    fres = details::mod_self(fin, 4, 3);
    fex = build_field(2, {{1,1},{3,4}});
    FIELD_EQ(fin, build_field(2, {{1,1},{3,-1}}));
    FIELD_EQ(fex, fres);
    fres = details::mod_self(fin, build_field(42, {{0,5},{1,3}}), 0);
    fex = build_field(2, {{0,5},{1,1},{3,-1}});
    FIELD_EQ(fex, fres);
    fres = details::mod_self(std::move(fin), 4, 3);
    fex = build_field(2, {{1,1},{3,4}});
    FIELD_EQ(fex, fres);
    tuple<tuple<field<int>,int>,field<int>> tin = {{build_field(2, {{1,1},{3,-1}}), 4}, build_field(1, {{2,5},{3,0}})};
    tuple<tuple<int,int>,int> t;
    field<tuple<tuple<int,int>,int>> tex, tres;
    t = {{1,2},0};
    tres = details::mod_self(tin, t, 2);
    t = {{2,4},1};
    tex = build_field(t, {{1,{{1,4},1}},{2,{{1,2},0}},{3,{{-1,4},0}}});
    FIELD_EQ(tex, tres);
}

TEST_F(FieldTest, MapReduce) {
    field<bool> eq;
    field<int> x = map_hood([] (int i) {return i%2;}, fi2);
    eq = map_hood([] (int i, int j) {return i==j;}, x, build_field(1, {{1,0},{2,1}}));
    EXPECT_TRUE(eq);
    mod_hood([] (int i, int j) {return i+j;}, x, fi1);
    eq = map_hood([] (int i, int j) {return i==j;}, x, build_field(3, {{1,1},{2,3},{3,0}}));
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
    using loc_type = tuple<int, int>;
    loc_type t = details::fold_hood([] (device_t uid, loc_type x, loc_type y) {
        return uid == 1 ? y : std::max(x, y);
    }, y, {0,1,2});
    EXPECT_EQ(t, make_tuple(5,2));
    tuple<device_t, loc_type> u;
    u = details::fold_hood([] (device_t uid, loc_type x, tuple<device_t, loc_type> y) {
        return std::max(make_tuple(uid, x), y);
    }, y, make_tuple(0, make_tuple(3,-1)), {0,1,2,3}, 2);
    EXPECT_EQ(u, (tuple<device_t, loc_type>{3, {1,2}}));
    field<tuple<int,int>> z{{3,4}};
    details::self(z, 1) = make_tuple(7,8);
    details::self(z, 2) = make_tuple(9,0);
    field<tuple<int,int>> w = map_hood([] (tuple<int,int> a, tuple<int,int> b, tuple<int,int> c) {
        return b * c - a;
    }, y, z, y);
    EXPECT_EQ(std::vector<device_t>({0,1,2}), joined_domain(w));
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
    mod_hood([] (tuple<int,int> a, tuple<int,int> b, tuple<int,int> c) {
        return a + b + c;
    }, f, z, y);
    EXPECT_EQ(std::vector<device_t>({0,1,2}), joined_domain(f));
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

TEST_F(FieldTest, UnaryOperators) {
    field<bool> eq = !fb1;
    EXPECT_FALSE(details::other(eq));
    EXPECT_FALSE(details::self(eq,1));
    EXPECT_TRUE(details::self(eq,2));
    EXPECT_FALSE(details::self(eq,3));
    eq = map_hood([] (int i, int j) {return i==j;}, fi1, +fi1);
    EXPECT_TRUE(eq);
    eq = map_hood([] (int i, int j) {return i==j;}, -fi1, build_field(-2, {{1,-1},{3,1}}));
    EXPECT_TRUE(eq);
    field<char> fc = build_field<char>(15, {{1,22}});
    eq = map_hood([] (int i, int j) {return i==j;}, ~fc, build_field<char>(-16, {{1,-23}}));
    EXPECT_TRUE(eq);
    tuple<field<bool>, bool> x{{true}, false};
    details::self(get<0>(x), 2) = false;
    x = !x;
    EXPECT_EQ(std::vector<device_t>({2}), joined_domain(x));
    EXPECT_EQ(make_tuple(true,true), details::self(x, 2));
    EXPECT_EQ(make_tuple(false,true), details::other(x));
}

TEST_F(FieldTest, BinaryOperators) {
    field<bool> eq;
    eq = (fi1 + fi2) == build_field(3, {{1,5},{2,5},{3,0}});
    EXPECT_TRUE(eq);
    eq = (fi1 * 2) == build_field(4, {{1,2},{3,-2}});
    EXPECT_TRUE(eq);
    eq = (2 * fi1) == build_field(4, {{1,2},{3,-2}});
    EXPECT_TRUE(eq);
    eq = (1 << fi2) == build_field(2, {{1,16},{2,8}});
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
    eq = (fi2 % 2) == build_field(1, {{1,0},{2,1}});
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
    EXPECT_EQ(std::vector<device_t>({0,1,2}), joined_domain(z));
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
    EXPECT_EQ(fi2, build_field(1, {{1,0},{2,1}}));
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
    EXPECT_EQ(std::vector<device_t>({0,1,2}), joined_domain(z));
    EXPECT_EQ(make_tuple(1,0.5), details::self(z, 0));
    EXPECT_EQ(make_tuple(4,1.5), details::self(z, 1));
    EXPECT_EQ(make_tuple(2,1.0), details::self(z, 2));
    EXPECT_EQ(make_tuple(1,0.5), details::other(z));
}
