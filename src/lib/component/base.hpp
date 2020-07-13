// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file base.hpp
 * @brief Implementation of the `base` component and `combine` template for component chaining.
 */

#ifndef FCPP_COMPONENT_BASE_H_
#define FCPP_COMPONENT_BASE_H_

#include <chrono>
#include <limits>

#include "lib/settings.hpp"
#include "lib/common/mutex.hpp"
#include "lib/common/profiler.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration flag associating to whether parallelism is enabled.
    template <bool b>
    struct parallel {};

    //! @brief Node initialisation tag associating to a `device_t` unique identifier.
    struct uid {};

    //! @brief Net initialisation tag associating to a factor to be applied to real time.
    struct realtime {};
}


/**
 * @brief Empty component (base case for component construction).
 *
 * Must be unique and last in a composition of components.
 *
 * <b>Declaration flags:</b>
 * - \ref tags::parallel defines whether parallelism is enabled (defaults to \ref FCPP_PARALLEL).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::uid associates to a `device_t` unique identifier (required).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::realtime associates to a `double` factor to be applied to real time (defaults to `FCPP_REALTIME`).
 */
template <class... Ts>
struct base {
    //! @brief Whether parallelism is enabled.
    constexpr static bool parallel = common::option_flag<tags::parallel, FCPP_PARALLEL, Ts...>;

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

            #define MISSING_TAG_MESSAGE "\033[1m\033[4mmissing required tags::uid node initialisation tag\033[0m"
            
            //! @name constructors
            //@{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : uid(common::get_or<tags::uid>(t, 0)), net(n) {
                static_assert(common::tagged_tuple<S,T>::tags::template count<tags::uid> >= 1, MISSING_TAG_MESSAGE);
            }

            #undef MISSING_TAG_MESSAGE

            //! @brief Deleted copy constructor.
            node(const node&) = delete;
            
            //! @brief Deleted copy assignment.
            node& operator=(const node&) = delete;
            //@}
            
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
            void receive(times_t, device_t, const common::tagged_tuple<S,T>&) {}
            
            //! @brief Produces a message to send to a target, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t, device_t, common::tagged_tuple<S,T>& t) const {
                return t;
            }
            
            //! @brief The unique identifier of the device.
            const device_t uid;
            
            //! @brief A mutex for regulating access to the node.
            common::mutex<parallel> mutex;

            //! @brief A reference to the corresponding net object.
            typename F::net& net;

          protected: // visible by node objects only
            //! @brief Gives access to the node as instance of `F::node`. Should NEVER be overridden.
            typename F::node& as_final() {
                return *static_cast<typename F::node*>(this);
            }
            
            //! @brief Gives const access to the node as instance of `F::node`. Should NEVER be overridden.
            const typename F::node& as_final() const {
                return *static_cast<const typename F::node*>(this);
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
            //@{
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) {
                m_realtime_start = std::chrono::high_resolution_clock::now();
                m_realtime_factor = common::get_or<tags::realtime>(t, FCPP_REALTIME);
            }
            
            //! @brief Deleted copy constructor.
            net(const net&) = delete;
            
            //! @brief Deleted copy assignment.
            net& operator=(const net&) = delete;
            //@}

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
            
            //! @brief Runs the events at real time pace until a given end. Should NEVER be overridden.
            void run(times_t end = TIME_MAX) {
                while (as_final().next() < end)
                    if (as_final().next() <= real_time())
                        as_final().update();
                PROFILE_REPORT();
            }
            
          protected: // visible by net objects only
            //! @brief Gives access to the net as instance of `F::net`. Should NEVER be overridden.
            typename F::net& as_final() {
                return *static_cast<typename F::net*>(this);
            }
            
            //! @brief Gives const access to the net as instance of `F::net`. Should NEVER be overridden.
            const typename F::net& as_final() const {
                return *static_cast<const typename F::net*>(this);
            }
            
            //! @brief An estimate of real time elapsed from start. Should NEVER be overridden.
            times_t real_time() const {
                if (m_realtime_factor == std::numeric_limits<double>::infinity())
                    return TIME_MAX;
                return (std::chrono::high_resolution_clock::now() - m_realtime_start).count() * m_realtime_factor;
            }
            
          private: // implementation details
            //! @brief The start time of the program.
            std::chrono::high_resolution_clock::time_point m_realtime_start;
            
            //! @brief A factor warping progression of real time.
            double m_realtime_factor;
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
