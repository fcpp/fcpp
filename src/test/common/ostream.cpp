// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <sstream>
#include <iomanip>

#include "gtest/gtest.h"

#include "lib/common/multitype_map.hpp"
#include "lib/common/ostream.hpp"
#include "lib/common/random_access_map.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/data/field.hpp"
#include "lib/data/tuple.hpp"
#include "lib/data/vec.hpp"
#include "lib/internal/context.hpp"
#include "lib/internal/flat_ptr.hpp"
#include "lib/internal/twin.hpp"

#define PRINT_EQ(x, ...)    ss << __VA_ARGS__; EXPECT_EQ(ss.str(), x); ss.str(""); EXPECT_EQ(to_string(__VA_ARGS__), x)

TEST(OstreamTest, STD) {
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed;
    using namespace std;
    PRINT_EQ("[4, 2, 0]", array<int,3>{4,2,0});
    PRINT_EQ("(10; 4.20)", pair<int,double>{10,4.2});
    PRINT_EQ("(10; 'x'; 4.20)", tuple<int,char,double>{10,'x',4.2});
    PRINT_EQ("[4, 2, 0, 10]", vector<int>{4,2,0,10});
    PRINT_EQ("{0, 2, 4, 10}", set<int>{4,2,0,10});
    PRINT_EQ("{42}", unordered_set<int>{42});
    PRINT_EQ("{2:\"world\", 4:\"hello\"}", map<int, std::string>{{4,"hello"},{2,"world"}});
    PRINT_EQ("{42:\"hello world\"}", unordered_map<int, std::string>{{42, "hello world"}});
}

TEST(OstreamTest, FCPP) {
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed;
    using namespace fcpp;
    PRINT_EQ("{0:false, 2:false, *:true}", details::make_field({0,1,2}, std::vector<bool>{true,false,true,false}));
    PRINT_EQ("{0:'y', 2:'z', *:'x'}", details::make_field({0,2}, std::vector<char>{'x','y','z'}));
    PRINT_EQ("[4.00, 2.00, 0.00]", vec<3>{4,2,0});
    PRINT_EQ("(false; 'a'; fcpp::common::type_sequence<void>)",
              tuple<bool,char,common::type_sequence<void>>{false,'a',{}});
    PRINT_EQ("2", internal::flat_ptr<int,true>{2});
    PRINT_EQ("true", internal::flat_ptr<bool,false>{true});
    common::multitype_map<trace_t,bool,char> m;
    m.insert(42, 'x');
    m.insert(10, false);
    PRINT_EQ("(bool => {10:false}; char => {42:'x'})", m);
    PRINT_EQ("{42:\"hello world\"}", common::random_access_map<int, std::string>{{42, "hello world"}});
    PRINT_EQ("(void => 3; int& => 'x')", common::make_tagged_tuple<void,int&>(3, 'x'));
    PRINT_EQ("(2)", internal::twin<int,true>{2});
    PRINT_EQ("(2; 2)", internal::twin<int,false>{2});
    {
        internal::context<true, true, int, bool, char> c;
        c.insert(42, m, 0, 10, 10);
        PRINT_EQ("(42:(bool => {10:false}; char => {42:'x'})@0)", c);
    }
    {
        internal::context<false, false, int, bool, char> c;
        c.insert(42, m, 0, 10, 10);
        PRINT_EQ("(42:(bool => {10:false}; char => {42:'x'})@0)", c);
    }
}

TEST(OstreamTest, Mixed) {
    std::stringstream ss;
    ss << std::setprecision(2) << std::fixed;
    PRINT_EQ("(42; (4.20; 2); \"42\")", fcpp::make_tuple(42, std::make_pair(4.20, fcpp::internal::flat_ptr<int,true>{2}), "42"));
    PRINT_EQ("([4.00, 2.00, 0.00]; {0:('y'; 1), 2:('z'; 2), *:('x'; 0)}; ([(4; '4'), (2; '2'), (0; '0')]))", std::make_tuple(fcpp::make_vec(4,2,0), fcpp::details::make_field({0,2}, std::vector<std::pair<char,int>>{{'x',0},{'y',1},{'z',2}}), fcpp::internal::twin<std::vector<fcpp::tuple<int, char>>, true>(std::vector<fcpp::tuple<int, char>>{{4, '4'}, {2, '2'}, {0, '0'}}) ));
}
