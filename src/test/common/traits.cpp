// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <array>
#include <string>
#include <tuple>
#include <typeinfo>

#include "test/helper.hpp"

#include "lib/common/traits.hpp"

using namespace fcpp;

template<class T> struct proxy {};
template <class T, size_t N> struct array {};

TEST(TraitsTest, TypeName) {
    std::string ex, res;
    ex  = "double";
    res = common::type_name<double>();
    EXPECT_EQ(ex, res);
    ex  = "std::array<int, 10ul>";
    res = common::type_name<std::array<int,10>>();
    EXPECT_EQ(ex, res);
}

TEST(TraitsTest, QueueOp) {
    EXPECT_SAME(short, common::type_sequence<short,int,double,double,char>::front);
    EXPECT_SAME(short, common::type_sequence<char,int,double,double,short>::back);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int,double,double,char>::push_back<>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int,double,double>::push_back<char>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int,double>::push_back<double,char>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int>::push_back<double,double,char>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<>::push_back<int,double,double,char>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<double,char>::push_front<int,double>);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<short,int,double,double,char>::pop_front);
    EXPECT_SAME(common::type_sequence<int,double,double,char>,
                common::type_sequence<int,double,double,char,short>::pop_back);
}

TEST(TraitsTest, ArrayOp) {
    int i;
    i = common::type_sequence<int,double,double,char>::size;
    EXPECT_EQ(4, i);
    i = common::type_sequence<>::size;
    EXPECT_EQ(0, i);
    EXPECT_SAME(common::type_sequence<double,char>,
                common::type_sequence<int,double,double,char>::slice<1,-1,2>);
    EXPECT_SAME(common::type_sequence<int,char>,
                common::type_sequence<int,double,double,char>::slice<0,-1,3>);
    EXPECT_SAME(common::type_sequence<double>,
                common::type_sequence<int,double,double,char>::slice<2,-1,5>);
    EXPECT_SAME(common::type_sequence<double,double>,
                common::type_sequence<int,double,double,char>::slice<1,3,1>);
    EXPECT_SAME(double, common::type_sequence<int,int,double,char>::get<2>);
}

TEST(TraitsTest, SetOp) {
    EXPECT_SAME(common::type_sequence<int,char>,
                common::type_sequence<int,double,double,char>::intersect<short,char,int>);
    EXPECT_SAME(common::type_sequence<double,double>,
                common::type_sequence<int,double,double,char>::intersect<double>);
    EXPECT_SAME(common::type_sequence<double,double,char,float,int>,
                common::type_sequence<double,double,char>::unite<double,float,int,float>);
    EXPECT_SAME(common::type_sequence<void,char>,
                common::type_sequence<void,double,double,char>::subtract<double,float,int,float>);
    EXPECT_SAME(common::type_sequence<double>,
                common::type_sequence<int,double,double,char>::repeated);
    EXPECT_SAME(common::type_sequence<int,double,char>,
                common::type_sequence<int,double,double,char>::uniq);
    EXPECT_SAME(common::type_cat<common::type_sequence<int,double,char>, common::type_sequence<float>, common::type_sequence<bool,void>>,
                common::type_sequence<int,double,char,float,bool,void>::uniq);
    EXPECT_SAME(common::type_product< common::type_sequence<common::type_sequence<double,float>, common::type_sequence<char>>, common::type_sequence<common::type_sequence<>, common::type_sequence<bool>>, common::type_sequence<common::type_sequence<void>> >,
                common::type_sequence<common::type_sequence<double,float,void>, common::type_sequence<char,void>, common::type_sequence<double,float,bool,void>, common::type_sequence<char,bool,void>>);
    int i;
    i = common::type_sequence<int,double,double,char>::repeated::size;
    EXPECT_EQ(1, i);
    i = common::type_sequence<int,double,char>::repeated::size;
    EXPECT_EQ(0, i);
}

TEST(TraitsTest, SearchOp) {
    int t;
    t = common::type_sequence<int,double,char>::find<int>;
    EXPECT_EQ(0, t);
    t = common::type_sequence<int,char,double>::find<double>;
    EXPECT_EQ(2, t);
    t = common::type_sequence<int,char,double>::count<double>;
    EXPECT_EQ(1, t);
    t = common::type_sequence<double,char>::count<int>;
    EXPECT_EQ(0, t);
    t = common::type_sequence<int,char,int>::count<int>;
    EXPECT_EQ(2, t);
}

TEST(TraitsTest, BoolPack) {
    bool val;
    val = common::all_true<>;
    EXPECT_TRUE(val);
    val = common::all_true<true, true, true>;
    EXPECT_TRUE(val);
    val = common::all_true<true, false>;
    EXPECT_FALSE(val);
    val = common::all_false<>;
    EXPECT_TRUE(val);
    val = common::all_false<false, false, false>;
    EXPECT_TRUE(val);
    val = common::all_false<true, false>;
    EXPECT_FALSE(val);
    val = common::some_true<>;
    EXPECT_FALSE(val);
    val = common::some_true<false, false, false>;
    EXPECT_FALSE(val);
    val = common::some_true<true, false>;
    EXPECT_TRUE(val);
    val = common::some_false<>;
    EXPECT_FALSE(val);
    val = common::some_false<true, true, true>;
    EXPECT_FALSE(val);
    val = common::some_false<true, false>;
    EXPECT_TRUE(val);
}

TEST(TraitsTest, IsTemplate) {
    bool b;
    b = common::is_sized_template<array, array<proxy<int>, 4>>;
    EXPECT_TRUE(b);
    b = common::is_sized_template<array, int>;
    EXPECT_FALSE(b);
    b = common::is_sized_template<array, proxy<array<int, 4>>>;
    EXPECT_FALSE(b);
    b = common::is_class_template<proxy, array<proxy<int>, 4>>;
    EXPECT_FALSE(b);
    b = common::is_class_template<proxy, int>;
    EXPECT_FALSE(b);
    b = common::is_class_template<proxy, proxy<array<int, 4>>>;
    EXPECT_TRUE(b);
}

TEST(TraitsTest, HasTemplate) {
    bool b;
    b = common::has_template<proxy, proxy<double>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, const proxy<double>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, proxy<double>&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, proxy<double>&&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, const proxy<double>&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, const proxy<double>&&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, int>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, const int>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, int&>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, const int&>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, int&&>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, const int&&>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::array<proxy<double>,4>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, const std::array<proxy<double>,4>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::array<int,4>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, const std::array<int,4>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::pair<proxy<double>,int>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, const std::pair<proxy<double>,int>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::pair<int,double>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, const std::pair<int,double>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::tuple<proxy<double>,char>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, const std::tuple<proxy<double>,char>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::tuple<int,char>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, const std::tuple<int,char>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, const std::array<std::tuple<std::array<proxy<double>,3>,char>,4>&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>&&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::array<std::tuple<std::array<double,3>,char>,4>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, const std::array<std::tuple<std::array<double,3>,char>,4>&>;
    EXPECT_FALSE(b);
}

TEST(TraitsTest, ExtractTemplate) {
    EXPECT_SAME(char,           common::partial_decay<char>);
    EXPECT_SAME(char&,          common::partial_decay<char&>);
    EXPECT_SAME(char,           common::partial_decay<char&&>);
    EXPECT_SAME(char,           common::partial_decay<char const>);
    EXPECT_SAME(char const&,    common::partial_decay<char const&>);
    EXPECT_SAME(char,           common::partial_decay<char const&&>);
    EXPECT_SAME(double,         common::extract_template<proxy, double>);
    EXPECT_SAME(double,         common::extract_template<proxy, const double>);
    EXPECT_SAME(const double&,  common::extract_template<proxy, double&>);
    EXPECT_SAME(double,         common::extract_template<proxy, double&&>);
    EXPECT_SAME(double,         common::extract_template<proxy, proxy<double>>);
    EXPECT_SAME(double,         common::extract_template<proxy, proxy<const double>>);
    EXPECT_SAME(double&,        common::extract_template<proxy, proxy<double&>>);
    EXPECT_SAME(double&,        common::extract_template<proxy, proxy<double&>&>);
    EXPECT_SAME(double&,        common::extract_template<proxy, const proxy<double&>>);
    EXPECT_SAME(const double&,  common::extract_template<proxy, proxy<const double>&>);
    EXPECT_SAME(double,         common::extract_template<proxy, proxy<double>&&>);
    EXPECT_SAME(double,         common::extract_template<proxy, proxy<double&&>>);
    EXPECT_SAME(std::pair<double,int>,
                common::extract_template<proxy, std::pair<proxy<const double>,int>&&>);
    EXPECT_SAME(std::pair<double,int>&,
                common::extract_template<proxy, const proxy<std::pair<double,int>&>&>);
    EXPECT_SAME(const std::pair<double,int>&,
                common::extract_template<proxy, std::pair<double,int>&>);
    EXPECT_SAME(array<double,4>,
                common::extract_template<proxy, array<proxy<double>,4>>);
    EXPECT_SAME(array<double,4>,
                common::extract_template<proxy, proxy<array<double,4>>>);
    EXPECT_SAME(array<double&,4>,
                common::extract_template<proxy, array<proxy<double&>,4>>);
    EXPECT_SAME(array<double&,4>,
                common::extract_template<proxy, array<proxy<double>&,4>>);
    EXPECT_SAME(array<double&,4>,
                common::extract_template<proxy, array<proxy<double>,4>&>);
    EXPECT_SAME(array<double,4>,
                common::extract_template<proxy, const array<proxy<double>,4>&&>);
    EXPECT_SAME(std::tuple<double,int>,
                common::extract_template<proxy, std::tuple<double,int>>);
    EXPECT_SAME(std::tuple<double,char>,
                common::extract_template<proxy, std::tuple<proxy<double>,char>>);
    EXPECT_SAME(std::tuple<double&,const char&>,
                common::extract_template<proxy, std::tuple<proxy<double>,char>&>);
    EXPECT_SAME(std::tuple<double,char>,
                common::extract_template<proxy, proxy<std::tuple<double,char>>&&>);
    EXPECT_SAME(const array<std::tuple<array<double,3>,char>,4>&,
                common::extract_template<proxy, array<std::tuple<array<double,3>,char>,4>&>);
    EXPECT_SAME(array<std::tuple<array<const double&,3>,const char&>,4>,
                common::extract_template<proxy, const array<std::tuple<array<proxy<double>,3>,char>,4>&>);
    EXPECT_SAME(array<std::tuple<array<double&,3>,const char&>,4>,
                common::extract_template<proxy, array<std::tuple<array<proxy<double>,3>,char>,4>&>);
    EXPECT_SAME(array<std::tuple<array<double,3>,char>,4>,
                common::extract_template<proxy, array<std::tuple<proxy<array<double,3>>,char>,4>&&>);
    EXPECT_SAME(array<std::tuple<array<double,3>,char>&,4>,
                common::extract_template<proxy, array<proxy<std::tuple<array<double,3>,char>>,4>&>);
    EXPECT_SAME(array<std::tuple<array<double,3>,char>,4>,
                common::extract_template<proxy, proxy<array<std::tuple<array<double,3>,char>,4>>&&>);
}

TEST(TraitsTest, TemplateArgs) {
    EXPECT_SAME(common::type_sequence<int, proxy<double>, char>,
                common::template_args<std::tuple<int, proxy<double>, char>>);
    EXPECT_SAME(common::type_sequence<int&&, proxy<double>&&, char&&>,
                common::template_args<std::tuple<int, proxy<double>, char>&&>);
    EXPECT_SAME(common::type_sequence<const int&, const proxy<double>&, const char&>,
                common::template_args<const std::tuple<int, proxy<double>, char>&>);
    EXPECT_SAME(common::type_sequence<proxy<double>>,
                common::template_args<array<proxy<double>, 4>>);
    EXPECT_SAME(common::type_sequence<proxy<double>&&>,
                common::template_args<array<proxy<double>, 4>&&>);
    EXPECT_SAME(common::type_sequence<const proxy<double>&>,
                common::template_args<const array<proxy<double>, 4>&>);
}

template <bool b>
struct flagopt {};

template <size_t i>
struct onenum {};

template <size_t... is>
struct numopt {};

template <typename T>
struct onetype {};

template <typename... Ts>
struct typeopt {};

TEST(TraitsTest, Options) {
    using plainseq = common::type_sequence<onenum<10>, flagopt<false>, numopt<2,3>, onetype<int>, typeopt<int,char>>;
    struct hideseq : public plainseq {};
    EXPECT_SAME(common::details::type_sequence_decay<hideseq>, plainseq);
    EXPECT_SAME(common::details::type_sequence_decay<std::tuple<int,char>>, common::type_sequence<>);
    bool b;
    b = common::option_flag<flagopt, false, int, void, char>;
    EXPECT_FALSE(b);
    b = common::option_flag<flagopt, true, int, flagopt<false>, char, bool, flagopt<true>>;
    EXPECT_FALSE(b);
    b = common::option_flag<flagopt, true, common::type_sequence<int, flagopt<false>, char>, bool, flagopt<true>>;
    EXPECT_FALSE(b);
    b = common::option_flag<flagopt, true, hideseq, bool, flagopt<true>>;
    EXPECT_FALSE(b);
    b = common::option_flag<flagopt, true, int, void, char>;
    EXPECT_TRUE(b);
    b = common::option_flag<flagopt, false, int, flagopt<true>, char, bool, flagopt<true>>;
    EXPECT_TRUE(b);
    size_t n;
    n = common::option_num<onenum, 42, int, bool>;
    EXPECT_EQ(n, 42ULL);
    n = common::option_num<onenum, 42, int, bool, onenum<10>, void, onenum<6>>;
    EXPECT_EQ(n, 10ULL);
    n = common::option_num<onenum, 42, int, common::type_sequence<bool, onenum<10>, void>, onenum<6>>;
    EXPECT_EQ(n, 10ULL);
    n = common::option_num<onenum, 42, int, hideseq, onenum<6>>;
    EXPECT_EQ(n, 10ULL);
    EXPECT_SAME(common::option_nums<numopt,void>,
                common::index_sequence<>);
    EXPECT_SAME(common::option_nums<numopt,void,numopt<2,3>,bool>,
                common::index_sequence<2,3>);
    EXPECT_SAME(common::option_nums<numopt,void,numopt<2,3>,bool,numopt<>,numopt<4>>,
                common::index_sequence<2,3,4>);
    EXPECT_SAME(common::option_nums<numopt,void,common::type_sequence<numopt<2,3>,bool,numopt<>>,numopt<4>>,
                common::index_sequence<2,3,4>);
    EXPECT_SAME(common::option_nums<numopt,void,hideseq,numopt<>,numopt<4>>,
                common::index_sequence<2,3,4>);
    EXPECT_SAME(common::option_type<onetype,std::string,void,char>,
                std::string);
    EXPECT_SAME(common::option_type<onetype,std::string,void,char,onetype<int>,bool,onetype<void>>,
                int);
    EXPECT_SAME(common::option_type<onetype,std::string,void,common::type_sequence<char,onetype<int>>,onetype<void>>,
                int);
    EXPECT_SAME(common::option_type<onetype,std::string,void,hideseq,onetype<void>>,
                int);
    EXPECT_SAME(common::option_types<typeopt,void>,
                common::type_sequence<>);
    EXPECT_SAME(common::option_types<typeopt,void,typeopt<int,char>,bool>,
                common::type_sequence<int,char>);
    EXPECT_SAME(common::option_types<typeopt,void,typeopt<int,char>,bool,typeopt<>,typeopt<long>>,
                common::type_sequence<int,char,long>);
    EXPECT_SAME(common::option_types<typeopt,void,common::type_sequence<typeopt<int,char>,bool>,typeopt<long>>,
                common::type_sequence<int,char,long>);
    EXPECT_SAME(common::option_types<typeopt,void,hideseq,typeopt<long>>,
                common::type_sequence<int,char,long>);
    EXPECT_SAME(common::option_multitypes<typeopt,void,typeopt<int,char>,bool,typeopt<>,typeopt<long>>,
                common::type_sequence<
                    common::type_sequence<int,char>,
                    common::type_sequence<>,
                    common::type_sequence<long>
                >);
    EXPECT_SAME(common::option_multitypes<typeopt,void,hideseq,typeopt<>,typeopt<long>>,
                common::type_sequence<
                    common::type_sequence<int,char>,
                    common::type_sequence<>,
                    common::type_sequence<long>
                >);
    EXPECT_SAME(common::apply_templates<common::type_sequence<bool, char, double>, std::tuple>,
                std::tuple<bool,char,double>);
    EXPECT_SAME(common::apply_templates<common::type_sequence<
                    bool, common::type_sequence<char, int>, double
                >, std::tuple, typeopt>,
                std::tuple<bool,typeopt<char,int>,double>);
}
