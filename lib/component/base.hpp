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
#include "lib/common/tagged_tuple.hpp"


//! @brief Namespace of tags to be used for `tagged_tuple` objects.
namespace tags {
    //! @brief Tag for setting a factor to be applied to real time (defaults to `FCPP_REALTIME`).
    struct realtime {};
}


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Empty component (base case for component construction).
struct base {
    /**
     * @brief The actual component.
     * Parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     */
    template <typename F>
    struct component {
        //! @brief The local part of the component.
        struct node {
            // The public interface gathers what may be called from net objects and main programs.
            
            //! @name constructors
            //@{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const tagged_tuple<S,T>&) : net(n) {}

            //! @brief Deleted copy constructor.
            node(const node&) = delete;
            
            //! @brief Move constructor.
            node(node&&) = default;
            //@}
            
            //! @name assignment operators
            //@{
            //! @brief Deleted copy assignment.
            node& operator=(const node&) = delete;
            
            //! @brief Move assignment.
            node& operator=(node&&) = default;
            //@}
            
            //! @brief Returns next event to schedule for the node component.
            times_t next() {
                return std::numeric_limits<times_t>::max(); // no event to schedule
            }
            
            //! @brief Updates the internal status of node component.
            void update() {}
            
              // The protected interface gathers what may be called from node objects only.
          protected:
            //! @brief A reference to the corresponding net object.
            typename F::net& net;
            
            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = tagged_tuple_t<>;
            
            //! @brief Gives access to the current node as instance of `F::node`.
            typename F::node& as_final() {
                return *static_cast<typename F::node*>(this);
            }
            
            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t, device_t, const tagged_tuple<S,T>&) {}
            
            //! @brief Produces a message to send to a target, storing it in its argument and returning it.
            template <typename S, typename T>
            tagged_tuple<S,T>& send(times_t, device_t, tagged_tuple<S,T>& t) {
                return t;
            }

            //! @brief Performs actions at round start.
            void round_start() {}
            
            //! @brief Performs the main round computation.
            void round_main() {}

            //! @brief Performs actions at round end.
            void round_end() {}
        };
        
        //! @brief The global part of the component.
        class net {
            //! @brief The start time of the program.
            std::chrono::high_resolution_clock::time_point m_realtime_start;
            
            //! @brief The factor at which real time progresses.
            double m_realtime_factor;
            
              // The public interface gathers what may be called from node objects and main programs.
          public:
            //! @name constructors
            //@{
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const tagged_tuple<S,T>& t) {
                m_realtime_start = std::chrono::high_resolution_clock::now();
                m_realtime_factor = get_or<tags::realtime>(t, FCPP_REALTIME);
            }
            
            //! @brief Deleted copy constructor.
            net(const net&) = delete;
            
            //! @brief Move constructor.
            net(net&&) = default;
            //@}
            
            //! @name assignment operators
            //@{
            //! @brief Deleted copy assignment.
            net& operator=(const net&) = delete;
            
            //! @brief Move assignment.
            net& operator=(net&&) = default;
            //@}

            //! @brief Returns next event to schedule for the network component.
            times_t next() {
                return std::numeric_limits<times_t>::max(); // no event to schedule
            }
            
            //! @brief Updates the internal status of network component.
            void update() {}
            
            //! @brief Runs the events, keeping the pace with real time as much as possible.
            void run() {
                while (as_final().next() < std::numeric_limits<times_t>::max())
                    if (as_final().next() <= real_time())
                        as_final().update();
            }
            
              // The protected interface gathers what may be called from node objects only.
          protected:
            //! @brief Gives access to the net as instance of `F::net`.
            typename F::net& as_final() {
                return *static_cast<typename F::net*>(this);
            }
            
            //! @brief An estimate of the real time elapsed from start on this device.
            times_t real_time() {
                if (m_realtime_factor == std::numeric_limits<double>::infinity())
                    return std::numeric_limits<times_t>::max();
                return (std::chrono::high_resolution_clock::now() - m_realtime_start).count() * m_realtime_factor;
            }
        };
    };
};


//! @cond INTERNAL
namespace details {
    // Combines components `Ts` given the final component type `F`.
    template <typename F, typename... Ts>
    struct combine;

    // Inductive case when some components are given.
    template <typename F, typename T, typename... Ts>
    struct combine<F, T, Ts...> : public T::template component<F, combine<F, Ts...>> {};

    // Base case when no components are given.
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

#endif // FCPP_COMPONENT_BASE_H_
