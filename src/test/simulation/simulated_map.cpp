// Copyright © 2021 Gianmarco Rampulla. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/simulation/simulated_map.hpp"

#include "test/helper.hpp"

using namespace fcpp;
using namespace component::tags;

struct tag {};
struct gat {};
struct oth {};

// Component exposing the storage interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        using node = typename P::node;
        using net  = typename P::net;
    };
};


using combo1 = component::combine_spec<
    exposer,
    component::simulated_map<dimension<2>>,
    component::base<>
>;


TEST(SimulatedMapTest, CollisionTest) {

    combo1::net network{common::make_tagged_tuple<oth>("foo")};

}