// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/collection.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<
        coordination::gossip_max_t<int>,
        coordination::gossip_mean_t<real_t>,
        coordination::sp_collection_t<int,real_t>,
        coordination::mp_collection_t<int,real_t>,
        coordination::wmp_collection_t<real_t>,
        coordination::list_idem_collection_t<int>        
    >,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

template <class...>
struct lagdist {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;

            field<real_t> nbr_dist() const {
                return {1};
            }

            field<real_t> nbr_lag() const {
                return {1};
            }

            times_t current_time() const {
                return m_t;
            }

            times_t next_time() const {
                return m_t+1;
            }

            void round_start(times_t t) {
                P::node::round_start(t);
                m_t = t;
            }

          private:
            times_t m_t;
        };
        using net = typename P::net;
    };
};
DECLARE_COMBINE(calc_dist, lagdist, component::calculus);

template <int O>
using combo = calc_dist<options<O>>;

template <class...>
struct lagdist1 {
    template <typename F, typename P>
    struct component : public P {
        struct node : public P::node {
            using P::node::node;

            field<real_t> nbr_dist() const {
                device_t uid = P::node::uid;
                if (uid == 0)
                    return fcpp::details::make_field<real_t>({0,1,2}, {INF,0,1,5});
                else if (uid == 1)
                    return fcpp::details::make_field<real_t>({0,1,3}, {INF,1,0,6});
                else if (uid == 2)
                    return fcpp::details::make_field<real_t>({0,2,3}, {INF,5,0,10});             
                else
                    return fcpp::details::make_field<real_t>({1,2,3}, {INF,6,10,0});
            }

            field<real_t> nbr_lag() const {
                return {1};
            }

            times_t current_time() const {
                return m_t;
            }

            times_t next_time() const {
                return m_t+1;
            }

            void round_start(times_t t) {
                P::node::round_start(t);
                m_t = t;
            }

          private:
            times_t m_t;
        };
        using net = typename P::net;
    };
};
DECLARE_COMBINE(calc_dist1, lagdist1, component::calculus);

template <int O>
using combo1 = calc_dist1<options<O>>;


real_t adder(real_t x, real_t y) {
    return x+y;
}

real_t divider(real_t x, size_t n) {
    return x/n;
}

real_t multiplier(real_t x, real_t f) {
    return x*f;
}


MULTI_TEST(CollectionTest, Gossip, O, 3) {
    {
        test_net<combo<O>, std::tuple<int>(int)> n{
            [&](auto& node, int val){
                return std::make_tuple(
                    coordination::gossip_max(node, 0, val)
                );
            }
        };
        EXPECT_ROUND(n, {0, 1, 2},
                        {0, 1, 2});
        EXPECT_ROUND(n, {0, 1, 0},
                        {1, 2, 2});
        EXPECT_ROUND(n, {0, 0, 0},
                        {2, 2, 2});
        EXPECT_ROUND(n, {0, 3, 0},
                        {2, 3, 2});
        EXPECT_ROUND(n, {0, 3, 0},
                        {3, 3, 3});
    }
    {
        test_net<combo<O>, std::tuple<real_t>(real_t)> n{
            [&](auto& node, real_t val){
                return std::make_tuple(
                    coordination::gossip_mean(node, 0, val)
                );
            }
        };
        EXPECT_ROUND(n, {0, 4, 8},
                        {0, 4, 8});
        EXPECT_ROUND(n, {0, 4, 8},
                        {2, 4, 6});
        EXPECT_ROUND(n, {6, 4, 2},
                        {5, 4, 3});
    }
}

MULTI_TEST(CollectionTest, SP, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(int, real_t)> n{
        [&](auto& node, int id, real_t val){
            return std::make_tuple(
                coordination::sp_collection(node, 0, id, val, 0, adder)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {3, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
}

MULTI_TEST(CollectionTest, MP, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(int, real_t)> n{
        [&](auto& node, int id, real_t val){
            return std::make_tuple(
                coordination::mp_collection(node, 0, id, val, 0, adder, divider)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {3, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
}

MULTI_TEST(CollectionTest, WMP, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(int, real_t)> n{
        [&](auto& node, int id, real_t val){
            return std::make_tuple(
                coordination::wmp_collection(node, 0, id, 2, val, adder, multiplier)
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {3, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {7, 6, 4});
}


MULTI_TEST(CollectionTest, LISTidem, O, 3) {
    test_net<combo<O>, std::tuple<real_t>(int, int)> n{
        [&](auto& node, int id, int val){
            return std::make_tuple(
                coordination::list_idem_collection(node, 0, id, val, 2, 0, 0, 1,[&](int x, int y){return std::max(x,y);})
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {1, 2, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {2, 4, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {4, 4, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 4},
                    {4, 4, 4});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 2},
                    {4, 4, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 2},
                    {4, 2, 2});
    EXPECT_ROUND(n, {0, 1, 2},
                    {1, 2, 2},
                    {2, 2, 2}); 
}

MULTI_TEST(CollectionTest, LISTidemGraph, O, 3) {
    using topo_type = std::vector<std::vector<int>>;
    std::vector<real_t> distance {0,1,5,7}; 

    test_net<combo1<0>, std::tuple<real_t>(int, int), 4> n{
        {{0,1,2},{0,1,3},{0,2,3},{1,2,3}}, 
        [&](auto& node, int id, int val){
            return std::make_tuple(
                coordination::list_idem_collection(node, 0, distance[id], val, 10, 0, 0, 1,[&](int x, int y){return std::max(x,y);})
            );
        }
    };
    EXPECT_ROUND(n, {0, 1, 2, 3},
                    {0, 1, 2, 3},
                    {0, 1, 2, 3});
    EXPECT_ROUND(n, {0, 1, 2, 3},
                    {0, 1, 2, 3},
                    {2, 3, 3, 3});  
    EXPECT_ROUND(n, {0, 1, 2, 3},
                    {0, 1, 2, 3},
                    {3, 3, 2, 3});   
    EXPECT_ROUND(n, {0, 1, 2, 3},
                    {0, 1, 2, 10},
                    {3, 3, 2, 10});                                                             
    EXPECT_ROUND(n, {0, 1, 2, 3},
                    {0, 1, 2, 10},
                    {3, 10, 2, 10});
    EXPECT_ROUND(n, {0, 1, 2, 3},
                    {0, 1, 2, 10},
                    {10, 10, 2, 10});                            
}
