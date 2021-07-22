// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <algorithm>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"

#include "test/helper.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
using combo = component::combine_spec<
    component::calculus<
        exports<field<int>, times_t, int>,
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


template <typename T>
void sendto(T const& source, T& dest) {
    typename T::message_t m;
    dest.receive(0, source.uid, source.send(0, m));
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
