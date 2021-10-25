// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/mutex.hpp"
#include "lib/common/ostream.hpp"
#include "lib/simulation/batch.hpp"

#include "test/helper.hpp"

using namespace fcpp;

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
    auto x1 = batch::list<void>(2, 3, 5, 9)(nullptr);
    EXPECT_SAME(decltype(x1), std::array<int, 4>);
    std::array<int, 4> v1{2, 3, 5, 9};
    EXPECT_EQ(x1, v1);
    auto x2 = batch::literals<void>("ciao", "pippo")(nullptr);
    EXPECT_SAME(decltype(x2), std::array<std::string, 2>);
    std::array<std::string, 2> v2{"ciao", "pippo"};
    EXPECT_EQ(x2, v2);
    auto x3 = batch::arithmetic<void>(2.0, 5.1, 0.5)(nullptr);
    EXPECT_SAME(decltype(x3), std::vector<double>);
    std::vector<double> v3{2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0};
    EXPECT_EQ(x3, v3);
    auto x4 = batch::geometric<void>(1, 100, 2)(nullptr);
    EXPECT_SAME(decltype(x4), std::vector<int>);
    std::vector<int> v4{1, 2, 4, 8, 16, 32, 64};
    EXPECT_EQ(x4, v4);
    auto x5 = batch::recursive<void>(0, [](size_t i, int prev, auto const& tup) -> common::option<int> {
        if (i == 0) return tup;
        if (prev == 1) return {};
        return prev%2 ? 3*prev+1 : prev/2;
    })(15);
    EXPECT_SAME(decltype(x5), std::vector<int>);
    std::vector<int> v5{15, 46, 23, 70, 35, 106, 53, 160, 80, 40, 20, 10, 5, 16, 8, 4, 2, 1};
    EXPECT_EQ(x5, v5);
}

TEST(BatchTest, Formulas) {
    auto x1 = batch::formula<void>([](auto const& t){ return get<0>(t)+1; })(std::make_tuple(4,2));
    EXPECT_SAME(decltype(x1), std::array<int, 1>);
    std::array<int, 1> v1{5};
    EXPECT_EQ(x1, v1);
    auto x2 = batch::stringify<void>()(common::make_tagged_tuple<void,char>(2,5.5));
    EXPECT_SAME(decltype(x2), std::array<std::string, 1>);
    std::array<std::string, 1> v2{"void-2_char-5.5"};
    EXPECT_EQ(x2, v2);
    x2 = batch::stringify<void>("experiment", "txt")(common::make_tagged_tuple<void,char>(2,5.5));
    v2 = {"experiment_void-2_char-5.5.txt"};
    EXPECT_EQ(x2, v2);
}

TEST(BatchTest, TupleSequence) {
    using namespace batch;
    auto x1 = make_tagged_tuple_sequence(list<char>(1,2,5));
    std::vector<common::tagged_tuple_t<char, int>> v1;
    EXPECT_SAME(decltype(x1), decltype(v1));
    v1.emplace_back(1);
    v1.emplace_back(2);
    v1.emplace_back(5);
    EXPECT_EQ(x1, v1);
    auto x2 = make_tagged_tuple_sequence(arithmetic<char>(1,3,1), geometric<double>(3,20,2));
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
    auto x3 = make_tagged_tuple_sequence(list<char>(1,7,3), list<double>(2,4), formula<short>([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), stringify<bool>());
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string>> v3;
    EXPECT_SAME(decltype(x3), decltype(v3));
    v3.emplace_back(1,2,+1,"char-1_double-2_short-1");
    v3.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v3.emplace_back(7,2,-5,"char-7_double-2_short--5");
    v3.emplace_back(7,4,-3,"char-7_double-4_short--3");
    v3.emplace_back(3,2,-1,"char-3_double-2_short--1");
    v3.emplace_back(3,4,+1,"char-3_double-4_short-1");
    EXPECT_EQ(x3, v3);
    auto x4 = make_tagged_tuple_sequence(list<char>(1,7,3), list<double>(2,4), formula<short>([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), filter([](auto const& tup){
        return common::get<short>(tup) < 0;
    }), stringify<bool>());
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string>> v4;
    EXPECT_SAME(decltype(x4), decltype(v4));
    v4.emplace_back(1,2,+1,"char-1_double-2_short-1");
    v4.emplace_back(1,4,+3,"char-1_double-4_short-3");
    v4.emplace_back(3,4,+1,"char-3_double-4_short-1");
    EXPECT_EQ(x4, v4);
    auto x5 = make_tagged_tuple_sequence(list<char>(1,7,3), list<double>(2,4), formula<short>([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), filter([](auto const& tup){
        return common::get<short>(tup) < 0;
    }), stringify<bool>(), constant<void,long>('a', 7));
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string, void, char, long, int>> v5;
    EXPECT_SAME(decltype(x5), decltype(v5));
    v5.emplace_back(1,2,+1,"char-1_double-2_short-1",'a',7);
    v5.emplace_back(1,4,+3,"char-1_double-4_short-3",'a',7);
    v5.emplace_back(3,4,+1,"char-3_double-4_short-1",'a',7);
    EXPECT_EQ(x5, v5);
}

TEST(BatchTest, Run) {
    std::vector<std::string> w;
    v = {};
    batch::run(combomock{});
    w.push_back("()");
    EXPECT_EQ(v, w);
    v = {};
    w = {};
    batch::run(combomock{}, common::tags::sequential_execution{}, batch::make_tagged_tuple_sequence(batch::list<char>(1,2,5,8), batch::list<double>(2,7)), batch::make_tagged_tuple_sequence(batch::list<double>(3,0,6), batch::list<char>(1,2,4)));
    w.push_back("(char => 1; double => 2)");
    w.push_back("(char => 1; double => 7)");
    w.push_back("(char => 2; double => 2)");
    w.push_back("(char => 2; double => 7)");
    w.push_back("(char => 5; double => 2)");
    w.push_back("(char => 5; double => 7)");
    w.push_back("(char => 8; double => 2)");
    w.push_back("(char => 8; double => 7)");
    w.push_back("(char => 1; double => 3)");
    w.push_back("(char => 2; double => 3)");
    w.push_back("(char => 4; double => 3)");
    w.push_back("(char => 1; double => 0)");
    w.push_back("(char => 2; double => 0)");
    w.push_back("(char => 4; double => 0)");
    w.push_back("(char => 1; double => 6)");
    w.push_back("(char => 2; double => 6)");
    w.push_back("(char => 4; double => 6)");
    EXPECT_EQ(v, w);
    v = {};
    batch::run(combomock{}, common::tags::parallel_execution{17}, batch::make_tagged_tuple_sequence(batch::list<char>(1,2,5,8), batch::list<double>(2,7)), batch::make_tagged_tuple_sequence(batch::list<double>(3,0,6), batch::list<char>(1,2,4)));
    EXPECT_NEQ(v, w);
    std::sort(v.begin(), v.end());
    std::sort(w.begin(), w.end());
    EXPECT_EQ(v, w);
}

TEST(BatchTest, Options) {
    using namespace batch;
    using types = option_combine<genericombok, void, options<int, bool>, common::type_sequence<char,long>, options<common::type_sequence<double>, short>>;
    v = {};
    batch::run(types{}, batch::make_tagged_tuple_sequence(batch::list<char>(1,2,5), batch::list<double>(2)), batch::make_tagged_tuple_sequence(batch::list<double>(3,0), batch::list<char>(1,2)));
    EXPECT_EQ(v.size(), 28ULL);
}
