// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <sstream>

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

#define STREAM_EQ(x, ...)     ss << __VA_ARGS__; EXPECT_EQ(ss.str(), x); ss.str("")

TEST(StreamTest, STD) {
    std::stringstream ss;
    using namespace std;
    STREAM_EQ("[4, 2, 0]", array<int,3>{4,2,0});
    STREAM_EQ("(10; 4.2)", pair<int,double>{10,4.2});
    STREAM_EQ("(10; 'x'; 4.2)", tuple<int,char,double>{10,'x',4.2});
    STREAM_EQ("[4, 2, 0, 10]", vector<int>{4,2,0,10});
    STREAM_EQ("{0, 2, 4, 10}", set<int>{4,2,0,10});
    STREAM_EQ("{42}", unordered_set<int>{42});
    STREAM_EQ("{2:\"world\", 4:\"hello\"}", map<int, std::string>{{4,"hello"},{2,"world"}});
    STREAM_EQ("{42:\"hello world\"}", unordered_map<int, std::string>{{42, "hello world"}});
}

TEST(StreamTest, FCPP) {
    std::stringstream ss;
    using namespace fcpp;
    STREAM_EQ("{0:'y', 2:'z', *:'x'}", details::make_field({0,2}, std::vector<char>{'x','y','z'}));
    STREAM_EQ("[4, 2, 0]", vec<3>{4,2,0});
    STREAM_EQ("(false; 'a'; fcpp::common::type_sequence<void>)",
              tuple<bool,char,common::type_sequence<void>>{false,'a',{}});
    STREAM_EQ("2", internal::flat_ptr<int,true>{2});
    STREAM_EQ("true", internal::flat_ptr<bool,false>{true});
    common::multitype_map<trace_t,bool,char> m;
    m.insert(42, 'x');
    m.insert(10, false);
    STREAM_EQ("(bool => {10:false}; char => {42:'x'})", m);
    STREAM_EQ("{42:\"hello world\"}", common::random_access_map<int, std::string>{{42, "hello world"}});
    STREAM_EQ("(void => 3; int& => 'x')", common::make_tagged_tuple<void,int&>(3, 'x'));
    STREAM_EQ("(2)", internal::twin<int,true>{2});
    STREAM_EQ("(2; 2)", internal::twin<int,false>{2});
    {
        internal::context<true, true, int, bool, char> c;
        c.insert(42, m, 0, 10, 10);
        STREAM_EQ("(42:(bool => {10:false}; char => {42:'x'})@0)", c);
    }
    {
        internal::context<false, false, int, bool, char> c;
        c.insert(42, m, 0, 10, 10);
        STREAM_EQ("(42:(bool => {10:false}; char => {42:'x'})@0)", c);
    }
}
