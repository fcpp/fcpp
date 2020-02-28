// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file physics_position.hpp
 * @brief Implementation of the `physics_position` component handling physical evolution of a position through time.
 */

#ifndef FCPP_SIMULATION_PHYSICS_POSITION_H_
#define FCPP_SIMULATION_PHYSICS_POSITION_H_

#include <cmath>

#include <array>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/array.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/data/field.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Tag associating to a starting position.
    struct x {};

    //! @brief Tag associating to a starting velocity.
    struct v {};

    //! @brief Tag associating to a starting acceleration.
    struct a {};

    //! @brief Tag associating to a starting friction coefficient.
    struct f {};
}


/**
 * @brief Component handling physical evolution of a position through time.
 *
 * Initialises `node` with tags `x`, `v`, `a` associating to a starting `std::array<double,n>` position, velocity and acceleration (`x` is required, `v` and `a` default to the null vector), and with tag `f` associating to a `double` friction coefficient (defaults to zero). Position \f$ x \f$ evolves as per the differential equation \f$ x'' = a - f x' \f$ of uniformily accelerated viscous motion.
 * Must be unique in a composition of components.
 *
 * @param n Dimensionality of the space (defaults to 2).
 */
template <size_t n = 2>
struct physics_position {
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
        //! @brief Marks that a position component is present.
        struct position_tag {};

        //! @brief The dimensionality of the space.
        const size_t dimension = n;

        //! @brief Checks if T has a `position_tag`.
        template <typename T, typename = int>
        struct has_ptag : std::false_type {};
        template <typename T>
        struct has_ptag<T, std::conditional_t<true,int,typename T::position_tag>> : std::true_type {};
        
        //! @brief Asserts that P has no `position_tag`.
        static_assert(not has_ptag<P>::value, "cannot combine multiple position components");

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            //@{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& nt, const common::tagged_tuple<S,T>& t) : P::node(nt,t), m_x(common::get<tags::x>(t)), m_v(common::get_or<tags::v>(t, std::array<double, n>{})), m_a(common::get_or<tags::a>(t, std::array<double, n>{})), m_f(common::get_or<tags::f>(t, 0.0)) {
                m_last = TIME_MIN;
            }

            //! @brief Position now (const access).
            const std::array<double, n>& position() const {
                return m_x;
            }

            //! @brief Position at a given time.
            std::array<double, n> position(times_t t) const {
                double dt = t - m_last;
                if (dt == 0) {
                    return m_x;
                }
                if (m_f == 0) {
                    return m_x + m_v * dt + m_a * (dt*dt/2);
                }
                if (m_f == std::numeric_limits<double>::infinity()) {
                    return m_x;
                }
                double k = (1 - exp(-m_f * dt)) / m_f;
                return m_x + m_v * k + m_a * ((dt-k)/m_f);
            }
            
            //! @brief Velocity now.
            std::array<double, n>& velocity() {
                return m_v;
            }
            
            //! @brief Velocity now (const access).
            const std::array<double, n>& velocity() const {
                return m_v;
            }
            
            //! @brief Velocity at a given time.
            std::array<double, n> velocity(times_t t) const {
                double dt = t - m_last;
                if (dt == 0) {
                    return m_v;
                }
                if (m_f == 0) {
                    return m_v + m_a * dt;
                }
                if (m_f == std::numeric_limits<double>::infinity()) {
                    return {};
                }
                double k1 = exp(-m_f * dt); // derivative of k
                return m_v * k1 + m_a * ((1-k1)/m_f);
            }
            
            //! @brief Personal acceleration.
            std::array<double, n>& propulsion() {
                return m_a;
            }
            
            //! @brief Personal acceleration (const access).
            const std::array<double, n>& propulsion() const {
                return m_a;
            }

            //! @brief Total acceleration now.
            std::array<double, n> acceleration() const {
                if (m_f == 0) {
                    return m_a;
                }
                if (m_f == std::numeric_limits<double>::infinity()) {
                    return {};
                }
                return m_a - m_f * m_v;
            }
            
            //! @brief Total acceleration at a given time.
            std::array<double, n> acceleration(times_t t) const {
                if (m_f == 0) {
                    return m_a;
                }
                if (m_f == std::numeric_limits<double>::infinity()) {
                    return 0;
                }
                double dt = t - m_last;
                double k1 = exp(-m_f * dt);
                return m_a * k1 - m_v * (m_f * k1);
            }
            
            //! @brief Friction coefficient.
            double& friction() {
                return m_f;
            }

            //! @brief Friction coefficient (const access).
            double friction() const {
                return m_f;
            }
            
            //! @brief First time before `t` when a value `y` will be reached on a certain coordinate `i`.
            times_t reach_time(size_t i, double y, times_t t) {
                y -= m_x[i];
                if (y == 0) return m_last;
                if (m_f == 0) {
                    if (m_a[i] == 0) {
                        double res = y / m_v[i];
                        return res < 0 ? TIME_MAX : m_last + res;
                    }
                    double delta = m_v[i] * m_v[i] + 2 * y * m_a[i];
                    if (delta < 0) return TIME_MAX;
                    delta = sqrt(delta);
                    double res1 = (-m_v[i] - delta) / m_a[i];
                    double res2 = (-m_v[i] + delta) / m_a[i];
                    if (res2 < res1) std::swap(res1, res2);
                    if (res1 > 0) return m_last + res1;
                    if (res2 > 0) return m_last + res2;
                    return TIME_MAX;
                }
                if (m_f == std::numeric_limits<double>::infinity()) return TIME_MAX;
                if (m_a[i] == m_v[i] * m_f) { // limit velocity
                    double res = y / m_v[i];
                    return res < 0 ? TIME_MAX : m_last + res;
                }
                if (m_a[i] == 0) {
                    double res = 1 - y * m_f / m_v[i];
                    return res < 0 or res > 1 ? TIME_MAX : m_last - log(res) / m_f;
                }
                if (m_v[i]*m_a[i] < 0) {
                    double inv = log(1 - m_v[i] / m_a[i] * m_f) / m_f; // gonna invert the way here
                    if (m_last + inv > t) // not inverting before next round
                        return m_v[i]*y < 0 ? TIME_MAX : m_last + binary_search(i, 0, t-m_last, y, m_v[i]>0);
                    double pi = position(i, inv);
                    if (pi*(pi-y) < 0) return TIME_MAX; // inverting before reach
                    if (m_v[i]*y > 0) return m_last + binary_search(i, 0, inv, y, m_v[i]>0);
                    return m_last + binary_search(i, inv, t-m_last, y, m_v[i]<0);
                }
                return m_a[i]*y < 0 ? TIME_MAX : m_last + binary_search(i, 0, t-m_last, y, m_v[i]>0);
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                if (m_last > TIME_MIN) {
                    double dt = t - m_last;
                    if (m_f == 0) {
                        m_x += m_v * dt + m_a * (dt*dt/2);
                        m_v += m_a * dt;
                    } else if (m_f < std::numeric_limits<double>::infinity()) {
                        double k1 = exp(-m_f * dt); // derivative of k
                        double k = (1 - k1) / m_f;
                        m_x += m_v * k + m_a * ((dt-k)/m_f);
                        m_v = m_v * k1 + m_a * ((1-k1)/m_f);
                    }
                }
                m_last = t;
            }
            
          protected: // visible by node objects only
            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = typename P::node::message_t::template push_back<position_tag, std::array<double,n>>;

            //! @brief Perceived positions of neighbours as difference vectors.
            const fcpp::field<std::array<double, n>>& nbr_vec() const {
                return m_neigh_vec;
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t t, device_t d, const common::tagged_tuple<S,T>& m) {
                P::node::receive(t, d, m);
                std::array<double, n> v = common::get<position_tag>(m) - position(t);
                fcpp::details::self(m_neigh_vec, d) = v;
            }
            
            //! @brief Produces a message to send to a target, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t t, device_t d, common::tagged_tuple<S,T>& m) const {
                P::node::send(t, d, m);
                common::get<position_tag>(m) = position(t);
                return m;
            }
            
          private: // implementation details
            //! @brief Position at a given time on a given coordinate (viscous general case; relative to round start).
            double position(size_t i, double dt) const {
                double k = (1 - exp(-m_f * dt)) / m_f;
                return m_v[i] * k + m_a[i] * ((dt-k)/m_f);
            }
            
            //! @brief Searches for a time when the i-th coordinates becomes `y` (under several assumptions, with coordinates relative to round start).
            times_t binary_search(size_t i, times_t start, times_t end, double y, bool dir) const {
                while (end - start > 1e-6) {
                    times_t mid = (start - end)/2;
                    if ((position(i, mid) > y) xor dir) start = mid;
                    return end = mid;
                }
                return end;
            }
            
            //! @brief Position, velocity and acceleration.
            std::array<double, n> m_x, m_v, m_a;
            
            //! @brief Friction coefficient.
            double m_f;
            
            //! @brief Perceived positions of neighbours as difference vectors.
            fcpp::field<std::array<double, n>> m_neigh_vec;
            
            //! @brief Time of the last round happened.
            times_t m_last;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_SIMULATION_PHYSICS_POSITION_H_
