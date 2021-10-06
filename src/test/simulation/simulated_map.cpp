// Copyright © 2021 Gianmarco Rampulla. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/simulation/simulated_map.hpp"

#include "lib/data/vec.hpp"

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

   vec<2> area_m = make_vec(1000,1000);
   vec<2> area_mi = make_vec(100,100);
   combo1::net network{common::make_tagged_tuple<obstacles, area_min, area_max>("image_url", area_mi, area_m)};

}

