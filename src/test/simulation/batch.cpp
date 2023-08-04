// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/mutex.hpp"
#include "lib/common/ostream.hpp"
#include "lib/simulation/batch.hpp"

#include "test/helper.hpp"

using namespace fcpp;

struct tag {};
struct gat {};
struct oth {};
struct hto {};

common::mutex<true> m;
std::vector<std::string> v;

// slow computation
int workhard(int& t, int n=30) {
    if (n <= 1) return 1;
    t += 1;
    int r = (workhard(t, n-1) + workhard(t, n-2))/2;
    t -= 1;
    return r;
}

#ifdef FCPP_DISABLE_THREADS
#define EXPECT_NEQ(a, b)    EXPECT_EQ(a, b)
#else
#define EXPECT_NEQ(a, b)    EXPECT_NE(a, b)
#endif


struct combomock {
    struct net {
        template <typename T>
        net(T const& t) {
            int tmp = 0;
            tmp += workhard(tmp);
            std::stringstream s;
            s << t;
            common::lock_guard<true> l(m);
            if (tmp == 1)
                v.push_back(s.str());
        }

        void run() {}
    };
};

template <typename... Ts>
struct genericombok : public combomock {};


TEST(BatchTest, Lists) {
    {
        using tuple_type = common::tagged_tuple_t<tag, int, gat, char>;
        auto x = batch::constant<tag, gat>(2, 'x');
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 0);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(2, 'x'));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::list<tag>(2, 3, 5, 9);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 4);
        EXPECT_EQ(x.extra_size(), 0);
        tuple_type t;
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(5));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, std::string>;
        auto x = batch::list<tag>("ciao", "pippo");
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 2);
        EXPECT_EQ(x.extra_size(), 0);
        tuple_type t;
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type("pippo"));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::double_list<tag,int>({2, 3, 5}, {4, 6});
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 3);
        EXPECT_EQ(x.extra_size(), 2);
        tuple_type t;
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(3));
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(6));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, double>;
        auto x = batch::arithmetic<tag>(2.0, 5.0, 0.5);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 7);
        EXPECT_EQ(x.extra_size(), 0);
        tuple_type t;
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(4.0));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, double>;
        auto x = batch::arithmetic<tag>(2.0, 4.0, 0.5, 2.0);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 4);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(2.0));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(2.5));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(3.0));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(3.5));
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(4.0));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, double>;
        auto x = batch::arithmetic<tag>(2.0, 4.0, 0.5, 3.0);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 4);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(3.0));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(2.0));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(2.5));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(3.5));
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(4.0));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, double>;
        auto x = batch::arithmetic<tag>(2.0, 4.0, 0.5, 3.25);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 5);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(3.25));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(2.0));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(2.5));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(3.0));
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(3.5));
        EXPECT_TRUE(x(t, 5));
        EXPECT_EQ(t, tuple_type(4.0));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, double>;
        auto x = batch::arithmetic<tag>(2.0, 4.0, 0.5, 4.0);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 4);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(4.0));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(2.0));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(2.5));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(3.0));
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(3.5));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, double>;
        auto x = batch::arithmetic<tag>(2.0, 3.0, 0.5, 3.25, 3.75);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 2);
        EXPECT_EQ(x.extra_size(), 3);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(3.25));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(3.75));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(2.0));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(2.5));
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(3.0));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::geometric<tag>(1, 100, 2);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 7);
        EXPECT_EQ(x.extra_size(), 0);
        tuple_type t;
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(8));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::geometric<tag>(1, 8, 2, 1);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 3);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(1));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(2));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(4));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(8));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::geometric<tag>(1, 8, 2, 4);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 3);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(4));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(1));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(2));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(8));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::geometric<tag>(1, 8, 2, 3);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 4);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(3));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(1));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(2));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(4));
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(8));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::geometric<tag>(1, 8, 2, 8);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 3);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(8));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(1));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(2));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(4));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::geometric<tag>(1, 4, 2, 3, 6);
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 2);
        EXPECT_EQ(x.extra_size(), 3);
        tuple_type t;
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, tuple_type(3));
        EXPECT_TRUE(x(t, 1));
        EXPECT_EQ(t, tuple_type(6));
        EXPECT_TRUE(x(t, 2));
        EXPECT_EQ(t, tuple_type(1));
        EXPECT_TRUE(x(t, 3));
        EXPECT_EQ(t, tuple_type(2));
        EXPECT_TRUE(x(t, 4));
        EXPECT_EQ(t, tuple_type(4));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        auto x = batch::recursive<tag>(0, [](size_t i, int prev) -> common::option<int> {
            if (i == 0) return 15;
            if (prev == 1) return {};
            return prev%2 ? 3*prev+1 : prev/2;
        });
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 18);
        EXPECT_EQ(x.extra_size(), 0);
        tuple_type t;
        EXPECT_TRUE(x(t, 5));
        EXPECT_EQ(t, tuple_type(106));
    }
}

TEST(BatchTest, Formulas) {
    {
        using tuple_type = common::tagged_tuple_t<tag, int>;
        using extra_type = common::tagged_tuple_t<gat, int, tag, int>;
        auto x = batch::formula<tag, int>([](auto const& t) {
            return common::get<gat>(t) * 2;
        });
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 0);
        extra_type t(10, 0);
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, extra_type(10, 20));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, std::string>;
        using extra_type = common::tagged_tuple_t<gat, int, oth, char, tag, std::string>;
        auto x = batch::stringify<tag>();
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 0);
        extra_type t(42, 'a', "");
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, extra_type(42, 'a', "gat-42_oth-a"));
    }
    {
        using tuple_type = common::tagged_tuple_t<tag, std::string>;
        using extra_type = common::tagged_tuple_t<gat, double, oth, bool, tag, std::string>;
        auto x = batch::stringify<tag>("experiment", "txt");
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 0);
        extra_type t(4.25, true, "");
        EXPECT_TRUE(x(t, 0));
        EXPECT_EQ(t, extra_type(4.25, true, "experiment_gat-4.25_oth-true.txt"));
    }
    {
        using tuple_type = common::tagged_tuple_t<>;
        using extra_type = common::tagged_tuple_t<tag, int, gat, int>;
        auto x = batch::filter([](auto const& t){
            return common::get<tag>(t) < common::get<gat>(t);
        });
        EXPECT_SAME(typename decltype(x)::value_type, tuple_type);
        EXPECT_EQ(x.core_size(), 1);
        EXPECT_EQ(x.extra_size(), 0);
        extra_type t;
        t = {2, 3};
        EXPECT_FALSE(x(t, 0));
        t = {4, 4};
        EXPECT_TRUE(x(t, 0));
        t = {4, 2};
        EXPECT_TRUE(x(t, 0));
    }
}

template <typename G>
auto generator_to_vector(G const& g) {
    std::vector<typename G::value_type> v;
    for (int i=0; i<g.size(); ++i) if (g.count(i)) v.push_back(g[i]);
    return v;
}

TEST(BatchTest, TupleSequence) {
    using namespace batch;
    auto x1 = generator_to_vector(make_tagged_tuple_sequence(list<char>(1,2,5)));
    std::vector<common::tagged_tuple_t<char, int>> v1;
    EXPECT_SAME(decltype(x1), decltype(v1));
    v1.emplace_back(1);
    v1.emplace_back(2);
    v1.emplace_back(5);
    EXPECT_EQ(x1, v1);
    auto x2 = generator_to_vector(make_tagged_tuple_sequence(arithmetic<char>(1,3,1), geometric<double>(3,20,2)));
    std::vector<common::tagged_tuple_t<char, int, double, int>> v2;
    EXPECT_SAME(decltype(x2), decltype(v2));
    v2.emplace_back(1,3);
    v2.emplace_back(1,6);
    v2.emplace_back(1,12);
    v2.emplace_back(2,3);
    v2.emplace_back(2,6);
    v2.emplace_back(2,12);
    v2.emplace_back(3,3);
    v2.emplace_back(3,6);
    v2.emplace_back(3,12);
    EXPECT_EQ(x2, v2);
    auto g3 = make_tagged_tuple_sequence(list<char>(1,7,3), list<double>(2,4), formula<short,int>([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), stringify<bool>());
    auto x3 = generator_to_vector(g3);
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string>> v3;
    EXPECT_SAME(decltype(x3), decltype(v3));
    v3.emplace_back(1,2,+1,"char-1_double-2_short-1");
    v3.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v3.emplace_back(7,2,-5,"char-7_double-2_short--5");
    v3.emplace_back(7,4,-3,"char-7_double-4_short--3");
    v3.emplace_back(3,2,-1,"char-3_double-2_short--1");
    v3.emplace_back(3,4,+1,"char-3_double-4_short-1");
    EXPECT_EQ(x3, v3);
    auto x4 = generator_to_vector(make_tagged_tuple_sequence(list<char>(1,7,3), list<double>(2,4), formula<short,int>([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), batch::filter([](auto const& tup){
        return common::get<short>(tup) < 0;
    }), stringify<bool>()));
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string>> v4;
    EXPECT_SAME(decltype(x4), decltype(v4));
    v4.emplace_back(1,2,+1,"char-1_double-2_short-1");
    v4.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v4.emplace_back(3,4,+1,"char-3_double-4_short-1");
    EXPECT_EQ(x4, v4);
    auto x5 = generator_to_vector(make_tagged_tuple_sequence(list<char>(1,7,3), list<double>(2,4), formula<short,int>([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), batch::filter([](auto const& tup){
        return common::get<short>(tup) < 0;
    }), stringify<bool>(), constant<void,long>('a', 7)));
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string, void, char, long, int>> v5;
    EXPECT_SAME(decltype(x5), decltype(v5));
    v5.emplace_back(1,2,+1,"char-1_double-2_short-1",'a',7);
    v5.emplace_back(1,4,+3,"char-1_double-4_short-3",'a',7);
    v5.emplace_back(3,4,+1,"char-3_double-4_short-1",'a',7);
    EXPECT_EQ(x5, v5);
}

TEST(BatchTest, TupleSequences) {
    using namespace batch;
    auto g1 = make_tagged_tuple_sequence(list<char>(1,7,3), list<double>(2,4), formula<short,int>([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), stringify<bool>());
    auto g2 = make_tagged_tuple_sequence(list<char>(1,7,3), list<double>(2,4), formula<short,int>([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), batch::filter([](auto const& tup){
        return common::get<short>(tup) < 0;
    }), stringify<bool>());
    auto gtot = make_tagged_tuple_sequences(g1, g2);
    auto x = generator_to_vector(gtot);
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string>> v;
    v.emplace_back(1,2,+1,"char-1_double-2_short-1");
    v.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v.emplace_back(7,2,-5,"char-7_double-2_short--5");
    v.emplace_back(7,4,-3,"char-7_double-4_short--3");
    v.emplace_back(3,2,-1,"char-3_double-2_short--1");
    v.emplace_back(3,4,+1,"char-3_double-4_short-1");
    v.emplace_back(1,2,+1,"char-1_double-2_short-1");
    v.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v.emplace_back(3,4,+1,"char-3_double-4_short-1");
    EXPECT_EQ(x, v);
    gtot.slice(1,7,2);
    x = generator_to_vector(gtot);
    v = {};
    v.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v.emplace_back(7,4,-3,"char-7_double-4_short--3");
    v.emplace_back(3,4,+1,"char-3_double-4_short-1");
    EXPECT_EQ(x, v);
    gtot.slice(0,-1);
    x = generator_to_vector(gtot);
    v = {};
    v.emplace_back(1,2,+1,"char-1_double-2_short-1");
    v.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v.emplace_back(7,2,-5,"char-7_double-2_short--5");
    v.emplace_back(7,4,-3,"char-7_double-4_short--3");
    v.emplace_back(3,2,-1,"char-3_double-2_short--1");
    v.emplace_back(3,4,+1,"char-3_double-4_short-1");
    v.emplace_back(1,2,+1,"char-1_double-2_short-1");
    v.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v.emplace_back(3,4,+1,"char-3_double-4_short-1");
    EXPECT_EQ(x, v);
    gtot.shuffle();
}

TEST(BatchTest, Run) {
    std::vector<std::string> w;
    v = {};
    batch::run(combomock{});
    EXPECT_EQ(v, w);
    v = {};
    w = {};
    batch::run(combomock{}, common::tags::sequential_execution{}, batch::make_tagged_tuple_sequence(batch::list<char>(1,2,5,8), batch::list<double>(2,7)), batch::make_tagged_tuple_sequence(batch::list<double>(3,0,6), batch::list<char>(1,2,4)));
    w.push_back("(type_index => 0; char => 1; double => 2)");
    w.push_back("(type_index => 0; char => 1; double => 7)");
    w.push_back("(type_index => 0; char => 2; double => 2)");
    w.push_back("(type_index => 0; char => 2; double => 7)");
    w.push_back("(type_index => 0; char => 5; double => 2)");
    w.push_back("(type_index => 0; char => 5; double => 7)");
    w.push_back("(type_index => 0; char => 8; double => 2)");
    w.push_back("(type_index => 0; char => 8; double => 7)");
    w.push_back("(type_index => 0; char => 1; double => 3)");
    w.push_back("(type_index => 0; char => 2; double => 3)");
    w.push_back("(type_index => 0; char => 4; double => 3)");
    w.push_back("(type_index => 0; char => 1; double => 0)");
    w.push_back("(type_index => 0; char => 2; double => 0)");
    w.push_back("(type_index => 0; char => 4; double => 0)");
    w.push_back("(type_index => 0; char => 1; double => 6)");
    w.push_back("(type_index => 0; char => 2; double => 6)");
    w.push_back("(type_index => 0; char => 4; double => 6)");
    EXPECT_EQ(v, w);
    v = {};
    batch::run(combomock{}, common::tags::parallel_execution{17}, batch::make_tagged_tuple_sequence(batch::list<char>(1,2,5,8), batch::list<double>(2,7)), batch::make_tagged_tuple_sequence(batch::list<double>(3,0,6), batch::list<char>(1,2,4)));
    EXPECT_NEQ(v, w);
    std::sort(v.begin(), v.end());
    std::sort(w.begin(), w.end());
    EXPECT_EQ(v, w);
    v = {};
    batch::run(combomock{}, batch::make_tagged_tuple_sequence(batch::list<char>(1,2,5,8), batch::list<double>(2,7)), batch::make_tagged_tuple_sequence(batch::list<double>(3,0,6), batch::list<char>(1,2,4)));
    std::sort(v.begin(), v.end());
    EXPECT_EQ(v, w);
    v = {};
    w = {};
}

TEST(BatchTest, Options) {
    using namespace batch;
    using types = option_combine<genericombok, void, options<int, bool>, common::type_sequence<char,long>, options<common::type_sequence<double>, short>>;
    EXPECT_SAME(types, common::type_sequence<genericombok<void, int, char, long, double>, genericombok<void, bool, char, long, double>, genericombok<void, int, char, long, short>, genericombok<void, bool, char, long, short>>);
    v = {};
    batch::run(types{}, batch::make_tagged_tuple_sequence(batch::list<char>(1,2,5), batch::list<double>(2)), batch::make_tagged_tuple_sequence(batch::list<double>(3,0), batch::list<char>(1,2)));
    EXPECT_EQ(v.size(), 28ULL);
    v = {};
    batch::run(types{}, common::tags::sequential_execution{}, batch::make_tagged_tuple_sequence(batch::list<char>(1,2,5), batch::list<double>(2,1)));
    EXPECT_EQ(v.size(), 24ULL);
}
