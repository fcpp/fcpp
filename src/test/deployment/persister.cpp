// Copyright © 2022 Giorgio Audrito. All Rights Reserved.

#include <cstdio>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/component/scheduler.hpp"
#include "lib/component/storage.hpp"
#include "lib/coordination/basics.hpp"
#include "lib/deployment/hardware_identifier.hpp"
#include "lib/deployment/persister.hpp"

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


struct main {
    template <typename node_t>
    void operator()(node_t& node, times_t) {
        node.storage(gat{}) += 1;
        node.storage(oth{}) = coordination::old(node, 0, 0, [](int const& o) {
            return o+1;
        });
    }
};

template <typename T>
void sendto(T const& source, T& dest) {
    typename T::message_t m;
    dest.receive(0, source.uid, source.send(0, m));
}

using seq_per = sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 82, 10>, distribution::constant_n<size_t, 5>>;

template <int O>
using combo1 = component::combine_spec<
    component::scheduler<round_schedule<seq_per>>,
    component::persister<tuple_store<tag,bool,gat,int>>,
    component::calculus<
        program<main>,
        exports<common::export_list<int>>,
        export_pointer<(O & 1) == 1>,
        export_split<(O & 2) == 2>,
        online_drop<(O & 4) == 4>
    >,
    component::storage<tuple_store<tag,bool,gat,int,oth,int>>,
    component::hardware_identifier<>,
    component::base<>
>;


MULTI_TEST(PersisterTest, Main, O, 3) {
    remove(".persistence");
    {
        typename combo1<O>::net network{common::make_tagged_tuple<persistence_path>(".persistence")};
        common::unique_lock<FCPP_PARALLEL> l;
        auto& n = network.node_at(42, l);
        EXPECT_EQ(0, n.storage(gat{}));
        EXPECT_EQ(0, n.storage(oth{}));
        EXPECT_EQ(1.5f, network.next());
        network.update();
        sendto(n, n);
        EXPECT_EQ(1, n.storage(gat{}));
        EXPECT_EQ(1, n.storage(oth{}));
        EXPECT_EQ(3.5f, network.next());
        network.update();
        sendto(n, n);
        EXPECT_EQ(2, n.storage(gat{}));
        EXPECT_EQ(2, n.storage(oth{}));
        EXPECT_EQ(5.5f, network.next());
    }
    {
        typename combo1<O>::net network{common::make_tagged_tuple<persistence_path>(".persistence")};
        common::unique_lock<FCPP_PARALLEL> l;
        auto& n = network.node_at(42, l);
        EXPECT_EQ(2, n.storage(gat{}));
        EXPECT_EQ(2, n.storage(oth{}));
        EXPECT_EQ(1.5f, network.next());
        network.update();
        sendto(n, n);
        EXPECT_EQ(3, n.storage(gat{}));
        EXPECT_EQ(3, n.storage(oth{}));
        EXPECT_EQ(3.5f, network.next());
        network.update();
        sendto(n, n);
        EXPECT_EQ(4, n.storage(gat{}));
        EXPECT_EQ(4, n.storage(oth{}));
        EXPECT_EQ(5.5f, network.next());
        network.run();
    }
    remove(".persistence");
}
