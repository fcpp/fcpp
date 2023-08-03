// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include <cstdio>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/logger.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/storage.hpp"

#include "test/helper.hpp"

using namespace fcpp;
using namespace component::tags;


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

using seq_per = sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::uniform_n<times_t, 2, 10, 1, meantag, devtag>, distribution::constant_n<times_t, 62, 10>, distribution::constant_n<size_t, 5>>;

template <int O>
using combo1 = component::combine_spec<
    exposer,
    fakeid,
    component::logger<
        parallel<(O & 1) == 1>,
        value_push<true>,
        log_schedule<seq_per>,
        aggregators<gat,aggregator::mean<double>>,
        log_functors<tag, functor::add<aggregator::mean<gat>, plot::time>>
    >,
    component::storage<tuple_store<tag,bool,gat,int>>,
    component::base<parallel<(O & 1) == 1>>
>;
template <int O>
using combo2 = component::combine_spec<
    exposer,
    component::logger<
        parallel<(O & 1) == 1>,
        value_push<true>,
        log_schedule<seq_per>,
        aggregators<gat,aggregator::mean<double>,tag,aggregator::count<bool>>
    >,
    component::storage<tuple_store<tag,bool,gat,int>>,
    component::identifier<
        parallel<(O & 1) == 1>,
        synchronised<(O & 2) == 1>
    >,
    component::base<parallel<(O & 1) == 1>>
>;
using aggregator_t = aggregators<gat,aggregator::mean<double>,tag,aggregator::count<bool>>;
using plotter_t = plot::split<oth, plot::split<plot::time, plot::values<aggregator_t, common::type_sequence<>, gat, tag>>>;
template <int O>
using combo3 = component::combine_spec<
    exposer,
    component::logger<
        parallel<(O & 1) == 1>,
        value_push<true>,
        log_schedule<seq_per>,
        aggregator_t,
        extra_info<oth,int>,
        plot_type<plotter_t>
    >,
    component::storage<tuple_store<tag,bool,gat,int>>,
    component::identifier<
        parallel<(O & 1) == 1>,
        synchronised<(O & 2) == 1>
    >,
    component::base<parallel<(O & 1) == 1>>
>;


TEST(LoggerTest, MakeStream) {
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

MULTI_TEST(LoggerTest, Push, O, 1) {
    std::stringstream s;
    {
        typename combo1<O>::net network{common::make_tagged_tuple<output,devtag>(&s,0)};
        {
            typename combo1<O>::node device1{network, common::make_tagged_tuple<uid,oth,gat>(1,'b',5)};
            typename combo1<O>::node device2{network, common::make_tagged_tuple<uid,tag>(2,true)};
            typename combo1<O>::node device3{network, common::make_tagged_tuple<uid,gat>(3,1)};
            EXPECT_EQ(1.5f, network.next());
            network.update();
            EXPECT_EQ(3.5f, network.next());
            device1.round_start(2);
            device1.storage(tag{}) = true;
            device1.round_end(2);
            device3.round_start(2.5f);
            device3.storage(tag{}) = true;
            device3.storage(gat{}) = 3;
            device3.round_end(2.5f);
            device2.round_start(3);
            device2.storage(gat{}) = 1;
            device2.round_end(3);
            network.update();
            EXPECT_EQ(5.5f, network.next());
            EXPECT_EQ((network.template aggregator<aggregator::mean<gat>>()), 3.0);
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
    EXPECT_EQ("# time mean(gat) tag ", line);
    getline(s, line);
    EXPECT_EQ("1.5 2 3.5 ", line);
    getline(s, line);
    EXPECT_EQ("3.5 3 6.5 ", line);
    getline(s, line);
    EXPECT_EQ("5.5 nan nan ", line);
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

MULTI_TEST(LoggerTest, Pull, O, 2) {
    std::stringstream s;
    {
        typename combo2<O>::net network{common::make_tagged_tuple<output,devtag,name,fakeid>(&s, 0, "foo", false)};
        network.node_emplace(common::make_tagged_tuple<oth,gat>('b',5));
        network.node_emplace(common::make_tagged_tuple<tag>(true));
        network.node_emplace(common::make_tagged_tuple<gat>(1));
        EXPECT_EQ(1.5f, network.next());
        network.update();
        EXPECT_EQ(3.5f, network.next());
        {
            common::unique_lock<(O & 1) == 1> l;
            auto& n = network.node_at(0, l);
            n.round_start(2);
            n.storage(tag{}) = true;
            n.round_end(2);
        }
        {
            common::unique_lock<(O & 1) == 1> l;
            auto& n = network.node_at(2, l);
            n.round_start(2.5f);
            n.storage(tag{}) = true;
            n.storage(gat{}) = 3;
            n.round_end(2.5f);
        }
        {
            common::unique_lock<(O & 1) == 1> l;
            auto& n = network.node_at(1, l);
            n.round_start(3);
            n.storage(gat{}) = 1;
            n.round_end(3);
        }
        network.update();
        EXPECT_EQ(5.5f, network.next());
        EXPECT_EQ((network.template aggregator<aggregator::mean<gat>>()), 3.0);
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

MULTI_TEST(LoggerTest, Plot, O, 2) {
    plotter_t p;
    {
        typename combo3<O>::net network{common::make_tagged_tuple<output,devtag,name,fakeid,plotter,oth>("/dev/null", 0, "foo", false, &p, 42)};
        network.node_emplace(common::make_tagged_tuple<oth,gat>('b',5));
        network.node_emplace(common::make_tagged_tuple<tag>(true));
        network.node_emplace(common::make_tagged_tuple<gat>(1));
        EXPECT_EQ(1.5f, network.next());
        network.update();
        EXPECT_EQ(3.5f, network.next());
        {
            common::unique_lock<(O & 1) == 1> l;
            auto& n = network.node_at(0, l);
            n.round_start(2);
            n.storage(tag{}) = true;
            n.round_end(2);
        }
        {
            common::unique_lock<(O & 1) == 1> l;
            auto& n = network.node_at(2, l);
            n.round_start(2.5f);
            n.storage(tag{}) = true;
            n.storage(gat{}) = 3;
            n.round_end(2.5f);
        }
        {
            common::unique_lock<(O & 1) == 1> l;
            auto& n = network.node_at(1, l);
            n.round_start(3);
            n.storage(gat{}) = 1;
            n.round_end(3);
        }
        network.update();
        EXPECT_EQ(5.5f, network.next());
        network.node_erase(1);
        network.node_erase(2);
        network.node_erase(0);
        network.run();
    }
    std::stringstream s;
    s << plot::file("experiment", p.build());
    EXPECT_EQ(s.str(), "// experiment\nstring name = \"experiment\";\n\nimport \"plot.asy\" as plot;\nunitsize(1cm);\n\nplot.ROWS = 1;\nplot.COLS = 1;\n\nplot.put(plot.plot(name+\"-timy-oth42\", \"oth = 42\", \"time\", \"y\", new string[] {\"gat (mean-finite mean)\", \"tag (count-finite mean)\"}, new pair[][] {{(1.5, 2), (3.5, 3), (5.5, nan)}, {(1.5, 1), (3.5, 3), (5.5, 0)}}));\n\n\nshipout(\"experiment\");\n");
}

MULTI_TEST(LoggerTest, Nulls, O, 2) {
    typename combo3<O>::net network{common::make_tagged_tuple<output,devtag,name,fakeid,oth>(nullptr, 0, "foo", false, 42)};
    network.node_emplace(common::make_tagged_tuple<oth,gat>('b',5));
    network.node_emplace(common::make_tagged_tuple<tag>(true));
    network.node_emplace(common::make_tagged_tuple<gat>(1));
    EXPECT_EQ(1.5f, network.next());
    network.update();
    EXPECT_EQ(3.5f, network.next());
    {
        common::unique_lock<(O & 1) == 1> l;
        auto& n = network.node_at(0, l);
        n.round_start(2);
        n.storage(tag{}) = true;
        n.round_end(2);
    }
    {
        common::unique_lock<(O & 1) == 1> l;
        auto& n = network.node_at(2, l);
        n.round_start(2.5f);
        n.storage(tag{}) = true;
        n.storage(gat{}) = 3;
        n.round_end(2.5f);
    }
    {
        common::unique_lock<(O & 1) == 1> l;
        auto& n = network.node_at(1, l);
        n.round_start(3);
        n.storage(gat{}) = 1;
        n.round_end(3);
    }
    network.update();
    EXPECT_EQ(5.5f, network.next());
    network.node_erase(1);
    network.node_erase(2);
    network.node_erase(0);
    network.run();
}
