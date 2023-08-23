// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file base.hpp
 * @brief Implementation of the `base` component and `combine` template for component chaining.
 */

#ifndef FCPP_COMPONENT_BASE_H_
#define FCPP_COMPONENT_BASE_H_

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>

#include "lib/settings.hpp"
#include "lib/common/mutex.hpp"
#include "lib/common/profiler.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration flag associating to whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
    template <bool b>
    struct parallel {};

    //! @brief Declaration flag associating to whether running should follow real time (defaults to `FCPP_REALTIME < INF`).
    template <bool b>
    struct realtime {};

    //! @brief Declaration tag associating to a clock type (defaults to `std::chrono::high_resolution_clock`).
    template <typename T>
    struct clock_type {};

    //! @brief Node initialisation tag associating to a `device_t` unique identifier (required).
    struct uid {};
}


//! @brief Defines a predicate `has_name<T>` checking whether a given component is present in `T`.
#define CHECK_COMPONENT(name)                       \
        template <typename T, typename = void>      \
        struct has_##name : std::false_type {};     \
        template <typename T>                       \
        struct has_##name<T, std::conditional_t<true,void,typename T::name##_tag>> : std::true_type {}

//! @brief Declares a component, checking that there are no duplicates among parents.
#define DECLARE_COMPONENT(name)                     \
        struct name##_tag {};                       \
        CHECK_COMPONENT(name);                      \
        static_assert(not has_##name<P>::value, "cannot combine multiple " #name " components")

//! @brief Requires that a given component is present among parents.
#define REQUIRE_COMPONENT(name,parent)              \
        CHECK_COMPONENT(parent);                    \
        static_assert(has_##parent<P>::value, "missing " #parent " parent for " #name " component")

//! @brief Requires that a given component is present among parents, if a condition is met.
#define REQUIRE_COMPONENT_IF(name,parent,cond)      \
        CHECK_COMPONENT(parent);                    \
        static_assert(not cond or has_##parent<P>::value, "missing " #parent " parent for " #name " component with " #cond)

//! @brief Requires that a given component is absent among parents.
#define AVOID_COMPONENT(name,parent)                \
        CHECK_COMPONENT(parent);                    \
        static_assert(not has_##parent<P>::value, #parent " cannot be parent of " #name " component")


/**
 * @brief Empty component (base case for component construction).
 *
 * Must be last in a composition of components.
 *
 * <b>Declaration flags:</b>
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 * - \ref tags::realtime defines whether running should follow real time (defaults to `FCPP_REALTIME < INF`).
 * - \ref tags::clock_type defines a clock type (defaults to `std::chrono::high_resolution_clock`)
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::uid associates to a `device_t` unique identifier (required).
 */
template <class... Ts>
struct base {
    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

    //! @brief Whether running should follow real time.
    constexpr static bool realtime = common::option_flag<tags::realtime, FCPP_REALTIME < INF, Ts...>;

    //! @brief The clock type used for time measurements.
    using clock_t = common::option_type<tags::clock_type, std::chrono::high_resolution_clock, Ts ...>;

    /**
     * @brief The actual component.
     *
     * Component functionalities are added to those of the parent by inheritance at multiple levels: the whole component class inherits tag for static checks of correct composition, while `node` and `net` sub-classes inherit actual behaviour.
     * Further parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     */
    template <typename F>
    struct component {
        //! @brief The local part of the component.
        class node {
          public: // visible by net objects and the main program
            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = common::tagged_tuple_t<>;

            //! @brief The mutex type.
            using mutex_type = common::mutex<parallel>;

            //! @cond INTERNAL
            #define MISSING_TAG_MESSAGE "\033[1m\033[4mmissing required tags::uid node initialisation tag\033[0m"
            //! @endcond

            //! @name constructors
            //! @{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : uid(common::get_or<tags::uid>(t, 0)), net(n) {
                static_assert(common::tagged_tuple<S,T>::tags::template count<tags::uid> >= 1, MISSING_TAG_MESSAGE);
            }

            #undef MISSING_TAG_MESSAGE

            //! @brief Deleted copy constructor.
            node(node const&) = delete;

            //! @brief Deleted copy assignment.
            node& operator=(node const&) = delete;
            //! @}

            /**
             * @brief Returns next event to schedule for the node component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return TIME_MAX; // no event to schedule
            }

            //! @brief Updates the internal status of node component.
            void update() {}

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t) {}

            //! @brief Performs computations at round middle with current time `t`.
            void round_main(times_t) {}

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t) {}

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t, device_t, common::tagged_tuple<S,T> const&) {}

            //! @brief Produces the message to send, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t, common::tagged_tuple<S,T>& t) const {
                return t;
            }

            //! @brief The unique identifier of the device.
            device_t const uid;

            //! @brief A mutex for regulating access to the node.
            mutex_type mutex;

            //! @brief A reference to the corresponding net object.
            typename F::net& net;

          protected: // visible by node objects only
            //! @brief Gives access to the node as instance of `F::node`. Should NEVER be overridden.
            typename F::node& as_final() {
                return *static_cast<typename F::node*>(this);
            }

            //! @brief Gives const access to the node as instance of `F::node`. Should NEVER be overridden.
            typename F::node const& as_final() const {
                return *static_cast<typename F::node const*>(this);
            }

            //! @brief Performs a computation round with current time `t`. Should NEVER be overridden.
            void round(times_t t) {
                PROFILE_COUNT("round");
                {
                    PROFILE_COUNT("round/start");
                    as_final().round_start(t);
                }
                {
                    PROFILE_COUNT("round/main");
                    as_final().round_main(t);
                }
                {
                    PROFILE_COUNT("round/end");
                    as_final().round_end(t);
                }
            }
        };

        //! @brief The global part of the component.
        class net {
          public: // visible by node objects and the main program
            //! @name constructors
            //! @{

            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const&) {
                m_realtime_start = clock_t::now();
                m_realtime_factor = real_t(clock_t::period::num) / clock_t::period::den;
                m_last_update = m_next_update = 0;
                m_warn_delay = FCPP_TIME_EPSILON;
            }

            //! @brief Deleted copy constructor.
            net(net const&) = delete;

            //! @brief Deleted copy assignment.
            net& operator=(net const&) = delete;

            //! @}

            /**
             * @brief Returns next event to schedule for the net component.
             *
             * Should correspond to the next time also during updates.
             */
            times_t next() const {
                return TIME_MAX; // no event to schedule
            }

            //! @brief Updates the internal status of net component.
            void update() {}

            //! @brief Runs the events until a given end. Should NEVER be overridden.
            void run(times_t end = TIME_MAX) {
                times_t nxt;
                while ((nxt = as_final().next()) < end) {
                    m_next_update = nxt;
                    maybe_sleep(nxt, std::integral_constant<bool, realtime>{});
                    m_last_update = nxt;
                    as_final().update();
                }
                PROFILE_REPORT();
            }

            //! @brief A measure of the internal time clock.
            times_t internal_time() const {
                if (not realtime or (m_last_update == m_next_update)) return m_last_update;
                return std::max(std::min(real_time(), m_next_update), m_last_update);
            }

            //! @brief An estimate of real time elapsed from start. Should NEVER be overridden.
            inline times_t real_time() const {
                return (clock_t::now() - m_realtime_start).count() * m_realtime_factor;
            }

          protected: // visible by net objects only
            //! @brief Gives access to the net as instance of `F::net`. Should NEVER be overridden.
            typename F::net& as_final() {
                return *static_cast<typename F::net*>(this);
            }

            //! @brief Gives const access to the net as instance of `F::net`. Should NEVER be overridden.
            typename F::net const& as_final() const {
                return *static_cast<typename F::net const*>(this);
            }

          private: // implementation details
            //! @brief Does not wait before an update.
            inline void maybe_sleep(times_t, std::false_type) {}

            //! @brief Waits real time before an update.
            inline void maybe_sleep(times_t nxt, std::true_type) {
                typename clock_t::time_point t = m_realtime_start + typename clock_t::duration((long long)(nxt/m_realtime_factor));
                if (t > clock_t::now())
                    std::this_thread::sleep_until(t);
                else {
                    times_t delay = (clock_t::now() - t).count() * m_realtime_factor;
                    if (delay >= m_warn_delay) {
                        std::cerr << "WARNING: execution delayed (delay = " << delay << "s)" << std::endl;
                        m_warn_delay = 2*delay;
                    }
                }
            }

            //! @brief The start time of the program.
            typename clock_t::time_point m_realtime_start;

            //! @brief A conversion factor from clock ticks to real time in seconds.
            real_t m_realtime_factor;

            //! @brief The internal time of the last and next update.
            times_t m_last_update, m_next_update;

            //! @brief The minimum delay triggering a warning.
            times_t m_warn_delay;
        };
    };
};


//! @cond INTERNAL
namespace details {
    //! @brief Combines components `Ts` given the final component type `F` (the last component must be a `base`).
    template <typename F, typename... Ts>
    struct combine_spec;

    //! @brief Inductive case when some components are given.
    template <typename F, typename T, typename... Ts>
    struct combine_spec<F, T, Ts...> : public T::template component<F, combine_spec<F, Ts...>> {};

    //! @brief Base case when only the base component is given.
    template <typename F, typename T>
    struct combine_spec<F, T> : public T::template component<F> {};

    template <typename F, template <class...> class... Cs>
    struct combine;

    template <template <class...> class F, typename... Ts>
    struct combine<F<Ts...>> : public base<Ts...>::template component<F<Ts...>> {};

    template <template <class...> class F, typename... Ts, template <class...> class C, template <class...> class... Cs>
    struct combine<F<Ts...>, C, Cs...> : public C<Ts...>::template component<F<Ts...>, combine<F<Ts...>, Cs...>> {};
}
//! @endcond


/**
 * @brief Combines components (each instantiated with arguments) into a single object.
 *
 * @param Ts Instantiated components to chain together (the last must be a `base`).
 */
template <typename... Ts>
struct combine_spec : public details::combine_spec<combine_spec<Ts...>, Ts...> {};


/**
 * @brief Combines components into a single templated object.
 *
 * @param Ts Template components to chain together (`base` is implied as last).
 */
template <template<class...> class... Cs>
struct combine {
    //! @brief Instantiates the combination for given arguments.
    template<typename... Ts>
    struct component : public details::combine<component<Ts...>, Cs...> {};
};


/**
 * @brief Declares a name to be a specific sequence of options (declaration tags and flags).
 *
 * Example of intended usage:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * namespace component {
 *   DECLARE_COMBINE(mycombo, calculus, exporter, storage, ...);
 *   DECLARE_OPTIONS(myopt, tags::program<myprogram>, tags::dimension<2>, ...);
 *   mycombo<myopt>::net network;
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @param name The name of the option sequence.
 * @param ... The sequence of options.
 */
#define DECLARE_OPTIONS(name, ...) struct name : public fcpp::common::type_sequence<__VA_ARGS__> {}


/**
 * @brief Declares a name to be a specific component composition.
 *
 * Example of intended usage:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * namespace component {
 *   DECLARE_COMBINE(mycombo, calculus, exporter, storage, ...);
 *   DECLARE_OPTIONS(myopt, tags::program<myprogram>, tags::dimension<2>, ...);
 *   mycombo<myopt>::net network;
 * }
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @param name The name of the composition.
 * @param ... Template components to chain together.
 */
#define DECLARE_COMBINE(name, ...) template <class... Ts> struct name : public fcpp::component::details::combine<name<Ts...>, __VA_ARGS__> {}


}


}

#endif // FCPP_COMPONENT_BASE_H_
