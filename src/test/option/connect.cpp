// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <algorithm>
#include <vector>

#include "gtest/gtest.h"

#include "lib/option/connect.hpp"

using namespace fcpp;


TEST(ConnectTest, Clique) {
    connect::clique<3> connector(nullptr, common::make_tagged_tuple<>());
    connect::clique<3>::data_type data;
    bool connect;
    connect = connector(nullptr, data, make_vec(0.5f,1,3), data, make_vec(0.4f,0.9f,3));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1,3), data, make_vec(7,10,3));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1,3), data, make_vec(0.5f,0,3));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1,3), data, make_vec(0.51f,0,3));
    EXPECT_TRUE(connect);
}

TEST(ConnectTest, Fixed) {
    connect::fixed<1> connector(nullptr, common::make_tagged_tuple<>());
    connect::fixed<1>::data_type data;
    bool connect;
    connect = connector(nullptr, data, make_vec(0.5f,1), data, make_vec(0.4f,0.9f));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1), data, make_vec(7,10));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1), data, make_vec(0.5f,0));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1), data, make_vec(0.51f,0));
    EXPECT_FALSE(connect);
}

TEST(ConnectTest, Radial) {
    connect::radial<70, connect::fixed<10>> connector(nullptr, common::make_tagged_tuple<>());
    connect::radial<70, connect::fixed<10>>::data_type data;
    std::mt19937_64 gen(42);
    int count = 0;
    for (size_t i=0; i<100000; ++i)
        count += connector(gen, data, make_vec(0.5f,1), data, make_vec(7.5f,1));
    EXPECT_NEAR(50000, count, 500);
    count = 0;
    for (size_t i=0; i<100000; ++i)
        count += connector(gen, data, make_vec(1,0.5f), data, make_vec(1,10.5f));
    EXPECT_NEAR(1000, count, 100);
}

TEST(ConnectTest, Powered) {
    connect::powered<4> connector(nullptr, common::make_tagged_tuple<>());
    connect::powered<4>::data_type data = 0.5f;
    bool connect;
    connect = connector(nullptr, data, make_vec(0.5f,1), data, make_vec(0.4f,0.9f));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1), data, make_vec(7,10));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1), data, make_vec(0.5f,0));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data, make_vec(0.5f,1), data, make_vec(0.51f,0));
    EXPECT_FALSE(connect);
}

TEST(ConnectTest, Hierarchical) {
    connect::hierarchical<connect::fixed<1>> connector(nullptr, common::make_tagged_tuple<>());
    connect::hierarchical<connect::fixed<1>>::data_type data1 = 0;
    connect::hierarchical<connect::fixed<1>>::data_type data2 = 1;
    connect::hierarchical<connect::fixed<1>>::data_type data3 = 2;
    bool connect;
    connect = connector(nullptr, data1, make_vec(0.5f,1), data1, make_vec(0.4f,0.9f));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data1, make_vec(7,10));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data1, make_vec(0.5f,0));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data1, make_vec(0.51f,0));
    EXPECT_FALSE(connect);

    connect = connector(nullptr, data1, make_vec(0.5f,1), data2, make_vec(0.4f,0.9f));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data2, make_vec(7,10));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data2, make_vec(0.5f,0));
    EXPECT_TRUE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data2, make_vec(0.51f,0));
    EXPECT_FALSE(connect);

    connect = connector(nullptr, data2, make_vec(0.5f,1), data2, make_vec(0.4f,0.9f));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data2, make_vec(0.5f,1), data2, make_vec(7,10));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data2, make_vec(0.5f,1), data2, make_vec(0.5f,0));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data2, make_vec(0.5f,1), data2, make_vec(0.51f,0));
    EXPECT_FALSE(connect);

    connect = connector(nullptr, data1, make_vec(0.5f,1), data3, make_vec(0.4f,0.9f));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data3, make_vec(7,10));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data3, make_vec(0.5f,0));
    EXPECT_FALSE(connect);
    connect = connector(nullptr, data1, make_vec(0.5f,1), data3, make_vec(0.51f,0));
    EXPECT_FALSE(connect);
}
