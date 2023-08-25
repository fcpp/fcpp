// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/basics.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


struct tag {};

using tuple_t = common::tagged_tuple_t<tag, int>;

template <int O>
DECLARE_OPTIONS(options,
    exports<common::export_list<coordination::spawn_t<tuple_t, bool>, coordination::spawn_t<int, status>, coordination::spawn_t<int, field<bool>>, field<int>, times_t, int>>,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

DECLARE_COMBINE(calc_only, component::calculus);

template <int O>
using combo = calc_only<options<O>>;


template <typename T>
void sendto(T const& source, T& dest) {
    typename T::message_t m;
    dest.receive(0, source.uid, source.send(0, m));
}

template <typename T>
void sendall(T& x, T& y, T& z) {
    x.round_end(0);
    y.round_end(0);
    z.round_end(0);
    sendto(x, x);
    sendto(x, y);
    sendto(x, z);
    sendto(y, x);
    sendto(y, y);
    sendto(y, z);
    sendto(z, x);
    sendto(z, y);
    sendto(z, z);
    x.round_start(0);
    y.round_start(0);
    z.round_start(0);
}

TEST(BasicsTest, ResultType) {
    auto rf = [](field<int>){
        return 2.5;
    };
    auto tf = [](field<int>){
        return tuple<std::string, double>("foo", 2.5);
    };
    EXPECT_SAME(coordination::return_result_type<int, decltype(rf)(int)>, double);
    EXPECT_SAME(coordination::export_result_type<int, decltype(rf)(int)>, double);
    EXPECT_SAME(coordination::return_result_type<int, decltype(tf)(int)>, std::string);
    EXPECT_SAME(coordination::export_result_type<int, decltype(tf)(int)>, double);
}

template <typename node_t>
times_t delayed(node_t& node, trace_t call_point, times_t t) {
    return coordination::old(node, call_point, t);
}

template <typename node_t>
times_t delayed(node_t& node, trace_t call_point, times_t t, int start) {
    return coordination::old(node, call_point, start, t);
}

template <typename node_t>
int counter(node_t& node, trace_t call_point) {
    return coordination::old(node, call_point, 0, [](int const& o) {
        return o+1;
    });
}

template <typename node_t>
int counter2(node_t& node, trace_t call_point) {
    return coordination::old(node, call_point, 1.0, [](int const& o) {
        return make_tuple(o, o+1);
    });
}

MULTI_TEST(BasicsTest, Old, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid>(0)};
    times_t d;
    d0.round_start(0);
    d = delayed(d0, 0, 1);
    EXPECT_EQ(1, d);
    d0.round_end(0);
    sendto(d0, d0);
    d0.round_start(0);
    d = delayed(d0, 0, 3);
    EXPECT_EQ(1, d);
    d0.round_end(0);
    d0.round_start(0);
    d = delayed(d0, 0, 5);
    EXPECT_EQ(1, d);
    d0.round_end(0);
    sendto(d0, d0);
    d0.round_start(0);
    d = delayed(d0, 0, 3);
    EXPECT_EQ(5, d);
    d = delayed(d0, 1, 3, 2);
    EXPECT_EQ(2, d);
    d0.round_end(0);
    sendto(d0, d0);
    d0.round_start(0);
    d = delayed(d0, 1, 6, 2);
    EXPECT_EQ(3, d);
    d = counter(d0, 2);
    EXPECT_EQ(1, d);
    d0.round_end(0);
    sendto(d0, d0);
    d0.round_start(0);
    d = counter(d0, 2);
    EXPECT_EQ(2, d);
    d0.round_end(0);
    sendto(d0, d0);
    d0.round_start(0);
    d = counter(d0, 2);
    EXPECT_EQ(3, d);
    d = counter2(d0, 3);
    EXPECT_EQ(1, d);
    d0.round_end(0);
    sendto(d0, d0);
    d0.round_start(0);
    d = counter2(d0, 3);
    EXPECT_EQ(2, d);
    d0.round_end(0);
    sendto(d0, d0);
    d0.round_start(0);
    d = counter2(d0, 3);
    EXPECT_EQ(3, d);
}

template <typename node_t>
int sharing(node_t& node, trace_t call_point, int x) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return coordination::fold_hood(node, 0, [](int x, int y) {
        return x+y;
    }, coordination::nbr(node, 1.0, x));
}

template <typename node_t>
int gossip(node_t& node, trace_t call_point, int x) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return coordination::nbr(node, 0, x, [&](field<int> n) {
        return std::max(coordination::fold_hood(node, 1, [](int x, int y) {
            return std::max(x,y);
        }, n), x);
    });
}

template <typename node_t>
int gossip2(node_t& node, trace_t call_point, int x) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return coordination::nbr(node, 0, double(x), [&](field<int> n) {
        int r = coordination::fold_hood(node, 1, [](int x, int y) {
            return std::max(x,y);
        }, n);
        return make_tuple(r, std::max(r, x));
    });
}

MULTI_TEST(BasicsTest, Nbr, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid>(0)};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid>(1)};
    typename combo<O>::node d2{network, common::make_tagged_tuple<uid>(2)};
    int d;
    d = sharing(d0, 0, 4);
    EXPECT_EQ(4, d);
    d = sharing(d1, 0, 2);
    EXPECT_EQ(2, d);
    d = sharing(d2, 0, 1);
    EXPECT_EQ(1, d);
    d0.round_end(0);
    d1.round_end(0);
    d2.round_end(0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    d0.round_start(0);
    d = sharing(d0, 0, 3);
    EXPECT_EQ(7, d);
    d = gossip(d0, 1, 3);
    EXPECT_EQ(3, d);
    d = gossip(d1, 1, 2);
    EXPECT_EQ(2, d);
    d = gossip(d2, 1, 4);
    EXPECT_EQ(4, d);
    d0.round_end(0);
    d1.round_end(0);
    d2.round_end(0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    sendto(d1, d1);
    sendto(d2, d2);
    d0.round_start(0);
    d1.round_start(0);
    d2.round_start(0);
    d = gossip(d0, 1, 1);
    EXPECT_EQ(4, d);
    d = gossip(d1, 1, 10);
    EXPECT_EQ(10, d);
    d = gossip(d2, 1, 1);
    EXPECT_EQ(4, d);
    d = gossip2(d0, 2, 3);
    EXPECT_EQ(3, d);
    d = gossip2(d1, 2, 2);
    EXPECT_EQ(2, d);
    d = gossip2(d2, 2, 4);
    EXPECT_EQ(4, d);
    d0.round_end(0);
    d1.round_end(0);
    d2.round_end(0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    sendto(d1, d1);
    sendto(d2, d2);
    d0.round_start(0);
    d1.round_start(0);
    d2.round_start(0);
    d = gossip2(d0, 2, 1);
    EXPECT_EQ(4, d);
    d = gossip2(d1, 2, 10);
    EXPECT_EQ(2, d);
    d = gossip2(d2, 2, 1);
    EXPECT_EQ(4, d);
}

template <typename node_t>
int weirdfeedback(node_t& node, trace_t call_point, double r) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return coordination::oldnbr(node, 0, r, [&](field<int> o, field<int> n) {
        field<int> x = (o + n) / 2;
        return make_tuple(coordination::fold_hood(node, 1, [](int x, int y) {
            return x + y;
        }, x), x);
    });
}

MULTI_TEST(BasicsTest, OldNbr, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid>(0)};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid>(1)};
    typename combo<O>::node d2{network, common::make_tagged_tuple<uid>(2)};
    int d;
    d0.round_start(0);
    d1.round_start(0);
    d2.round_start(0);
    d = weirdfeedback(d0, 0, 0);
    EXPECT_EQ(0, d);
    d = weirdfeedback(d1, 0, 10);
    EXPECT_EQ(10, d);
    d = weirdfeedback(d2, 0, 20);
    EXPECT_EQ(20, d);
    sendall(d0, d1, d2);
    d = weirdfeedback(d0, 0, -1000);
    EXPECT_EQ(15, d);
    d = weirdfeedback(d1, 0, -1000);
    EXPECT_EQ(30, d);
    d = weirdfeedback(d2, 0, -1000);
    EXPECT_EQ(45, d);
    sendall(d0, d1, d2);
    d = weirdfeedback(d0, 0, -1000);
    EXPECT_EQ(15, d);
    d = weirdfeedback(d1, 0, -1000);
    EXPECT_EQ(30, d);
    d = weirdfeedback(d2, 0, -1000);
    EXPECT_EQ(45, d);
}

template <status x>
struct converter {
    static constexpr int value() {
        return (int)x;
    }
};

TEST(BasicsTest, Status) {
    EXPECT_EQ(status::border_output, status::border and status::output);
    EXPECT_EQ(status::border_output, status::output and status::border);
    EXPECT_EQ(status::border_output, status::border_output and status::output);
    EXPECT_EQ(status::border_output, status::output and status::border_output);
    EXPECT_EQ(converter<status::border and status::output>::value(), 6);
    EXPECT_EQ(status::border, status::border xor status::output);
    EXPECT_EQ(status::border, status::output xor status::border);
    EXPECT_EQ(status::border, status::border_output xor status::output);
    EXPECT_EQ(status::border, status::output xor status::border_output);
    EXPECT_EQ(converter<status::border_output xor status::output>::value(), 2);
    std::stringstream ss;
    ss << status::border_output << status::output << status::internal;
    EXPECT_EQ(ss.str(), "border_outputoutputinternal");
}

template <typename node_t>
int spawning(node_t& node, trace_t call_point, bool b) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    common::option<tuple_t> kt;
    if (b) kt.emplace(node.uid);
    auto mt = coordination::spawn(node, 0, [&](tuple_t ti){
        int i = common::get<tag>(ti);
        return make_tuple(i, (int)node.uid >= i);
    }, kt);
    int c = 0;
    for (auto const& x  : mt) c += 1 << (common::get<tag>(x.first) * x.second);
    common::option<int> k;
    if (b) k.emplace(node.uid);
    auto m = coordination::spawn(node, 1, [&](int i, bool, char){
        return make_tuple(i, (int)node.uid >= i ? status::output : status::border);
    }, k, false, 'a');
    if (b) assert(m.size() > 0);
    for (auto const& x  : m) c += 1 << (x.first * x.second);
    auto mf = coordination::spawn(node, 2, [&](int i, bool, char){
        return make_tuple(i, node.nbr_uid() >= i);
    }, k, false, 'a');
    if (b) assert(mf.size() > 0);
    for (auto const& x  : mf) c += 1 << (x.first * x.second);
    return c;
}

MULTI_TEST(BasicsTest, Spawn, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid>(0)};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid>(1)};
    typename combo<O>::node d2{network, common::make_tagged_tuple<uid>(2)};
    int d;
    d = spawning(d0, 0, false);
    EXPECT_EQ(0, d);
    d = spawning(d1, 0, false);
    EXPECT_EQ(0, d);
    d = spawning(d2, 0, false);
    EXPECT_EQ(0, d);
    sendall(d0, d1, d2);
    d = spawning(d0, 0, false);
    EXPECT_EQ(0+0+0, d);
    d = spawning(d1, 0, true);
    EXPECT_EQ(2+2+2, d);
    d = spawning(d2, 0, false);
    EXPECT_EQ(0+0+0, d);
    sendall(d0, d1, d2);
    d = spawning(d0, 0, false);
    EXPECT_EQ(0+2+0, d);
    d = spawning(d1, 0, false);
    EXPECT_EQ(2+2+2, d);
    d = spawning(d2, 0, false);
    EXPECT_EQ(2+2+2, d);
    sendall(d0, d1, d2);
    d = spawning(d0, 0, true);
    EXPECT_EQ(1+3+1, d);
    d = spawning(d1, 0, false);
    EXPECT_EQ(2+2+2, d);
    d = spawning(d2, 0, true);
    EXPECT_EQ(18+18+18, d);
    sendall(d0, d1, d2);
    d = spawning(d0, 0, false);
    EXPECT_EQ(1+19+1, d);
    d = spawning(d1, 0, true);
    EXPECT_EQ(3+19+3, d);
    d = spawning(d2, 0, true);
    EXPECT_EQ(19+19+19, d);
}

MULTI_TEST(BasicsTest, NbrUid, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid>(0)};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid>(1)};
    EXPECT_EQ(0, (int)details::get_ids(d0.nbr_uid()).size());
    d0.round_start(0);
    d0.round_end(0);
    EXPECT_EQ(1, (int)details::get_ids(d0.nbr_uid()).size());
    sendto(d1, d0);
    EXPECT_EQ(1, (int)details::get_ids(d0.nbr_uid()).size());
    d0.round_start(0);
    d0.round_end(0);
    EXPECT_EQ(1, (int)details::self(d0.nbr_uid(), 1));
    EXPECT_EQ(2, (int)details::get_ids(d0.nbr_uid()).size());
}

MULTI_TEST(BasicsTest, CountHood, O, 3) {
    test_net<combo<O>, std::tuple<int>(int)> n{
        [&](auto& node, int value){
            return std::make_tuple(
                coordination::count_hood(node, 0)
            );
        }
    };
    EXPECT_ROUND(n, {1, 2, 4},
                    {1, 1, 1});
    EXPECT_ROUND(n, {1, 2, 4},
                    {2, 3, 2});
    EXPECT_ROUND(n, {1, 2, 4},
                    {2, 3, 2});
}

template <typename node_t>
int splitting(node_t& node, trace_t call_point, tuple<int, double> t) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return coordination::split(node, 0, t, [&](){
        return coordination::count_hood(node, 1);
    });
}

MULTI_TEST(BasicsTest, Split, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid>(0)};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid>(1)};
    typename combo<O>::node d2{network, common::make_tagged_tuple<uid>(2)};
    int d;
    d = splitting(d0, 0, {4, 2.0});
    EXPECT_EQ(1, d);
    d = splitting(d1, 0, {2, 4.0});
    EXPECT_EQ(1, d);
    d = splitting(d2, 0, {4, 2.0});
    EXPECT_EQ(1, d);
    sendall(d0, d1, d2);
    d = splitting(d0, 0, {4, 2.0});
    EXPECT_EQ(2, d);
    d = splitting(d1, 0, {2, 4.0});
    EXPECT_EQ(1, d);
    d = splitting(d2, 0, {4, 2.0});
    EXPECT_EQ(2, d);
    sendall(d0, d1, d2);
    d = splitting(d0, 0, {4, 2.0});
    EXPECT_EQ(2, d);
    d = splitting(d1, 0, {2, 4.0});
    EXPECT_EQ(1, d);
    d = splitting(d2, 0, {4, 2.0});
    EXPECT_EQ(2, d);
}
