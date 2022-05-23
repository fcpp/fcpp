// Copyright © 2021 Gianmarco Rampulla. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/simulation/simulated_map.hpp"

#include "./external/stb_image/stb_image.h"

#include "lib/data/vec.hpp"
#include <fstream>
#include <cstdio>
#include <vector>

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
   //create test bitmap
   std::vector<unsigned char> bitmap{137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,4,0,0,0,4,8,6,0,0,0,169,241,158,126,0,0,0,31,73,68,65,84,24,87,99,100,96,96,248,207,192,192,192,200,0,5,112,6,186,0,72,21,8,48,194,84,192,5,0,99,218,3,5,49,47,111,178,0,0,0,0,73,69,78,68,174,66,96,130,0,0};

#if _WIN32
    _mkdir(".\\textures\\");
     std::ofstream test_bitmap(".\\textures\\bitmap_test.png",std::ios::binary);
#else
    mkdir("./textures/", 0777);
    std::ofstream test_bitmap("./textures/bitmap_test.png",std::ios::binary);
#endif

   test_bitmap.write((char *)bitmap.data(),bitmap.size());
   test_bitmap.close();
   //start analysis
   combo1::net net{common::make_tagged_tuple<obstacles, area_min, area_max>("bitmap_test.png", make_vec(0,0), make_vec(40,40))};
   EXPECT_TRUE(net.is_obstacle(make_vec(0.82,32.36)));
   EXPECT_FALSE(net.is_obstacle(make_vec(11.45,33.58)));
   EXPECT_EQ(net.closest_obstacle(make_vec(1.78,23.78)), make_vec(1.78,30));
   EXPECT_EQ(net.closest_obstacle(make_vec(14.3,36.7)), make_vec(10,36.7));
   EXPECT_EQ(net.closest_space(make_vec(0.82,32.36)), make_vec(0.82,30));
   EXPECT_EQ(net.closest_space(make_vec(23.45,13.64)), make_vec(30,13.64));

   //remove test bitmap
#if _WIN32
    std::remove(".\\textures\\bitmap_test.png");
    _rmdir(".\\textures\\");
#else
    std::remove("./textures/bitmap_test.png");
    rmdir("./textures/");
#endif

}