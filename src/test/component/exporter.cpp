// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/common/aggregator.hpp"
#include "lib/common/distribution.hpp"
#include "lib/common/sequence.hpp"
#include "lib/component/base.hpp"
#include "lib/component/exporter.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"

using namespace fcpp;


struct tag {};
struct gat {};
struct oth {};
struct meantag {};
struct devtag  {};

// Fake identifier
struct fakeid {
    template <typename F, typename P>
    struct component : public P {
        using node = typename P::node;
        struct net : public P::net {
            using P::net::net;
            void node_emplace() {}
            void node_erase() {}
        };
    };
};

// Component exposing the storage interface.
struct exposer {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            using P::node::storage;
            using P::node::storage_tuple;
        };
        struct net : public P::net {
            using P::net::net;
            using P::net::node_emplace;
            using P::net::node_erase;
        };
    };
};

using seq_per = random::sequence_periodic<random::constant_distribution<times_t, 15, 10>, random::uniform_d<times_t, 2, 10, 1, meantag, devtag>, random::constant_distribution<times_t, 62, 10>, random::constant_distribution<size_t, 5>>;

using combo1 = component::combine<
    exposer,
    fakeid,
    component::exporter<component::tags::value_push<true>, component::tags::log_schedule<seq_per>, component::tags::aggregators<gat,aggregator::mean<double>>>,
    component::storage<component::tags::tuple_store<tag,bool,gat,int>>
>;
using combo2 = component::combine<
    exposer,
    component::exporter<component::tags::value_push<false>, component::tags::log_schedule<seq_per>, component::tags::aggregators<gat,aggregator::mean<double>,tag,aggregator::count<bool>>>,
    component::storage<component::tags::tuple_store<tag,bool,gat,int>>,
    component::identifier<component::tags::synchronised<false>>
>;


TEST(ExporterTest, MakeStream) {
    common::tagged_tuple_t<component::tags::name,const char*,component::tags::uid,int,oth,char,gat,bool> t{"bar",7,'b',false};
    std::shared_ptr<std::ostream> p;
    p = component::details::make_stream("foo", t);
    p = component::details::make_stream(std::string("foo"), t);
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
    EXPECT_EQ("# FCPP data export started at:  ", line.substr(0, 32));
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
    EXPECT_EQ("5.5 nan ", line);
    getline(s, line);
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("# FCPP data export finished at: ", line.substr(0, 32));
    getline(s, line);
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ("", line);
}

TEST(ExporterTest, Pull) {
    std::stringstream s;
    {
        combo2::net network{common::make_tagged_tuple<component::tags::output,devtag,component::tags::name,fakeid>(&s, 0.0, "foo",false)};
        network.node_emplace(common::make_tagged_tuple<oth,gat>('b',5));
        network.node_emplace(common::make_tagged_tuple<tag>(true));
        network.node_emplace(common::make_tagged_tuple<gat>(1));
        EXPECT_EQ(1.5, network.next());
        network.update();
        EXPECT_EQ(3.5, network.next());
        {
            common::unique_lock<FCPP_PARALLEL> l;
            auto& n = network.node_at(0, l);
            n.round_start(2.0);
            n.storage<tag>() = true;
            n.round_end(2.0);
        }
        {
            common::unique_lock<FCPP_PARALLEL> l;
            auto& n = network.node_at(2, l);
            n.round_start(2.5);
            n.storage<tag>() = true;
            n.storage<gat>() = 3;
            n.round_end(2.5);
        }
        {
            common::unique_lock<FCPP_PARALLEL> l;
            auto& n = network.node_at(1, l);
            n.round_start(3.0);
            n.storage<gat>() = 1;
            n.round_end(3.0);
        }
        network.update();
        EXPECT_EQ(5.5, network.next());
        network.node_erase(1);
        network.node_erase(2);
        network.node_erase(0);
        network.run();
    }
    std::string line;
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("# FCPP data export started at:  ", line.substr(0, 32));
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ("# devtag = 0, fakeid = false", line);
    getline(s, line);
    EXPECT_EQ("#", line);
    getline(s, line);
    EXPECT_EQ("# The columns have the following meaning:", line);
    getline(s, line);
    EXPECT_EQ("# time mean(gat) count(tag) ", line);
    getline(s, line);
    EXPECT_EQ("1.5 2 1 ", line);
    getline(s, line);
    EXPECT_EQ("3.5 3 3 ", line);
    getline(s, line);
    EXPECT_EQ("5.5 nan 0 ", line);
    getline(s, line);
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ(58, (int)line.size());
    EXPECT_EQ("# FCPP data export finished at: ", line.substr(0, 32));
    getline(s, line);
    EXPECT_EQ("##########################################################", line);
    getline(s, line);
    EXPECT_EQ("", line);
}
