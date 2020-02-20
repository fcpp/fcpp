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
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Tag associating to the unique identifier of an object.
    struct uid {};

    //! @brief Tag associating to a factor to be applied to real time.
    struct realtime {};
}


/**
 * @brief Empty component (base case for component construction).
 *
 * Initialises `node` with tag `id` associating to a `device_t` node identifier (required).
 * Initialises `net` with tag `realtime` associating to a `double` factor to be applied to real time (defaults to `FCPP_REALTIME`).
 */
struct base {
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
            //! @name constructors
            //@{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : uid(common::get<tags::uid>(t)), net(n) {}

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
            
            //! @brief The unique identifier of the device.
            const device_t uid;
            
            //! @brief A mutex for regulating access to the node.
            common::mutex<FCPP_PARALLEL> mutex;

          protected: // visible by node objects only
            //! @brief A reference to the corresponding net object.
            typename F::net& net;
            
            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = common::tagged_tuple_t<>;
            
            //! @brief Gives access to the node as instance of `F::node`. Should NEVER be overridden.
            typename F::node& as_final() {
                return *static_cast<typename F::node*>(this);
            }
            
            //! @brief Gives const access to the node as instance of `F::node`. Should NEVER be overridden.
            const typename F::node& as_final() const {
                return *static_cast<const typename F::node*>(this);
            }
            
            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t, device_t, const common::tagged_tuple<S,T>&) {}
            
            //! @brief Produces a message to send to a target, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t, device_t, common::tagged_tuple<S,T>& t) const {
                return t;
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t) {}
            
            //! @brief Performs computations at round middle with current time `t`.
            void round_main(times_t) {}

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t) {}
            
            //! @brief Performs a computation round with current time `t`. Should NEVER be overridden.
            void round(times_t t) {
                as_final().round_start(t);
                as_final().round_main(t);
                as_final().round_end(t);
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
            
            //! @brief Runs the events at real time pace. Should NEVER be overridden.
            void run() {
                while (as_final().next() < TIME_MAX)
                    if (as_final().next() <= real_time())
                        as_final().update();
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
    //! @brief Combines components `Ts` given the final component type `F`.
    template <typename F, typename... Ts>
    struct combine;

    //! @brief Inductive case when some components are given.
    template <typename F, typename T, typename... Ts>
    struct combine<F, T, Ts...> : public T::template component<F, combine<F, Ts...>> {};

    //! @brief Base case when no components are given.
    template <typename F>
    struct combine<F> : public base::template component<F> {};
}
//! @endcond


/**
 * @brief Combines components into a single object.
 *
 * @param Ts Components to chain together.
 */
template <typename... Ts>
class combine : public details::combine<combine<Ts...>, Ts...> {};


}


}

#endif // FCPP_COMPONENT_BASE_H_
