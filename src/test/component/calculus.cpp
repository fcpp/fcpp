// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#define FCPP_WARNING_TRACE false

#include <algorithm>

#include "gtest/gtest.h"

#include "lib/common/option.hpp"
#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"

#include "test/helper.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
using combo = component::combine_spec<
    component::calculus<
        exports<common::export_list<spawn_t<int, bool>, spawn_t<int, status>, field<int>, times_t, int>>,
        export_pointer<(O & 1) == 1>,
        export_split<(O & 2) == 2>,
        online_drop<(O & 4) == 4>
    >,
    component::base<>
>;


template <typename node_t>
times_t delayed(node_t& node, trace_t call_point, times_t t) {
    return old(node, call_point, t);
}

template <typename node_t>
times_t delayed(node_t& node, trace_t call_point, times_t start, times_t t) {
    return old(node, call_point, t, start);
}

template <typename node_t>
int counter(node_t& node, trace_t call_point) {
    return old(node, call_point, 0, [](int const& o) {
        return o+1;
    });
}

template <typename node_t>
int sharing(node_t& node, trace_t call_point, int x) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return fold_hood(node, 0, [](int x, int y) {
        return x+y;
    }, nbr(node, 1, x));
}

template <typename node_t>
int gossip(node_t& node, trace_t call_point, int x) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    return nbr(node, 0, x, [&](field<int> n) {
        return std::max(fold_hood(node, 1, [](int x, int y) {
            return std::max(x,y);
        }, n), x);
    });
}

template <typename node_t>
int spawning(node_t& node, trace_t call_point, bool b) {
    internal::trace_call trace_caller(node.stack_trace, call_point);
    common::option<int> k;
    if (b) k.emplace(node.uid);
    auto m = spawn(node, 0, [&](int i){
        return make_tuple(i, node.uid >= i);
    }, k);
    int c = 0;
    for (auto const& x  : m) c += 1 << (x.first * x.second);
    m = spawn(node, 1, [&](int i, bool, char){
        return make_tuple(i, node.uid >= i ? status::output : status::external);
    }, k, false, 'a');
    if (b) assert(m.size() > 0);
    for (auto const& x  : m) c += 1 << (x.first * x.second);
    return c;
}


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

template <typename T>
void rounder(T& node) {
    node.round_end(0);
    node.round_start(0);
}


MULTI_TEST(CalculusTest, Size, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid, hoodsize>(0, device_t(3))};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid>(1)};
    typename combo<O>::node d2{network, common::make_tagged_tuple<uid>(2)};
    typename combo<O>::node d3{network, common::make_tagged_tuple<uid>(3)};
    typename combo<O>::node d4{network, common::make_tagged_tuple<uid>(4)};
    d0.round_start(0);
    EXPECT_EQ(1, (int)d0.size());
    d0.round_end(0);
    sendto(d0, d0);
    d0.round_start(0);
    EXPECT_EQ(1, (int)d0.size());
    d0.round_end(0);
    sendto(d1, d0);
    d0.round_start(0);
    EXPECT_EQ(2, (int)d0.size());
    d0.round_end(0);
    sendto(d2, d0);
    d0.round_start(0);
    EXPECT_EQ(2, (int)d0.size());
    d0.round_end(0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    d0.round_start(0);
    EXPECT_EQ(3, (int)d0.size());
    d0.round_end(0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    sendto(d3, d0);
    d0.round_start(0);
    EXPECT_EQ(3, (int)d0.size());
    d0.round_end(0);
    sendto(d0, d0);
    sendto(d1, d0);
    sendto(d2, d0);
    sendto(d3, d0);
    sendto(d4, d0);
    d0.round_start(0);
    EXPECT_EQ(3, (int)d0.size());
    d0.round_end(0);
    d0.round_start(0);
    EXPECT_EQ(1, (int)d0.size());
    d0.round_end(0);
    sendto(d4, d0);
    d0.round_start(0);
    EXPECT_EQ(2, (int)d0.size());
    d0.round_end(0);
}

MULTI_TEST(CalculusTest, Old, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid>(0)};
    times_t d;
    d = delayed(d0, 0, 1);
    EXPECT_EQ(1, d);
    sendto(d0, d0);
    rounder(d0);
    d = delayed(d0, 0, 3);
    EXPECT_EQ(1, d);
    rounder(d0);
    d = delayed(d0, 0, 5);
    EXPECT_EQ(1, d);
    sendto(d0, d0);
    rounder(d0);
    d = delayed(d0, 0, 3);
    EXPECT_EQ(5, d);
    d = delayed(d0, 1, 3, 2);
    EXPECT_EQ(2, d);
    sendto(d0, d0);
    rounder(d0);
    d = delayed(d0, 1, 6, 2);
    EXPECT_EQ(3, d);
    d = counter(d0, 2);
    EXPECT_EQ(1, d);
    sendto(d0, d0);
    rounder(d0);
    d = counter(d0, 2);
    EXPECT_EQ(2, d);
    sendto(d0, d0);
    rounder(d0);
    d = counter(d0, 2);
    EXPECT_EQ(3, d);
}

MULTI_TEST(CalculusTest, Nbr, O, 3) {
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
    d0.round_start(0);
    d = gossip(d0, 1, 1);
    EXPECT_EQ(4, d);
}

template <status x>
struct converter {
    static constexpr int value() {
        return (int)x;
    }
};

TEST(CalculusTest, Status) {
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
}

TEST(CalculusTest, Spawn) {
    constexpr size_t O = 0;
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
    EXPECT_EQ(0, d);
    d = spawning(d1, 0, true);
    EXPECT_EQ(2+2, d);
    d = spawning(d2, 0, false);
    EXPECT_EQ(0, d);
    sendall(d0, d1, d2);
    d = spawning(d0, 0, false);
    EXPECT_EQ(0+2, d);
    d = spawning(d1, 0, false);
    EXPECT_EQ(2+2, d);
    d = spawning(d2, 0, false);
    EXPECT_EQ(2+2, d);
    sendall(d0, d1, d2);
    d = spawning(d0, 0, true);
    EXPECT_EQ(1+3, d);
    d = spawning(d1, 0, false);
    EXPECT_EQ(2+2, d);
    d = spawning(d2, 0, true);
    EXPECT_EQ(18+18, d);
    sendall(d0, d1, d2);
    d = spawning(d0, 0, false);
    EXPECT_EQ(1+19, d);
    d = spawning(d1, 0, true);
    EXPECT_EQ(3+19, d);
    d = spawning(d2, 0, true);
    EXPECT_EQ(19+19, d);
}
