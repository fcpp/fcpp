// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file identifier.hpp
 * @brief Implementation of the `identifier` component handling node creation and indexing.
 */

#ifndef FCPP_COMPONENT_IDENTIFIER_H_
#define FCPP_COMPONENT_IDENTIFIER_H_

#include <map>
#include <queue>
#include <type_traits>

#include "lib/common/algorithm.hpp"
#include "lib/common/random_access_map.hpp"
#include "lib/component/base.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


//! @cond INTERNAL
namespace details {
    /**
     * @brief Priority queue of pairs `(times_t, device_t)` designed for popping bunches of elements at a time.
     *
     * @param synchronised Whether lots of collisions (same time) are to be expected or not.
     */
    template <bool synchronised>
    class times_queue;

    //! @brief Specialisation for lots of collisions, as map of vectors.
    template <>
    class times_queue<true> {
      public:
        //! @brief Default constructor.
        times_queue() {
            m_queue.emplace(TIME_MAX, std::vector<device_t>());
        }

        //! @brief The smallest time in the queue.
        inline times_t next() const {
            return m_queue.begin()->first;
        }

        //! @brief Adds a new pair to the queue.
        inline void push(times_t t, device_t uid) {
            m_queue[t].push_back(uid);
        }

        //! @brief Pops elements with the smallest time if up to `t`.
        inline std::vector<device_t> pop(times_t t) {
            if (next() > t) return {};
            std::vector<device_t> v = std::move(m_queue.begin()->second);
            m_queue.erase(m_queue.begin());
            return v;
        }

      private:
        //! @brief The actual priority queue.
        std::map<times_t, std::vector<device_t>> m_queue;
    };

    //! @brief Specialisation for few collisions, as priority queue.
    template <>
    class times_queue<false> {
      public:
        //! @brief Default constructor.
        times_queue() {
            m_queue.emplace(TIME_MAX, device_t());
        }

        //! @brief The smallest time in the queue.
        inline times_t next() const {
            return m_queue.top().first;
        }

        //! @brief Adds a new pair to the queue.
        inline void push(times_t t, device_t uid) {
            m_queue.emplace(t, uid);
        }

        //! @brief Pops elements with times up to `t`.
        std::vector<device_t> pop(times_t t) {
            std::vector<device_t> v;
            while (next() <= t) {
                v.push_back(m_queue.top().second);
                m_queue.pop();
            }
            return v;
        }

      private:
        //! @brief The type of queue elements.
        using type = std::pair<times_t, device_t>;
        //! @brief The actual priority queue.
        std::priority_queue<type, std::vector<type>, std::greater<type>> m_queue;
    };
}
//! @endcond


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration flag associating to whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
    template <bool b>
    struct parallel;

    //! @brief Declaration flag associating to whether many events are expected to happen at the same time (defaults to \ref FCPP_SYNCHRONISED).
    template <bool b>
    struct synchronised {};

    //! @brief Node initialisation tag associating to a `device_t` unique identifier (required).
    struct uid;

    //! @brief Initialisation tag associating to the time sensitivity, allowing indeterminacy below it (defaults to \ref FCPP_TIME_EPSILON).
    struct epsilon {};

    //! @brief Net initialisation tag associating to the number of threads that can be created (defaults to \ref FCPP_THREADS).
    struct threads {};
}


/**
 * @brief Component handling node creation and indexing.
 *
 * The \ref timer component cannot be a parent of a \ref identifier to preserve node scheduling.
 *
 * <b>Declaration flags:</b>
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 * - \ref tags::synchronised defines whether many events are expected to happen at the same time (defaults to \ref FCPP_SYNCHRONISED).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::epsilon associates to the time sensitivity, allowing indeterminacy below it (defaults to \ref FCPP_TIME_EPSILON).
 * - \ref tags::threads associates to the number of threads that can be created (defaults to \ref FCPP_THREADS).
 *
 * Whenever \ref tags::parallel is false, \ref tags::threads is ignored and \ref tags::epsilon has only a minor effect (it is recommended to set it to zero).
 */
template <class... Ts>
struct identifier {
    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief Whether new values are pushed to aggregators or pulled when needed.
    constexpr static bool synchronised = common::option_flag<tags::synchronised, FCPP_SYNCHRONISED, Ts...>;

    /**
     * @brief The actual component.
     *
     * Component functionalities are added to those of the parent by inheritance at multiple levels: the whole component class inherits tag for static checks of correct composition, while `node` and `net` sub-classes inherit actual behaviour.
     * Further parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     * @param P The parent component to inherit from.
     */
    template <typename F, typename P>
    struct component : public P {
        //! @cond INTERNAL
        DECLARE_COMPONENT(identifier);
        AVOID_COMPONENT(identifier,timer);
        //! @endcond

        //! @brief The local part of the component.
        using node = typename P::node;

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief The type of nodes.
            using node_type = typename F::node;

            //! @brief The map type used internally for storing nodes.
            using map_type = common::random_access_map<device_t, node_type>;

            //! @brief The type of node locks.
            struct lock_type : public common::unique_lock<parallel> {
                //! @brief Default constructor not owning a lock.
                lock_type() noexcept : common::unique_lock<parallel>(), m_node(nullptr), m_next() {}
                //! @brief Move constructor.
                lock_type(lock_type&& o) = default;
                //! @brief Locking constructor, storing event information.
                lock_type(common::mutex<parallel>& m, node_type& n) noexcept : common::unique_lock<parallel>(m), m_node(&n), m_next(n.next()) {}
                //! @brief Destructor updating event queue if needed.
                ~lock_type() {
                    if (m_node->next() != m_next)
                        m_node->net.push_event(m_node->uid, m_node->next());
                }
                //! @brief Move assignment operator.
                lock_type& operator=(lock_type&& o) = default;
              private:
                //! @brief A reference to the node being locked.
                node_type* m_node;
                //! @brief The next event at the time of locking.
                times_t m_next;
            };

            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) : P::net(t), m_next_uid(0), m_epsilon(common::get_or<tags::epsilon>(t, FCPP_TIME_EPSILON)), m_threads(common::get_or<tags::threads>(t, FCPP_THREADS)) {}

            /**
             * @brief Returns next event to schedule for the net component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return std::min(m_queue.next(), P::net::next());
            }

            //! @brief Updates the internal status of net component.
            void update() {
                if (m_queue.next() < P::net::next()) {
                    times_t t = m_queue.next();
                    std::vector<device_t> nv = m_queue.pop(t + m_epsilon), dv;
                    assert(m_queue.next() > t);
                    common::mutex<parallel> m;
                    common::parallel_for(common::tags::general_execution<parallel>(m_threads), nv.size(), [&](size_t i, size_t){
                        if (m_nodes.count(nv[i]) > 0) {
                            node_type& n = m_nodes.at(nv[i]);
                            times_t nxt;
                            {
                                common::lock_guard<parallel> device_lock(n.mutex);
                                if (n.next() > t + m_epsilon) return;
                                n.update();
                                nxt = n.next();
                            }
                            if (nxt == TIME_MAX) {
                                common::lock_guard<parallel> l(m);
                                dv.push_back(n.uid);
                            } else if (nxt > t) push_event(n.uid, nxt);
                        }
                    });
                    for (device_t uid : dv) node_erase(uid);
                    assert(m_queue.next() > t);
                } else P::net::update();
            }

            //! @brief Pushes a new event into the queue.
            void push_event(device_t uid, times_t t) {
                common::lock_guard<parallel> l(m_queue_mutex);
                m_queue.push(t, uid);
            }

            //! @brief Returns the total number of nodes.
            inline size_t node_size() const {
                return m_nodes.size();
            }

            //! @brief Returns whether a node with a certain device identifier exists.
            inline size_t node_count(device_t uid) const {
                return m_nodes.count(uid);
            }

            //! @brief Const access to the node with a given device device identifier.
            inline node_type const& node_at(device_t uid) const {
                return m_nodes.at(uid);
            }

            //! @brief Access to the node with a given device device identifier (given a lock for the node's mutex).
            node_type& node_at(device_t uid, lock_type& l) {
                node_type& n = m_nodes.at(uid);
                l = lock_type(m_nodes.at(uid).mutex, n);
                return n;
            }

            //! @brief Constructs an empty node lock, to be used with \ref node_at.
            inline lock_type node_lock() {
                return {};
            }

          protected: // visible by net objects only
            //! @brief Random-access iterator to the first node (in a random order).
            typename map_type::const_iterator node_begin() const {
                return m_nodes.begin();
            }

            //! @brief Random-access const iterator to the first node (in a random order).
            typename map_type::iterator node_begin() {
                return m_nodes.begin();
            }

            //! @brief Random-access iterator to the last node (in a random order).
            typename map_type::const_iterator node_end() const {
                return m_nodes.end();
            }

            //! @brief Random-access const iterator to the last node (in a random order).
            typename map_type::iterator node_end() {
                return m_nodes.end();
            }

            //! @brief Creates a new node, initialising it with data in `t` (returns the identifier assigned).
            template <typename S, typename T>
            device_t node_emplace(common::tagged_tuple<S,T> const& t) {
                auto tt = push_uid(t, typename S::template intersect<tags::uid>());
                device_t const& id = common::get<tags::uid>(tt);
                m_nodes.emplace(std::piecewise_construct, std::make_tuple(id), std::tuple<typename F::net&, decltype(tt)>(P::net::as_final(), tt));
                push_event(id, m_nodes.at(id).next());
                return id;
            }

            //! @brief Erases the node with a given identifier.
            inline size_t node_erase(device_t uid) {
                return m_nodes.erase(uid);
            }

            //! @brief Erases all nodes.
            inline void node_clear() {
                m_nodes.clear();
            }

          private: // implementation details
            //! @brief Returns the next device UID to be created (without request).
            template <typename T>
            inline auto push_uid(T const& t, common::type_sequence<>) {
                while (m_nodes.count(m_next_uid) > 0) ++m_next_uid;
                using tt_type = typename T::template push_back<tags::uid, device_t>;
                tt_type tt(t);
                common::get<tags::uid>(tt) = m_next_uid++;
                return tt;
            }

            //! @brief Returns the next device UID to be created (with request).
            template <typename T>
            inline T const& push_uid(T const& t, common::type_sequence<tags::uid>) {
                assert(m_nodes.count(common::get<tags::uid>(t)) == 0);
                return t;
            }

            //! @brief The set of nodes, indexed by identifier.
            map_type m_nodes;

            //! @brief The queue of identifiers by next event.
            details::times_queue<synchronised> m_queue;

            //! @brief The next free identifier.
            device_t m_next_uid;

            //! @brief The time sensitivity.
            times_t const m_epsilon;

            //! @brief The number of threads to be used.
            size_t const m_threads;

            //! @brief Mutex for accessing the queue of events.
            common::mutex<parallel> m_queue_mutex;
        };
    };
};


}


}

#endif // FCPP_COMPONENT_IDENTIFIER_H_
