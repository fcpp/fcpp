// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <sstream>

#include "gtest/gtest.h"

#include "lib/common/tagged_tuple.hpp"

using namespace fcpp;


struct tag {};
struct gat {};
struct oth {};
struct hto {};


namespace tags {
    struct stuffer {};
    struct main {};
    namespace nest {
        struct other {};
        template <typename... T>
        struct temp {};
    }
}


class TaggedTupleTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
    }

    common::tagged_tuple_t<tag, int, gat, bool> t{2, true};
};


TEST_F(TaggedTupleTest, Operators) {
    common::tagged_tuple_t<tag, int, gat, bool> x(t), y, z;
    z = y;
    y = x;
    z = std::move(y);
    EXPECT_EQ(t, z);
    y = common::tagged_tuple_t<tag, int, gat, bool>{1, false};
    EXPECT_NE(t, y);
    y = common::make_tagged_tuple<tag,gat>(2, true);
    EXPECT_EQ(t, y);
    common::tagged_tuple_t<> e(y);
    common::tagged_tuple_t<> f = common::make_tagged_tuple<>();
    EXPECT_EQ(e, f);
    common::tagged_tuple_t<tag, int const, gat, bool const> u{t};
    EXPECT_EQ(u, t);
}

TEST_F(TaggedTupleTest, Get) {
    int i;
    i = common::get<tag>(t);
    EXPECT_EQ(2, i);
    common::get<tag>(t) = 3;
    i = common::get<tag>(t);
    EXPECT_EQ(3, i);
    bool b;
    b = common::get<gat>(t);
    EXPECT_EQ(true, b);
    i = std::get<0>(t);
    EXPECT_EQ(3, i);
    b = common::get_or<gat>(t, false);
    EXPECT_EQ(true, b);
    b = common::get_or<oth>(t, false);
    EXPECT_EQ(false, b);
}

TEST_F(TaggedTupleTest, Call) {
    auto x = common::make_tagged_tuple<tag,gat>([](int x){ return x+1; }, [](int x){ return 2*x; });
    auto y = x(3);
    auto z = common::make_tagged_tuple<tag,gat>(4, 6);
    EXPECT_EQ(y, z);
}

TEST_F(TaggedTupleTest, Assignment) {
    int i;
    i = common::get<tag>(t);
    EXPECT_EQ(2, i);
    t = common::make_tagged_tuple<oth,tag>("stuff",5);
    i = common::get<tag>(t);
    EXPECT_EQ(5, i);
    bool b;
    b = common::get<gat>(t);
    EXPECT_EQ(true, b);
    t = common::make_tagged_tuple<gat,tag>(false,3);
    i = common::get<tag>(t);
    EXPECT_EQ(3, i);
    b = common::get<gat>(t);
    EXPECT_EQ(false, b);
    common::tagged_tuple_t<oth, double, tag, int> x(t);
    i = common::get<tag>(x);
    EXPECT_EQ(3, i);
}

TEST_F(TaggedTupleTest, Types) {
    std::string ex, res;
    ex  = typeid(char).name();
    res = typeid(common::tagged_tuple_t<tag,int,gat,char,oth,bool>::tag_type<gat>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(common::type_sequence<bool,char>).name();
    res = typeid(common::tagged_tuple_t<tag,int,gat,char,oth,bool>::tag_types<oth,gat>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(common::type_sequence<tag,gat,oth>).name();
    res = typeid(common::tagged_tuple_t<tag,int,gat,char,oth,bool>::tags).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(common::type_sequence<int,char,bool>).name();
    res = typeid(common::tagged_tuple_t<tag,int,gat,char,oth,bool>::types).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(common::tagged_tuple_t<tag,int,gat,char,oth,bool>).name();
    res = typeid(common::tagged_tuple_t<gat,char,oth,bool>::push_front<tag,int>).name();
    EXPECT_EQ(ex, res);
    res = typeid(common::tagged_tuple_t<tag,int,gat,char>::push_back<oth,bool>).name();
    EXPECT_EQ(ex, res);
    ex  = typeid(common::tagged_tuple_t<tag,int,gat,char,oth,bool,hto,double>).name();
    res = typeid(common::tagged_tuple_cat<
        common::tagged_tuple_t<tag,int>,
        common::tagged_tuple_t<gat,char,oth,bool>,
        common::tagged_tuple_t<hto,double>
    >).name();
    EXPECT_EQ(ex, res);
}

TEST_F(TaggedTupleTest, Print) {
    std::stringstream s;
    t.print(s);
    EXPECT_EQ("tag => 2; gat => true", s.str());
    s.str("");
    t.print(s, common::arrowhead_tuple);
    EXPECT_EQ("tag => 2; gat => true", s.str());
    s.str("");
    t.print(s, common::assignment_tuple);
    EXPECT_EQ("tag = 2, gat = true", s.str());
    s.str("");
    t.print(s, common::underscore_tuple);
    EXPECT_EQ("tag-2_gat-true", s.str());
    s.str("");
    t.print(s, common::dictionary_tuple);
    EXPECT_EQ("tag:2, gat:true", s.str());
    common::tagged_tuple_t<oth,bool,tags::stuffer,char,void,double> t1{false,'z',4.5};
    s.str("");
    t1.print(s, common::assignment_tuple);
    EXPECT_EQ("oth = false, stuffer = 'z', void = 4.5", s.str());
    common::tagged_tuple_t<tags::main,std::string,tags::stuffer,char const*> t2{"tester","foo"};
    s.str("");
    t2.print(s, common::assignment_tuple);
    EXPECT_EQ("main = \"tester\", stuffer = \"foo\"", s.str());
    s.str("");
    t2.print(s, common::underscore_tuple, common::skip_tags<tags::main>);
    EXPECT_EQ("stuffer-foo", s.str());
    using nasty_type = tags::nest::temp<tags::nest::temp<tags::stuffer>,tags::nest::other>;
    common::tagged_tuple_t<tags::main,int,double,bool,nasty_type,char> t3{42,false,'w'};
    s.str("");
    t3.print(s, common::assignment_tuple);
    EXPECT_EQ("main = 42, double = false, temp<temp<stuffer>,other> = 'w'", s.str());
    s.str("");
    t3.print(s, common::assignment_tuple, common::skip_tags<double,tags::main,nasty_type>);
    EXPECT_EQ("", s.str());
}
