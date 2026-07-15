// Copyright © 2025 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/multitype_map.hpp"
#include "lib/common/option.hpp"
#include "lib/common/ostream.hpp"
#include "lib/common/plot.hpp"
#include "lib/common/serialize.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/data/bloom.hpp"
#include "lib/data/field.hpp"
#include "lib/data/hyperloglog.hpp"
#include "lib/data/tuple.hpp"
#include "lib/data/vec.hpp"
#include "lib/internal/flat_ptr.hpp"
#include "lib/option/aggregator.hpp"

using namespace fcpp;

struct tag {};
struct gat {};


template <typename T>
void rebuilder(T& y, T& z) {
    T const& x{y};
    common::osstream os, osx;
    os << y;
    osx << x;
    EXPECT_EQ((std::vector<char>)os, (std::vector<char>)osx);
    common::isstream is(os);
    is >> z;
}

template <typename T>
std::tuple<T,T,int,int,int> rebuild(T y, T z, bool h) {
    int h1 = 0, h2 = 1, h3 = 0;
    rebuilder(y, z);
    return {y, z, h1, h2, h3};
}

template <typename T>
std::tuple<T,T,int,int,int> rebuild(T y, T z) {
    common::hstream hs;
    int h1 = int(hs << y);
    hs = {};
    int h2 = y == z ? h1+1 : int(hs << z);
    rebuilder(y, z);
    hs = {};
    int h3 = int(hs << z);
    return {y, z, h1, h2, h3};
}

size_t rebuild_size(size_t y) {
    size_t z;
    common::osstream os;
    common::details::size_variable_write(os, y);
    common::isstream is(os);
    common::details::size_variable_read(is, z);
    return z;
}

#define SERIALIZE_CHECK(x, ...) {                       \
            auto result = rebuild(x, __VA_ARGS__);      \
            EXPECT_EQ(x, get<0>(result));               \
            EXPECT_EQ(x, get<1>(result));               \
            EXPECT_EQ(get<2>(result), get<4>(result));  \
            EXPECT_NE(get<2>(result), get<3>(result));  \
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
    std::multiset<int> ms = {1, 2, 4, 8};
    SERIALIZE_CHECK(ms, {});
    std::set<int> s = {1, 2, 4, 8};
    SERIALIZE_CHECK(s, {});
    std::unordered_multiset<trace_t> mt = {1, 2, 4, 8};
    SERIALIZE_CHECK(mt, {}, false);
    std::unordered_set<trace_t> t = {1, 2, 4, 8};
    SERIALIZE_CHECK(t, {}, false);
    std::map<trace_t,double> m = {{4, 2}, {42, 2.4}};
    SERIALIZE_CHECK(m, {});
    std::unordered_map<int,double> n = {{4, 2}, {42, 2.4}};
    SERIALIZE_CHECK(n, {}, false);
    std::tuple<std::unordered_map<int, std::pair<std::vector<char>, short>>, double> y = {
        {{2, {{}, 1}}, {3, {{2,3,4}, 2}}}, 4.2
    };
    SERIALIZE_CHECK(y, {}, false);
    std::unordered_map<std::tuple<int,bool>, int, common::hash<std::tuple<int,bool>>> u;
    u[std::make_tuple(4,false)] = 2;
    SERIALIZE_CHECK(u, {}, false);
    std::string str = "thestring";
    SERIALIZE_CHECK(str, {});
    common::option<std::array<int,2>, false> o0{};
    SERIALIZE_CHECK(o0, {});
    common::option<std::array<int,2>, true> o1{std::array<int,2>{}};
    SERIALIZE_CHECK(o1, {});
    common::option<std::array<int,2>> o20{}, o21{std::array<int,2>{}};
    SERIALIZE_CHECK(o20, {});
    SERIALIZE_CHECK(o21, {});
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
    bloom_filter<2, 128> bf{1, 6, 10, 27};
    SERIALIZE_CHECK(bf, {});
    hyperloglog_counter<64> hll{1, 6, 10, 27};
    SERIALIZE_CHECK(hll, {});
    common::tagged_tuple_t<void,bool,char,int> tt{true,42};
    SERIALIZE_CHECK(tt, {});
    common::multitype_map<trace_t, bool, char, int> m;
    m.insert(42);
    m.insert(10);
    m.insert(1, false);
    m.insert(2, 'z');
    m.insert(3, 'x');
    m.insert(4, 4242);
    SERIALIZE_CHECK(m, {}, false);
    internal::flat_ptr<int, true> p{42};
    SERIALIZE_CHECK(p, {});
    internal::flat_ptr<int, false> q{42};
    SERIALIZE_CHECK(q, {});
    internal::flat_ptr<common::multitype_map<trace_t, double, field<bool>>, false> e;
    e->insert(42);
    e->insert(10);
    e->insert(1, 4.2);
    e->insert(3, details::make_field<bool>({2, 4}, {false, true, true}));
    SERIALIZE_CHECK(e, {}, false);
    std::unordered_map<tuple<int,bool>, int, common::hash<tuple<int,bool>>> u;
    u[make_tuple(4,false)] = 2;
    SERIALIZE_CHECK(u, {}, false);
}

TEST(SerializeTest, Aggregators) {
    aggregator::count<bool> count;
    count.insert(false);
    count.insert(true);
    count.insert(true);
    SERIALIZE_CHECK(count, {});
    aggregator::distinct<int> distinct;
    distinct.insert(4);
    distinct.insert(2);
    distinct.insert(4);
    SERIALIZE_CHECK(distinct, {}, false);
    aggregator::list<int> list;
    list.insert(4);
    list.insert(2);
    list.insert(4);
    SERIALIZE_CHECK(list, {});
    aggregator::sum<int> sum;
    sum.insert(4);
    sum.insert(2);
    sum.insert(4);
    SERIALIZE_CHECK(sum, {});
    aggregator::only_finite<aggregator::mean<double>> mean;
    mean.insert(4);
    mean.insert(2);
    mean.insert(4);
    SERIALIZE_CHECK(mean, {});
    aggregator::moment<double,3> moment;
    moment.insert(4);
    moment.insert(2);
    moment.insert(4);
    SERIALIZE_CHECK(moment, {});
    aggregator::deviation<double> deviation;
    deviation.insert(4);
    deviation.insert(2);
    deviation.insert(4);
    SERIALIZE_CHECK(deviation, {});
    aggregator::deviation<double> stats;
    stats.insert(4);
    stats.insert(2);
    stats.insert(4);
    SERIALIZE_CHECK(stats, {});
    aggregator::min<int> min;
    min.insert(4);
    min.insert(2);
    min.insert(4);
    SERIALIZE_CHECK(min, {});
    aggregator::max<int> max;
    max.insert(4);
    max.insert(2);
    max.insert(4);
    SERIALIZE_CHECK(max, {});
    aggregator::quantile<double,false,50> quantilefalse;
    quantilefalse.insert(4);
    quantilefalse.insert(2);
    quantilefalse.insert(4);
    SERIALIZE_CHECK(quantilefalse, {}, false);
    aggregator::quantile<double,true,50> quantiletrue;
    quantiletrue.insert(4);
    quantiletrue.insert(2);
    quantiletrue.insert(4);
    SERIALIZE_CHECK(quantiletrue, {});
    aggregator::combine<aggregator::min<int>, aggregator::max<int>> combine;
    combine.insert(4);
    combine.insert(2);
    combine.insert(4);
    SERIALIZE_CHECK(combine, {});
}

TEST(SerializeTest, Plots) {
    plot::none none;
    none << common::make_tagged_tuple<tag,gat>(4,2);
    none << common::make_tagged_tuple<tag,gat>(2,4);
    SERIALIZE_CHECK(none, {}, false);
    plot::value<tag> value;
    value << common::make_tagged_tuple<tag,gat>(4,2);
    value << common::make_tagged_tuple<tag,gat>(2,4);
    SERIALIZE_CHECK(value, {});
    plot::filter<gat, filter::above<1>, plot::value<tag>> filter;
    filter << common::make_tagged_tuple<tag,gat>(4,2);
    filter << common::make_tagged_tuple<tag,gat>(2,4);
    SERIALIZE_CHECK(filter, {});
    plot::join<plot::value<tag>, plot::filter<gat, filter::above<1>, plot::none>> join;
    join << common::make_tagged_tuple<tag,gat>(4,2);
    join << common::make_tagged_tuple<tag,gat>(2,4);
    SERIALIZE_CHECK(join, {});
    plot::split<gat, plot::join<plot::value<tag>, plot::filter<gat, filter::above<1>, plot::none>>> split;
    split << common::make_tagged_tuple<tag,gat>(4,2);
    split << common::make_tagged_tuple<tag,gat>(2,4);
    SERIALIZE_CHECK(split, {});
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
