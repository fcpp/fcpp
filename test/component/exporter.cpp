// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/common/aggregator.hpp"
#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"
#include "lib/component/base.hpp"
#include "lib/component/exporter.hpp"
#include "lib/component/storage.hpp"

using namespace fcpp;


struct tag {};
struct gat {};
struct oth {};
struct meantag {};
struct devtag  {};

// Component exposing the storage interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using P::node::storage;
            using P::node::storage_tuple;
        };
        using net  = typename P::net;
    };
};

using seq_per = random::sequence_periodic<random::constant_distribution<times_t, 15, 10>, random::uniform_d<times_t, 2, 10, 1, meantag, devtag>, random::constant_distribution<times_t, 62, 10>, random::constant_distribution<size_t, 5>>;

using combo1 = component::combine<
    exposer,
    component::exporter<true,seq_per,gat,aggregator::mean<double>>,
    component::storage<tag,bool,gat,int>
>;
//using combo2 = component::combine<
//    exposer,
//    component::exporter<false,seq_per,gat,aggregator::mean<double>,tag,aggregator::count<bool>>,
//    component::storage<tag,bool,gat,int>,
//    component::identifier<> // TODO
//>;


TEST(ExporterTest, MakeStream) {
    common::tagged_tuple_t<component::tags::name,const char*,component::tags::uid,int,oth,char,gat,bool> t{"bar",7,'b',false};
    std::shared_ptr<std::ostream> p;
    p = component::details::make_stream("foo", t);
    p = component::details::make_stream(std::string("foo"), t);
    p = component::details::make_stream(nullptr, t);
    p = component::details::make_stream("foo/", t);
    std::stringstream s;
    p = component::details::make_stream(&s, t);
    *p << "foo";
    EXPECT_EQ("foo", s.str());
}

TEST(ExporterTest, Push) {
    std::stringstream s;
    {
        combo1::net network{common::make_tagged_tuple<component::tags::output,devtag>(&s,0.0)};
        {
            combo1::node device1{network, common::make_tagged_tuple<component::tags::uid,oth,gat>(1,'b',5)};
            combo1::node device2{network, common::make_tagged_tuple<component::tags::uid,tag>(2,true)};
            combo1::node device3{network, common::make_tagged_tuple<component::tags::uid,gat>(3,1)};
            EXPECT_EQ(1.5, network.next());
            network.update();
            EXPECT_EQ(3.5, network.next());
            device1.round_start(2.0);
            device1.storage<tag>() = true;
            device1.round_end(2.0);
            device3.round_start(2.5);
            device3.storage<tag>() = true;
            device3.storage<gat>() = 3;
            device3.round_end(2.5);
            device2.round_start(3.0);
            device2.storage<gat>() = 1;
            device2.round_end(3.0);
            network.update();
            EXPECT_EQ(5.5, network.next());
        }
        network.run();
    }
    std::string line;
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("# FC++ data export started at:  ", line.substr(0, 32));
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ("# devtag = 0", line);
    getline(s, line);
    EXPECT_EQ("#", line);
    getline(s, line);
    EXPECT_EQ("# The columns have the following meaning:", line);
    getline(s, line);
    EXPECT_EQ("# time mean(gat) ", line);
    getline(s, line);
    EXPECT_EQ("1.5 2 ", line);
    getline(s, line);
    EXPECT_EQ("3.5 3 ", line);
    getline(s, line);
    EXPECT_EQ("5.5 -nan ", line);
    getline(s, line);
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("# FC++ data export finished at: ", line.substr(0, 32));
    getline(s, line);
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ("", line);
}

TEST(ExporterTest, Pull) {
    std::stringstream s;
    {
//        combo2::net network{common::make_tagged_tuple<component::tags::output,meantag,component::tags::name>(&s, 0.0, "foo")};
//        {
//            combo2::node device{network, common::make_tagged_tuple<component::tags::uid,oth,gat>(7,'b',3)};
//        }
    }
}
