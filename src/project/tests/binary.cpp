// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#define TEST(a, b) std::cout << #a << ": " << #b << std::endl;
#define TEST_F(a, b) std::cout << #a << ": " << #b << std::endl;
#define EXPECT_EQ(a, b) std::cout << (a == b ? "OK" : "NO") << ": " << a << " == " << b << std::endl
#define EXPECT_NE(a, b) std::cout << (a != b ? "OK" : "NO") << ": " << a << " != " << b << std::endl

#include <sstream>
#include <iostream>

#include "lib/common/ostream.hpp"
#include "lib/common/tagged_tuple.hpp"

using namespace std;
using namespace fcpp;

struct tag {};
struct gat {};
struct oth {};
struct hto {};


namespace tags {
    struct stuffer {};
    struct main {};
}


common::tagged_tuple_t<tag, int, gat, bool> t{2, true};



int main() {
TEST_F(TagTupleTest, Print) {
    std::stringstream s;
    s << t;
    EXPECT_EQ("(tag => 2; gat => true)", s.str());
    s.str("");
    t.print(s, common::assignment_tuple);
    EXPECT_EQ("tag = 2, gat = true", s.str());
    s.str("");
    t.print(s, common::underscore_tuple);
    EXPECT_EQ("tag-2_gat-true", s.str());
    s.str("");
    t.print(s, common::dictionary_tuple);
    EXPECT_EQ("tag:2, gat:true", s.str());
    s.str("");
    t.print(s, common::arrowhead_tuple);
    EXPECT_EQ("tag => 2; gat => true", s.str());
    common::tagged_tuple_t<oth,bool,tags::stuffer,char,void,double> t1{false,'z',4.5};
    s.str("");
    s << t1;
    EXPECT_EQ("(oth => false; stuffer => z; void => 4.5)", s.str());
    common::tagged_tuple_t<tags::main,std::string,tags::stuffer,const char*> t2{"tester","foo"};
    s.str("");
    t2.print(s, common::assignment_tuple);
    EXPECT_EQ("main = tester, stuffer = foo", s.str());
}
}
