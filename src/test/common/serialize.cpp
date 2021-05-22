// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/multitype_map.hpp"
#include "lib/common/ostream.hpp"
#include "lib/common/serialize.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/data/field.hpp"
#include "lib/data/vec.hpp"
#include "lib/internal/flat_ptr.hpp"

using namespace fcpp;


template <typename T>
std::pair<T,T> rebuild(T y, T z) {
    common::osstream os;
    os << y;
    common::isstream is(os);
    is >> z;
    return {y, z};
}

size_t rebuild_size(size_t y) {
    size_t z;
    common::osstream os;
    common::details::size_variable_write(os, y);
    common::isstream is(os);
    common::details::size_variable_read(is, z);
    return z;
}

#define SERIALIZE_CHECK(x, null) {          \
            auto result = rebuild(x, null); \
            EXPECT_EQ(x, get<0>(result));   \
            EXPECT_EQ(x, get<1>(result));   \
        }


TEST(SerializeTest, Trivial) {
    int x = 42;
    SERIALIZE_CHECK(x, 0);
    double y = 4.2;
    SERIALIZE_CHECK(y, 0.0);
    char s[4] = "boh", t[4] = "meh";
    common::osstream os;
    os << s;
    common::isstream is(os);
    is >> t;
    EXPECT_EQ(std::string(s), "boh");
    EXPECT_EQ(std::string(t), "boh");
    struct base {
        size_t a;
        float b[2];
        bool operator==(base const& o) const {
            return a == o.a and b[0] == o.b[0] and b[1] == o.b[1];
        }
    } z;
    z.a = 1;
    z.b[0] = 2.5;
    z.b[1] = 3.25;
    SERIALIZE_CHECK(z, {});
}

TEST(SerializeTest, Indexed) {
    std::pair<int,int> x{4,2};
    SERIALIZE_CHECK(x, {});
    std::tuple<int,double,long long> y{4,2.4,2};
    SERIALIZE_CHECK(y, {});
    std::array<int,2> v = {4, 2};
    SERIALIZE_CHECK(v, {});
    std::pair<std::array<int,2>, std::tuple<int,double,long long>> z{v, y};
    SERIALIZE_CHECK(z, {});
}

TEST(SerializeTest, Iterable) {
    EXPECT_EQ(15ULL,     rebuild_size(15));
    EXPECT_EQ(3058ULL,   rebuild_size(3058));
    EXPECT_EQ(958102ULL, rebuild_size(958102));
    EXPECT_EQ(7646860119211199969ULL, rebuild_size(7646860119211199969LL));
    std::vector<int> x = {1, 2, 4, 8};
    SERIALIZE_CHECK(x, {});
    std::set<int> s = {1, 2, 4, 8};
    SERIALIZE_CHECK(s, {});
    std::unordered_set<trace_t> t = {1, 2, 4, 8};
    SERIALIZE_CHECK(t, {});
    std::map<trace_t,double> m = {{4, 2}, {42, 2.4}};
    SERIALIZE_CHECK(m, {});
    std::unordered_map<int,double> n = {{4, 2}, {42, 2.4}};
    SERIALIZE_CHECK(n, {});
    std::tuple<std::unordered_map<int, std::pair<std::vector<char>, short>>, double> y = {
        {{2, {{}, 1}}, {3, {{2,3,4}, 2}}}, 4.2
    };
    SERIALIZE_CHECK(y, {});
}

TEST(SerializeTest, FCPP) {
    field<int> f = details::make_field<int>({1,2}, {0,2,3});
    SERIALIZE_CHECK(f, {0});
    field<bool> g = details::make_field<bool>({2, 4}, {false, true, true});
    SERIALIZE_CHECK(g, {false});
    tuple<int, double> t{4, 2.4};
    SERIALIZE_CHECK(t, {});
    vec<3> v{1.0, 2.5, 5.25};
    SERIALIZE_CHECK(v, {});
    common::tagged_tuple_t<void,bool,char,int> tt{true,42};
    SERIALIZE_CHECK(tt, {});
    common::multitype_map<trace_t, bool, char, int> m;
    m.insert(42);
    m.insert(10);
    m.insert(1, false);
    m.insert(2, 'z');
    m.insert(3, 'x');
    m.insert(4, 4242);
    SERIALIZE_CHECK(m, {});
    internal::flat_ptr<int, true> p{42};
    SERIALIZE_CHECK(p, {});
    internal::flat_ptr<int, false> q{42};
    SERIALIZE_CHECK(q, {});
    internal::flat_ptr<common::multitype_map<trace_t, double, field<bool>>, false> e;
    e->insert(42);
    e->insert(10);
    e->insert(1, 4.2);
    e->insert(3, details::make_field<bool>({2, 4}, {false, true, true}));
    SERIALIZE_CHECK(e, {});
}

TEST(SerializeTest, Error) {
    std::string s = "hello world";
    std::vector<char> v;
    for (size_t i=0; i<s.size(); ++i) v.push_back(s[i]);
    common::isstream is(v);
    common::multitype_map<trace_t, bool, char, int> m;
    try {
        is >> m;
        EXPECT_TRUE(false);
    } catch (common::format_error& e) {
        EXPECT_STREQ(e.what(), "format error in deserialisation");
    }
}
