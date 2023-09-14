// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file test_net.hpp
 * @brief Helper class for testing functions on networks.
 */

#ifndef FCPP_TEST_NET_H_
#define FCPP_TEST_NET_H_

#include <array>
#include <sstream>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "lib/settings.hpp"
#include "lib/common/mutex.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/data/vec.hpp"

#include "test/helper.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {
    // Namespace of tags to be used for initialising components.
    namespace tags {
        //! @brief Net initialisation tag associating to an output stream for logging.
        struct output;
        //! @brief Node initialisation tag associating to a starting time of execution.
        struct start;
        //! @brief Node initialisation tag associating to a `device_t` unique identifier.
        struct uid;
        //! @brief Node initialisation tag associating to a starting position.
        struct x;
    }
}


//! @cond INTERNAL
namespace details {
    //! @brief Extracts a boolean parameter from a templated type.
    //! @{
    template <typename T>
    struct bool_parameter;
    template <template <bool> class T, bool b>
    struct bool_parameter<T<b>> : public std::integral_constant<bool, b> {};
    //! @}

    //! @brief Extracts information on parameters for test nets.
    //! @{
    template <typename T, typename F, int N = 3>
    struct round_type;
    template <typename T, typename... Os, typename... Is, int N>
    struct round_type<T, std::tuple<Os...>(Is...), N> {
        static constexpr size_t in_size = sizeof...(Is);
        static constexpr size_t out_size = sizeof...(Os);
        using in_type = std::tuple<std::array<Is, N>...>;
        using out_type = std::tuple<std::array<Os, N>...>;
        using full_type = std::tuple<std::array<Is, N>..., std::array<Os, N>...>;
        using fun_type = std::function<std::tuple<Os...>(T&, Is...)>;
    };
    //! @}

    //! @brief If C has an `identifier_tag` expose `node_emplace`, otherwise define it.
    //! @{
    template <typename C, typename = int>
    struct expose_identifier : public C::net {
        using C::net::net;
        template <typename S, typename T>
        device_t node_emplace(common::tagged_tuple<S,T> const& t) {
            while (m_nodes.count(m_next_uid) > 0) ++m_next_uid;
            using tt_type = typename common::tagged_tuple<S,T>::template push_back<component::tags::uid, device_t>;
            tt_type tt(t);
            common::get<component::tags::uid>(tt) = m_next_uid;
            m_nodes.emplace(std::piecewise_construct, std::make_tuple(m_next_uid), std::tuple<typename C::net&, tt_type>(*this, tt));
            return m_next_uid++;
        }
        template <typename T>
        typename C::node& node_at(device_t uid, T&&) {
            return m_nodes.at(uid);
        }
      private:
        std::unordered_map<device_t, typename C::node> m_nodes;
        device_t m_next_uid = 0;
    };
    template <typename C>
    struct expose_identifier<C, std::conditional_t<true,int,typename C::identifier_tag>> : public C::net {
        using C::net::net;
        using C::net::node_emplace;
    };
    //! @}
}
//! @endcond

/**
 * @brief A test network.
 * @param C The combination of components.
 * @param F The signature of the function to be executed in each round.
 * @param N The network size.
 */
template <typename C, typename F = std::tuple<>(), int N = 3>
struct test_net {
    //! @brief The node type.
    using node_type = typename C::node;
    //! @brief The net type.
    using net_type = details::expose_identifier<C>;
    //! @brief The type of input parameters for rounds.
    using in_type = typename details::round_type<node_type, F>::in_type;
    //! @brief The type of output results for rounds.
    using out_type = typename details::round_type<node_type, F>::out_type;
    //! @brief The type of parameters for rounds.
    using round_type = typename details::round_type<node_type, F, N>::full_type;
    //! @brief The type of functions to be executed in each round.
    using fun_type = typename details::round_type<node_type, F>::fun_type;
    //! @brief The type of the network topology description.
    using topo_type = std::vector<std::vector<int>>;

    //! @brief Constructs a test net without round function and with line topology.
    test_net() : test_net(fun_type{}) {}

    //! @brief Constructs a test net without round function and with given topology.
    test_net(topo_type t) : test_net(t, fun_type{}) {}

    //! @brief Constructs a test net given round function and line topology.
    test_net(fun_type f) : test_net(topo_type{}, f) {
        for (int i = 0; i < N; ++i) {
            m_topology.emplace_back();
            if (i > 0)
                m_topology.back().push_back(i-1);
            m_topology.back().push_back(i);
            if (i < N - 1)
                m_topology.back().push_back(i+1);
        }
    }

    //! @brief Constructs a test net given round function and topology.
    test_net(topo_type t, fun_type f) :
            m_count(0), m_topology(t), m_func(f), m_stream(),
            m_network(common::make_tagged_tuple<component::tags::output>(&m_stream)) {
        for (int i = 0; i < N; ++i)
            m_network.node_emplace(common::make_tagged_tuple<component::tags::x,component::tags::start>(make_vec(i * (1.25f - 0.25f*i),0), 0));
        round_start();
    }

    //! @brief Performs a full round of executions given inputs and expected outputs, and returning inputs with actual outputs.
    round_type full_round_expect(round_type r) {
        caller<in_size>(r, std::make_index_sequence<in_size>{}, r, std::make_index_sequence<out_size>{});
        round();
        return r;
    }

    //! @brief Performs a full round of executions given inputs, and returning outputs.
    out_type full_round(in_type x) {
        out_type y;
        caller<0>(x, std::make_index_sequence<in_size>{}, y, std::make_index_sequence<out_size>{});
        round();
        return y;
    }

    //! @brief Accesses a node in the network.
    node_type& d(int id) {
        common::unique_lock<parallel> l;
        return m_network.node_at(id, l);
    }

  private:
    //! @brief The number of inputs to the round function.
    static constexpr size_t in_size = details::round_type<node_type, F>::in_size;
    //! @brief The number of outputs to the round function.
    static constexpr size_t out_size = details::round_type<node_type, F>::out_size;
    //! @brief Whether parallelism is enabled.
    static constexpr bool parallel = details::bool_parameter<decltype(std::declval<node_type>().mutex)>::value;

    //! @brief Helper function calling the round function.
    template <size_t offset, typename I, typename O, size_t... is, size_t... os>
    void caller(I const& x, std::index_sequence<is...>, O& y, std::index_sequence<os...>) {
        for (int i = 0; i < N; ++i) {
            auto z = m_func(d(i), std::get<is>(x)[i]...);
            common::ignore_args((std::get<os+offset>(y)[i] = std::get<os>(z))...);
        }
    }

    //! @brief Performs round starts across the network.
    inline void round_start() {
        for (int i = 0; i < N; ++i) d(i).round_start(m_count);
    }

    //! @brief Performs round ends across the network.
    inline void round_end() {
        for (int i = 0; i < N; ++i) d(i).round_end(m_count);
    }

    //! @brief Ends a round, exchanges messages and starts a new one.
    void round() {
        round_end();
        for (int source = 0; source < N; ++source)
            for (int dest : m_topology[source]) {
                typename node_type::message_t m;
                d(dest).receive(m_count + 0.5f, source, d(source).send(m_count + 0.5f, m));
            }
        ++m_count;
        round_start();
    }

    //! @brief The round number.
    int m_count;
    //! @brief The topology of the network.
    topo_type m_topology;
    //! @brief The function to be executed at rounds.
    fun_type m_func;
    //! @brief A stream absorbing logging output.
    std::stringstream m_stream;
    //! @brief The net object.
    net_type m_network;
};


}

//! @brief Performs a full round on all devices given inputs and expected outputs.
#define EXPECT_ROUND(n, ...) {                          \
    typename decltype(n)::round_type r{__VA_ARGS__};    \
    EXPECT_EQ(n.full_round_expect(r), r);               \
}

#endif // FCPP_TEST_NET_H_
