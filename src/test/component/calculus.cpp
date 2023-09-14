// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#define FCPP_WARNING_TRACE false

#include <algorithm>
#include <sstream>

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
        exports<common::export_list<int>>,
        export_pointer<(O & 1) == 1>,
        export_split<(O & 2) == 2>,
        online_drop<(O & 4) == 4>
    >,
    component::base<>
>;

template <typename T>
void sendto(T const& source, T& dest) {
    typename T::message_t m;
    dest.receive(0, source.uid, source.send(0, m));
}


MULTI_TEST(CalculusTest, SizeThreshold, O, 3) {
    typename combo<O>::net  network{common::make_tagged_tuple<>()};
    typename combo<O>::node d0{network, common::make_tagged_tuple<uid, hoodsize>(0, device_t(3))};
    typename combo<O>::node d1{network, common::make_tagged_tuple<uid>(1)};
    typename combo<O>::node d2{network, common::make_tagged_tuple<uid>(2)};
    typename combo<O>::node d3{network, common::make_tagged_tuple<uid>(3)};
    typename combo<O>::node d4{network, common::make_tagged_tuple<uid>(4)};
    EXPECT_EQ(1, d0.message_threshold());
    d0.message_threshold(2);
    EXPECT_EQ(2, d0.message_threshold());
    d0.message_threshold(1);
    EXPECT_EQ(1, d0.message_threshold());
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
