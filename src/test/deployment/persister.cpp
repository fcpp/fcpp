// Copyright © 2022 Giorgio Audrito. All Rights Reserved.

#include <cstdio>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

#include "lib/component/scheduler.hpp"
#include "lib/component/storage.hpp"
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


// Component updating the storage.
struct progresser {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;
            void round_main(times_t t) {
                P::node::storage(gat{}) += 1;
            }
        };
        using net = typename P::net;
    };
};


using seq_per = sequence::periodic<distribution::constant_n<times_t, 15, 10>, distribution::constant_n<times_t, 2>, distribution::constant_n<times_t, 82, 10>, distribution::constant_n<size_t, 5>>;

template <int O>
using combo1 = component::combine_spec<
    progresser,
    component::scheduler<round_schedule<seq_per>>,
    component::persister<tuple_store<tag,bool,gat,int>>,
    component::storage<tuple_store<tag,bool,gat,int>>,
    component::hardware_identifier<parallel<(O & 1) == 1>>,
    component::base<parallel<(O & 1) == 1>>
>;


MULTI_TEST(PersisterTest, Main, O, 1) {
    remove(".persistence");
    {
        typename combo1<O>::net network{common::make_tagged_tuple<oth,persistence_path>(0.0f,".persistence")};
        auto const& n = network.node_at(42);
        EXPECT_EQ(0, n.storage(gat{}));
        EXPECT_EQ(1.5f, network.next());
        network.update();
        EXPECT_EQ(1, n.storage(gat{}));
        EXPECT_EQ(3.5f, network.next());
        network.run();
    }
    {
        typename combo1<O>::net network{common::make_tagged_tuple<oth,persistence_path>(0.0f,".persistence")};
        auto const& n = network.node_at(42);
        EXPECT_EQ(4, n.storage(gat{}));
        EXPECT_EQ(1.5f, network.next());
        network.update();
        EXPECT_EQ(5, n.storage(gat{}));
        EXPECT_EQ(3.5f, network.next());
        network.run();
    }
    remove(".persistence");
}
