// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/mutex.hpp"
#include "lib/simulation/batch.hpp"

#include "test/helper.hpp"

using namespace fcpp;

common::mutex<true> m;
std::vector<std::string> v;

int veryexpensivefunction(int x) {
    if (x < 2) return 1;
    return (veryexpensivefunction(x-1) + veryexpensivefunction(x-2))/2;
}

struct combomock {
    struct net {
        template <typename T>
        net(T const& t) {
            veryexpensivefunction(38);
            std::stringstream s;
            s << t;
            common::lock_guard<true> l(m);
            v.push_back(s.str());
        }
        
        void run() {}
    };
};


TEST(BatchTest, Lists) {
    auto x1 = batch::list(2, 3, 5, 9)(nullptr);
    EXPECT_SAME(decltype(x1), std::array<int, 4>);
    std::array<int, 4> v1{2, 3, 5, 9};
    EXPECT_EQ(x1, v1);
    auto x2 = batch::list("ciao", "pippo")(nullptr);
    EXPECT_SAME(decltype(x2), std::array<std::string, 2>);
    std::array<std::string, 2> v2{"ciao", "pippo"};
    EXPECT_EQ(x2, v2);
    auto x3 = batch::arithmetic(2.0, 5.1, 0.5)(nullptr);
    EXPECT_SAME(decltype(x3), std::vector<double>);
    std::vector<double> v3{2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0};
    EXPECT_EQ(x3, v3);
    auto x4 = batch::geometric(1, 100, 2)(nullptr);
    EXPECT_SAME(decltype(x4), std::vector<int>);
    std::vector<int> v4{1, 2, 4, 8, 16, 32, 64};
    EXPECT_EQ(x4, v4);
    auto x5 = batch::recursive(0, [](size_t i, int prev, auto const& tup) -> batch::option<int> {
        if (i == 0) return tup;
        if (prev == 1) return {};
        return prev%2 ? 3*prev+1 : prev/2;
    })(15);
    EXPECT_SAME(decltype(x5), std::vector<int>);
    std::vector<int> v5{15, 46, 23, 70, 35, 106, 53, 160, 80, 40, 20, 10, 5, 16, 8, 4, 2, 1};
    EXPECT_EQ(x5, v5);
}

TEST(BatchTest, Formulas) {
    auto x1 = batch::formula([](auto const& t){ return get<0>(t)+1; })(std::make_tuple(4,2));
    EXPECT_SAME(decltype(x1), std::array<int, 1>);
    std::array<int, 1> v1{5};
    EXPECT_EQ(x1, v1);
    auto x2 = batch::stringify()(common::make_tagged_tuple<void,char>(2,5.5));
    EXPECT_SAME(decltype(x2), std::array<std::string, 1>);
    std::array<std::string, 1> v2{"void-2_char-5.5"};
    EXPECT_EQ(x2, v2);
    x2 = batch::stringify("experiment", "txt")(common::make_tagged_tuple<void,char>(2,5.5));
    v2 = {"experiment_void-2_char-5.5.txt"};
    EXPECT_EQ(x2, v2);
}

TEST(BatchTest, TupleSequence) {
    using namespace batch;
    auto x1 = make_tagged_tuple_sequence(char{}, list(1,2,5));
    std::vector<common::tagged_tuple_t<char, int>> v1;
    EXPECT_SAME(decltype(x1), decltype(v1));
    v1.push_back(common::make_tagged_tuple<char>(1));
    v1.push_back(common::make_tagged_tuple<char>(2));
    v1.push_back(common::make_tagged_tuple<char>(5));
    EXPECT_EQ(x1, v1);
    auto x2 = make_tagged_tuple_sequence(char{}, arithmetic(1,3,1), double{}, geometric(3,20,2));
    std::vector<common::tagged_tuple_t<char, int, double, int>> v2;
    EXPECT_SAME(decltype(x2), decltype(v2));
    v2.push_back(common::make_tagged_tuple<char,double>(1,3));
    v2.push_back(common::make_tagged_tuple<char,double>(1,6));
    v2.push_back(common::make_tagged_tuple<char,double>(1,12));
    v2.push_back(common::make_tagged_tuple<char,double>(2,3));
    v2.push_back(common::make_tagged_tuple<char,double>(2,6));
    v2.push_back(common::make_tagged_tuple<char,double>(2,12));
    v2.push_back(common::make_tagged_tuple<char,double>(3,3));
    v2.push_back(common::make_tagged_tuple<char,double>(3,6));
    v2.push_back(common::make_tagged_tuple<char,double>(3,12));
    EXPECT_EQ(x2, v2);
    auto x3 = make_tagged_tuple_sequence(char{}, list(1,7,3), double{}, list(2,4), short{}, formula([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), bool{}, stringify());
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string>> v3;
    EXPECT_SAME(decltype(x3), decltype(v3));
    v3.push_back(common::make_tagged_tuple<char,double,short,bool>(1,2,+1,"char-1_double-2_short-1"));
    v3.push_back(common::make_tagged_tuple<char,double,short,bool>(1,4,+3,"char-1_double-4_short-3"));
    v3.push_back(common::make_tagged_tuple<char,double,short,bool>(7,2,-5,"char-7_double-2_short--5"));
    v3.push_back(common::make_tagged_tuple<char,double,short,bool>(7,4,-3,"char-7_double-4_short--3"));
    v3.push_back(common::make_tagged_tuple<char,double,short,bool>(3,2,-1,"char-3_double-2_short--1"));
    v3.push_back(common::make_tagged_tuple<char,double,short,bool>(3,4,+1,"char-3_double-4_short-1"));
    EXPECT_EQ(x3, v3);
    auto x4 = make_tagged_tuple_sequence(char{}, list(1,7,3), double{}, list(2,4), short{}, formula([](auto const& tup){
        return common::get<double>(tup) - common::get<char>(tup);
    }), tags::filter{}, [](auto const& tup){
        return common::get<short>(tup) < 0;
    }, bool{}, stringify());
    std::vector<common::tagged_tuple_t<char, int, double, int, short, int, bool, std::string>> v4;
    EXPECT_SAME(decltype(x4), decltype(v4));
    v4.push_back(common::make_tagged_tuple<char,double,short,bool>(1,2,+1,"char-1_double-2_short-1"));
    v4.push_back(common::make_tagged_tuple<char,double,short,bool>(1,4,+3,"char-1_double-4_short-3"));
    v4.push_back(common::make_tagged_tuple<char,double,short,bool>(3,4,+1,"char-3_double-4_short-1"));
    EXPECT_EQ(x4, v4);
}

TEST(BatchTest, Run) {
    v = {};
    batch::run<combomock>(common::tags::sequential_execution{}, batch::make_tagged_tuple_sequence(char{}, batch::list(1,2,5),double{}, batch::list(2)), batch::make_tagged_tuple_sequence(double{}, batch::list(3,0), char{}, batch::list(1,2)));
    std::vector<std::string> w;
    w.push_back("(char => 1; double => 2)");
    w.push_back("(char => 2; double => 2)");
    w.push_back("(char => 5; double => 2)");
    w.push_back("(char => 1; double => 3)");
    w.push_back("(char => 2; double => 3)");
    w.push_back("(char => 1; double => 0)");
    w.push_back("(char => 2; double => 0)");
    EXPECT_EQ(v, w);
    v = {};
    batch::run<combomock>(common::tags::parallel_execution{7}, batch::make_tagged_tuple_sequence(char{}, batch::list(1,2,5),double{}, batch::list(2)), batch::make_tagged_tuple_sequence(double{}, batch::list(3,0), char{}, batch::list(1,2)));
    EXPECT_NE(v, w);
    std::sort(v.begin(), v.end());
    std::sort(w.begin(), w.end());
    EXPECT_EQ(v, w);
}
