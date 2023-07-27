// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include <cstdio>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/component/scheduler.hpp"
#include "lib/component/storage.hpp"
#include "lib/deployment/hardware_logger.hpp"
#include "lib/deployment/hardware_identifier.hpp"

#include "test/fake_os.hpp"
#include "test/helper.hpp"

namespace fcpp {
    namespace component {
        namespace tags {
            struct start {};
        }
    }
}

using namespace fcpp;
using namespace component::tags;


struct tag {};
struct gat {};
struct oth {};


using seq_per = sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 82, 10>, distribution::constant_n<size_t, 5>>;

template <int O>
using combo1 = component::combine_spec<
    component::scheduler<round_schedule<seq_per>>,
    component::hardware_logger<tuple_store<tag,bool,gat,int>>,
    component::storage<tuple_store<tag,bool,gat,int>>,
    component::hardware_identifier<parallel<(O & 1) == 1>>,
    component::base<parallel<(O & 1) == 1>>
>;


TEST(HardwareLoggerTest, MakeStream) {
    common::tagged_tuple_t<name,char const*,uid,int,oth,char,gat,bool> t{"bar",7,'b',false};
    std::shared_ptr<std::ostream> p;
    p = component::details::make_stream<std::ostream>("foo", t);
    p = component::details::make_stream<std::ostream>(std::string("foo"), t);
    p = component::details::make_stream<std::ostream>("foo/", t);
    std::stringstream s;
    p = component::details::make_stream<std::ostream>(&s, t);
    *p << "foo";
    EXPECT_EQ("foo", s.str());
    std::remove("foo");
}

MULTI_TEST(HardwareLoggerTest, Main, O, 1) {
    std::stringstream s;
    {
        typename combo1<O>::net network{common::make_tagged_tuple<output,tag,gat,oth>(&s,true,42,0.0f)};
        EXPECT_EQ(1.5f, network.next());
        network.update();
        {
            common::unique_lock<(O & 1) == 1> l;
            network.node_at(42, l).storage(tag{}) = false;
        }
        EXPECT_EQ(3.5f, network.next());
        network.update();
        {
            common::unique_lock<(O & 1) == 1> l;
            network.node_at(42, l).storage(gat{}) = 10;
        }
        EXPECT_EQ(5.5f, network.next());
        network.run();
        EXPECT_EQ(TIME_MAX, network.next());
    }
    std::string line;
    getline(s, line);
    EXPECT_EQ(56, (int)line.size());
    EXPECT_EQ("########################################################", line);
    getline(s, line);
    EXPECT_EQ(56, (int)line.size());
    EXPECT_EQ("# FCPP execution started at:  ", line.substr(0, 30));
    getline(s, line);
    EXPECT_EQ(56, (int)line.size());
    EXPECT_EQ("########################################################", line);
    getline(s, line);
    EXPECT_EQ("# tag = true, gat = 42, oth = 0, start = 0, uid = 42", line);
    getline(s, line);
    EXPECT_EQ("#", line);
    getline(s, line);
    EXPECT_EQ("# The columns have the following meaning:", line);
    getline(s, line);
    EXPECT_EQ("# time tag gat ", line);
    getline(s, line);
    EXPECT_EQ("0 true 42 ", line);
    getline(s, line);
    EXPECT_EQ("1.5 true 42 ", line);
    getline(s, line);
    EXPECT_EQ("3.5 false 42 ", line);
    getline(s, line);
    EXPECT_EQ("5.5 false 10 ", line);
    getline(s, line);
    EXPECT_EQ("7.5 false 10 ", line);
    getline(s, line);
    EXPECT_EQ("########################################################", line);
    getline(s, line);
    EXPECT_EQ(56, (int)line.size());
    EXPECT_EQ("# FCPP execution finished at: ", line.substr(0, 30));
    getline(s, line);
    EXPECT_EQ("########################################################", line);
    getline(s, line);
    EXPECT_EQ("", line);
}

MULTI_TEST(HardwareLoggerTest, Nulls, O, 1) {
    {
        typename combo1<O>::net network{common::make_tagged_tuple<output,tag,gat,oth>(nullptr,true,42,0.0f)};
        EXPECT_EQ(1.5f, network.next());
        network.update();
        {
            common::unique_lock<(O & 1) == 1> l;
            network.node_at(42, l).storage(tag{}) = false;
        }
        EXPECT_EQ(3.5f, network.next());
        network.update();
        {
            common::unique_lock<(O & 1) == 1> l;
            network.node_at(42, l).storage(gat{}) = 10;
        }
        EXPECT_EQ(5.5f, network.next());
        network.run();
        EXPECT_EQ(TIME_MAX, network.next());
    }
}
