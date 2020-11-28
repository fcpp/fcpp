// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <vector>

#include "gtest/gtest.h"

#include "lib/option/connect.hpp"

using namespace fcpp;


TEST(ConnectTest, Clique) {
    connect::clique<> connector(nullptr, common::make_tagged_tuple<>());
    connect::clique<>::data_type data;
    bool connect;
    connect = connector(data, make_vec(0.5f,1), data, make_vec(0.4f,0.9f));
    EXPECT_TRUE(connect);
    connect = connector(data, make_vec(0.5f,1), data, make_vec(7,10));
    EXPECT_TRUE(connect);
    connect = connector(data, make_vec(0.5f,1), data, make_vec(0.5f,0));
    EXPECT_TRUE(connect);
    connect = connector(data, make_vec(0.5f,1), data, make_vec(0.51f,0));
    EXPECT_TRUE(connect);
}


TEST(ConnectTest, Fixed) {
    connect::fixed<1> connector(nullptr, common::make_tagged_tuple<>());
    connect::fixed<1>::data_type data;
    bool connect;
    connect = connector(data, make_vec(0.5f,1), data, make_vec(0.4f,0.9f));
    EXPECT_TRUE(connect);
    connect = connector(data, make_vec(0.5f,1), data, make_vec(7,10));
    EXPECT_FALSE(connect);
    connect = connector(data, make_vec(0.5f,1), data, make_vec(0.5f,0));
    EXPECT_TRUE(connect);
    connect = connector(data, make_vec(0.5f,1), data, make_vec(0.51f,0));
    EXPECT_FALSE(connect);
}
