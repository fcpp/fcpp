// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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
    std::string ex1 = "std::array<int, 10ul>";
    std::string ex2 = "std::__1::array<int, 10ul>";
    res = common::type_name<std::array<int,10>>();
    EXPECT_TRUE(res == ex1 or res == ex2);
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

TEST(TraitsTest, NumberSequence) {
    int i;
    i = common::number_sequence<1,2,4,4,0>::front;
    EXPECT_EQ(1, i);
    i = common::number_sequence<0,2,4,4,1>::back;
    EXPECT_EQ(1, i);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2,4,4,0>::push_back<>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2,4,4>::push_back<0>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2,4>::push_back<4,0>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2>::push_back<4,4,0>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<>::push_back<2,4,4,0>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<4,0>::push_front<2,4>);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<1,2,4,4,0>::pop_front);
    EXPECT_SAME(common::number_sequence<2,4,4,0>,
                common::number_sequence<2,4,4,0,1>::pop_back);
    i = common::number_sequence<2,4,4,0>::size;
    EXPECT_EQ(4, i);
    i = common::number_sequence<>::size;
    EXPECT_EQ(0, i);
    EXPECT_SAME(common::number_sequence<4,0>,
                common::number_sequence<2,4,4,0>::slice<1,-1,2>);
    EXPECT_SAME(common::number_sequence<2,0>,
                common::number_sequence<2,4,4,0>::slice<0,-1,3>);
    EXPECT_SAME(common::number_sequence<4>,
                common::number_sequence<2,4,4,0>::slice<2,-1,5>);
    EXPECT_SAME(common::number_sequence<4,4>,
                common::number_sequence<2,4,4,0>::slice<1,3,1>);
    i = common::number_sequence<2,2,4,0>::get<2>;
    EXPECT_EQ(4, i);
    i = common::number_sequence<2,4,0>::find<2>;
    EXPECT_EQ(0, i);
    i = common::number_sequence<2,0,4>::find<4>;
    EXPECT_EQ(2, i);
    i = common::number_sequence<2,0,4>::count<4>;
    EXPECT_EQ(1, i);
    i = common::number_sequence<4,0>::count<2>;
    EXPECT_EQ(0, i);
    i = common::number_sequence<2,0,2>::count<2>;
    EXPECT_EQ(2, i);
}

TEST(TraitsTest, ExportList) {
    EXPECT_SAME(common::export_list<>,
                common::type_sequence<>);
    EXPECT_SAME(common::export_list<int, double, double, bool>,
                common::type_sequence<int, double, bool>);
    EXPECT_SAME(common::export_list<int, double, common::type_sequence<void, int, char>, double, bool>,
                common::type_sequence<int, double, void, char, bool>);
    EXPECT_SAME(common::export_list<int, double, common::type_sequence<void, int, char, common::type_sequence<bool, float, bool>>, double, common::type_sequence<>, bool>,
                common::type_sequence<int, double, void, char, bool, float>);
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
    b = common::has_template<proxy, proxy<double> const>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, proxy<double>&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, proxy<double>&&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, proxy<double> const&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, proxy<double> const&&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, int>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, int const>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, int&>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, int const&>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, int&&>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, int const&&>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::array<proxy<double>,4>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::array<proxy<double>,4> const>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::array<int,4>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::array<int,4> const>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::pair<proxy<double>,int>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::pair<proxy<double>,int> const>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::pair<int,double>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::pair<int,double> const>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::tuple<proxy<double>,char>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::tuple<proxy<double>,char> const>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::tuple<int,char>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::tuple<int,char> const>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4> const&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::array<std::tuple<std::array<proxy<double>,3>,char>,4>&&>;
    EXPECT_TRUE(b);
    b = common::has_template<proxy, std::array<std::tuple<std::array<double,3>,char>,4>>;
    EXPECT_FALSE(b);
    b = common::has_template<proxy, std::array<std::tuple<std::array<double,3>,char>,4> const&>;
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
    EXPECT_SAME(double,         common::extract_template<proxy, double const>);
    EXPECT_SAME(double const&,  common::extract_template<proxy, double&>);
    EXPECT_SAME(double,         common::extract_template<proxy, double&&>);
    EXPECT_SAME(double,         common::extract_template<proxy, proxy<double>>);
    EXPECT_SAME(double,         common::extract_template<proxy, proxy<double const>>);
    EXPECT_SAME(double&,        common::extract_template<proxy, proxy<double&>>);
    EXPECT_SAME(double&,        common::extract_template<proxy, proxy<double&>&>);
    EXPECT_SAME(double&,        common::extract_template<proxy, proxy<double&> const>);
    EXPECT_SAME(double const&,  common::extract_template<proxy, proxy<double const>&>);
    EXPECT_SAME(double,         common::extract_template<proxy, proxy<double>&&>);
    EXPECT_SAME(double,         common::extract_template<proxy, proxy<double&&>>);
    EXPECT_SAME(std::pair<double,int>,
                common::extract_template<proxy, std::pair<proxy<double const>,int>&&>);
    EXPECT_SAME(std::pair<double,int>&,
                common::extract_template<proxy, proxy<std::pair<double,int>&> const&>);
    EXPECT_SAME(std::pair<double,int> const&,
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
                common::extract_template<proxy, array<proxy<double>,4> const&&>);
    EXPECT_SAME(std::tuple<double,int>,
                common::extract_template<proxy, std::tuple<double,int>>);
    EXPECT_SAME(std::tuple<double,char>,
                common::extract_template<proxy, std::tuple<proxy<double>,char>>);
    EXPECT_SAME(std::tuple<double&,char const&>,
                common::extract_template<proxy, std::tuple<proxy<double>,char>&>);
    EXPECT_SAME(std::tuple<double const&,char const&>,
                common::extract_template<proxy, std::tuple<proxy<double>,char> const&>);
    EXPECT_SAME(std::tuple<double,char>,
                common::extract_template<proxy, proxy<std::tuple<double,char>>&&>);
    EXPECT_SAME(array<std::tuple<array<double,3>,char>,4> const&,
                common::extract_template<proxy, array<std::tuple<array<double,3>,char>,4>&>);
    EXPECT_SAME(array<std::tuple<array<double const&,3>,char const&>,4>,
                common::extract_template<proxy, array<std::tuple<array<proxy<double>,3>,char>,4> const&>);
    EXPECT_SAME(array<std::tuple<array<double&,3>,char const&>,4>,
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
    EXPECT_SAME(common::type_sequence<int const&, proxy<double> const&, char const&>,
                common::template_args<std::tuple<int, proxy<double>, char> const&>);
    EXPECT_SAME(common::type_sequence<proxy<double>>,
                common::template_args<array<proxy<double>, 4>>);
    EXPECT_SAME(common::type_sequence<proxy<double>&&>,
                common::template_args<array<proxy<double>, 4>&&>);
    EXPECT_SAME(common::type_sequence<proxy<double> const&>,
                common::template_args<array<proxy<double>, 4> const&>);
}

int adder(int x, int y) {
    return x+y;
}

template <typename F, typename = common::if_signature<F, int(int,int)>>
int comb(F&& f, int x) {
    return f(x,x);
}

int comb(int x, int y) {
    return x+y;
}

template <typename F, typename = common::type_unwrap<void(common::type_sequence<common::if_signature<F, int(int,int)>>)>>
int comb(F&& f, int x, int y) {
    return f(x,y);
}

int comb(int x, int y, int z) {
    return x+y+z;
}

TEST(TraitsTest, Signature) {
    EXPECT_EQ(3, comb(1,2));
    EXPECT_EQ(4, comb(adder, 2));
    EXPECT_EQ(5, comb(adder, 2, 3));
    EXPECT_EQ(6, comb(1,2,3));
}

template <bool b>
struct flagopt {};

template <intmax_t i>
struct onenum {};

template <intmax_t i, intmax_t j=1>
struct twonum {};

template <intmax_t... is>
struct numopt {};

template <typename T>
struct onetype {};

template <typename... Ts>
struct typeopt {};

TEST(TraitsTest, Options) {
    using plainseq = common::type_sequence<onenum<10>, twonum<2>, flagopt<false>, numopt<2,3>, onetype<int>, typeopt<int,char>>;
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
    double f;
    f = common::option_float<twonum, 45, 10, int, bool>;
    EXPECT_DOUBLE_EQ(f, 4.5);
    f = common::option_float<twonum, 45, 10, int, bool, twonum<2>, void, twonum<6>>;
    EXPECT_DOUBLE_EQ(f, 2.0);
    f = common::option_float<twonum, 45, 10, int, common::type_sequence<bool, twonum<2>, void>, twonum<6>>;
    EXPECT_DOUBLE_EQ(f, 2.0);
    f = common::option_float<twonum, 45, 10, int, hideseq, twonum<6>>;
    EXPECT_DOUBLE_EQ(f, 2.0);
    EXPECT_SAME(common::option_nums<numopt,void>,
                common::number_sequence<>);
    EXPECT_SAME(common::option_nums<numopt,void,numopt<2,3>,bool>,
                common::number_sequence<2,3>);
    EXPECT_SAME(common::option_nums<numopt,void,numopt<2,3>,bool,numopt<>,numopt<4>>,
                common::number_sequence<2,3,4>);
    EXPECT_SAME(common::option_nums<numopt,void,common::type_sequence<numopt<2,3>,bool,numopt<>>,numopt<4>>,
                common::number_sequence<2,3,4>);
    EXPECT_SAME(common::option_nums<numopt,void,hideseq,numopt<>,numopt<4>>,
                common::number_sequence<2,3,4>);
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

struct custom_stream_type {};
template <>
struct common::is_ostream<custom_stream_type> : public std::true_type {};

TEST(TraitsTest, IsOStream) {
    bool b;
    b = common::is_ostream<std::ostream>::value;
    EXPECT_TRUE(b);
    b = common::is_ostream<int>::value;
    EXPECT_FALSE(b);
    b = common::is_ostream<custom_stream_type>::value;
    EXPECT_TRUE(b);
}
