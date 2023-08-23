// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file simulated_positioner.hpp
 * @brief Implementation of the `simulated_positioner` component handling physical evolution of a position through time.
 */

#ifndef FCPP_SIMULATION_SIMULATED_POSITIONER_H_
#define FCPP_SIMULATION_SIMULATED_POSITIONER_H_

#include <cmath>

#include <type_traits>

#include "lib/component/base.hpp"
#include "lib/data/field.hpp"
#include "lib/data/vec.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to the dimensionality of the space (defaults to 2).
    template <intmax_t n>
    struct dimension {};

    //! @brief Node initialisation tag associating to a starting position (required).
    struct x {};

    //! @brief Node initialisation tag associating to a starting velocity (defaults to the null vector).
    struct v {};

    //! @brief Node initialisation tag associating to a starting acceleration (defaults to the null vector).
    struct a {};

    //! @brief Node initialisation tag associating to a starting friction coefficient (defaults to zero).
    struct f {};
}


//! @cond INTERNAL
namespace details {
    //! @brief Returns a vector of NaNs.
    template <size_t n>
    vec<n> nan_vec() {
        vec<n> v;
        for (size_t i=0; i<n; ++i) v[i] = std::numeric_limits<real_t>::quiet_NaN();
        return v;
    }
}
//! @endcond


/**
 * @brief Component handling physical evolution of a position through time.
 *
 * Requires a \ref timer parent component.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::dimension defines the dimensionality of the space (defaults to 2).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::x associates to a starting position (required).
 * - \ref tags::v associates to a starting velocity (defaults to the null vector).
 * - \ref tags::a associates to a starting acceleration (defaults to the null vector).
 * - \ref tags::f associates to a starting friction coefficient (defaults to zero).
 *
 * Vectors are modelled as \ref vec objects. Position \f$ x \f$ evolves as per the differential equation \f$ x'' = a - f x' \f$ of uniformily accelerated viscous motion.
 */
template <class... Ts>
struct simulated_positioner {
    //! @brief The dimensionality of the space.
    constexpr static intmax_t dimension = common::option_num<tags::dimension, 2, Ts...>;

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
        DECLARE_COMPONENT(positioner);
        REQUIRE_COMPONENT(positioner,timer);
        //! @endcond

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            //! @brief Type for representing a position.
            using position_type = vec<dimension>;

            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = typename P::node::message_t::template push_back<positioner_tag, position_type>;

            //! @cond INTERNAL
            #define MISSING_TAG_MESSAGE "\033[1m\033[4mmissing required tags::x node initialisation tag\033[0m"
            //! @endcond

            //! @{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_x(common::get_or<tags::x>(t, position_type{})), m_v(common::get_or<tags::v>(t, position_type{})), m_a(common::get_or<tags::a>(t, position_type{})), m_f(common::get_or<tags::f>(t, 0)), m_nbr_vec{details::nan_vec<dimension>()}, m_nbr_dist{INF} {
                static_assert(common::tagged_tuple<S,T>::tags::template count<tags::x> >= 1, MISSING_TAG_MESSAGE);
                m_last = TIME_MIN;
                fcpp::details::self(m_nbr_vec, P::node::uid) = vec<dimension>();
                fcpp::details::self(m_nbr_dist, P::node::uid) = 0;
            }

            #undef MISSING_TAG_MESSAGE

            //! @brief Position now.
            position_type& position() {
                return m_x;
            }

            //! @brief Position now (const access).
            position_type const& position() const {
                return m_x;
            }

            //! @brief Position at a given time.
            position_type position(times_t t) const {
                real_t dt = t - m_last;
                if (dt == 0) {
                    return m_x;
                }
                if (m_f == 0) {
                    return m_x + m_v * dt + m_a * (dt*dt/2);
                }
                if (m_f == INF) {
                    return m_x;
                }
                real_t k = (1 - exp(-m_f * dt)) / m_f;
                return m_x + m_v * k + m_a * ((dt-k)/m_f);
            }

            //! @brief Velocity now.
            position_type& velocity() {
                return m_v;
            }

            //! @brief Velocity now (const access).
            position_type const& velocity() const {
                return m_v;
            }

            //! @brief Velocity at a given time.
            position_type velocity(times_t t) const {
                real_t dt = t - m_last;
                if (dt == 0) {
                    return m_v;
                }
                if (m_f == 0) {
                    return m_v + m_a * dt;
                }
                if (m_f == INF) {
                    return {};
                }
                real_t k1 = exp(-m_f * dt); // derivative of k
                return m_v * k1 + m_a * ((1-k1)/m_f);
            }

            //! @brief Personal acceleration.
            position_type& propulsion() {
                return m_a;
            }

            //! @brief Personal acceleration (const access).
            position_type const& propulsion() const {
                return m_a;
            }

            //! @brief Total acceleration now.
            position_type acceleration() const {
                if (m_f == 0) {
                    return m_a;
                }
                if (m_f == INF) {
                    return {};
                }
                return m_a - m_f * m_v;
            }

            //! @brief Total acceleration at a given time.
            position_type acceleration(times_t t) const {
                if (m_f == 0) {
                    return m_a;
                }
                if (m_f == INF) {
                    return 0;
                }
                real_t dt = t - m_last;
                real_t k1 = exp(-m_f * dt);
                return m_a * k1 - m_v * (m_f * k1);
            }

            //! @brief Friction coefficient.
            real_t& friction() {
                return m_f;
            }

            //! @brief Friction coefficient (const access).
            real_t friction() const {
                return m_f;
            }

            //! @brief First time after `t` when a value `y` will be reached on a certain coordinate `i`.
            times_t reach_time(size_t i, real_t y, times_t t) {
                y -= m_x[i];
                t -= m_last;
                if (m_a[i] == m_v[i] * m_f) { // limit velocity reached, linear motion
                    real_t sol = y / m_v[i];
                    return sol > t ? m_last + sol : TIME_MAX;
                }
                if (m_f == 0) { // no friction, uniformily accelerated motion
                    real_t delta = m_v[i] * m_v[i] + 2 * y * m_a[i];
                    if (delta < 0) return TIME_MAX;
                    delta = sqrt(delta);
                    delta = m_a[i] > 0 ? delta : -delta;
                    real_t sol1 = (-m_v[i] - delta) / m_a[i]; // lower solution
                    real_t sol2 = (-m_v[i] + delta) / m_a[i]; // higher solution
                    return sol1 > t ? m_last + sol1 : sol2 > t ? m_last + sol2 : TIME_MAX;
                }
                if (m_f == INF) return TIME_MAX; // infinite friction, no motion
                if (m_a[i] == 0) { // no acceleration, exponentially decreasing motion
                    real_t sol = 1 - y * m_f / m_v[i];
                    sol = sol <= 0 ? TIME_MIN : -log(sol) / m_f;
                    return sol > t ? m_last + sol : TIME_MAX;
                }
                real_t inv = m_v[i]*m_a[i] > 0 ? TIME_MIN : log(1 - m_v[i] / m_a[i] * m_f) / m_f; // inversion time
                if (inv <= t) { // unidirectional motion
                    real_t vt = inv > 0 ? -m_v[i] : inv == 0 ? m_a[i] : m_v[i];
                    real_t xt = position(i, t);
                    return vt * (y-xt) > 0 ? m_last + binary_search(i, t, y) : TIME_MAX;
                }
                // motion with inversion
                real_t xt = position(i, t);
                real_t xi = position(i, inv);
                if ((y-xt)*(y-xi) < 0) return m_last + binary_search(i, t, inv, y); // meeting before inversion
                if ((y-xi)*m_v[i] < 0) return m_last + binary_search(i, inv, y); // meeting after inversion
                return TIME_MAX;
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                PROFILE_COUNT("positioner");
                if (m_last > TIME_MIN) {
                    real_t dt = t - m_last;
                    if (m_f == 0) {
                        m_x += m_v * dt + m_a * (dt*dt/2);
                        m_v += m_a * dt;
                    } else if (m_f < INF) {
                        real_t k1 = exp(-m_f * dt); // derivative of k
                        real_t k = (1 - k1) / m_f;
                        m_x += m_v * k + m_a * ((dt-k)/m_f);
                        m_v = m_v * k1 + m_a * ((1-k1)/m_f);
                    }
                }
                m_last = t;
            }

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t t, device_t d, common::tagged_tuple<S,T> const& m) {
                P::node::receive(t, d, m);
                position_type v = common::get<positioner_tag>(m) - position(t);
                if (d != P::node::uid) {
                    fcpp::details::self(m_nbr_vec, d) = v;
                    fcpp::details::self(m_nbr_dist, d) = norm(v);
                }
            }

            //! @brief Produces the message to send, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t t, common::tagged_tuple<S,T>& m) const {
                P::node::send(t, m);
                common::get<positioner_tag>(m) = position(t);
                return m;
            }

            //! @brief Perceived positions of neighbours as difference vectors.
            fcpp::field<position_type> const& nbr_vec() const {
                return m_nbr_vec;
            }

            //! @brief Perceived distances from neighbours.
            fcpp::field<real_t> const& nbr_dist() const {
                return m_nbr_dist;
            }

            //! @brief Lags since most recent distance measurements.
            fcpp::field<times_t> const& nbr_dist_lag() const {
                return P::node::nbr_lag();
            }

          private: // implementation details
            //! @brief Position at a given time on a given coordinate (viscous general case; relative to round start).
            real_t position(size_t i, real_t dt) const {
                real_t k = (1 - exp(-m_f * dt)) / m_f;
                return m_v[i] * k + m_a[i] * ((dt-k)/m_f);
            }

            //! @brief Searches for a time when the i-th coordinates becomes `y` (under several assumptions, with coordinates relative to round start), assuming motion is monotonic and viscous general case.
            times_t binary_search(size_t i, times_t start, times_t end, real_t y) const {
                real_t xs = position(i, start);
                real_t xe = position(i, end);
                bool dir = xe > xs;
                if ((y-xe)*(y-xs) > 0) return TIME_MIN; // it never happens
                while (end - start > 1e-6) {
                    times_t mid = (start - end)/2;
                    if ((position(i, mid) > y) xor dir) start = mid;
                    return end = mid;
                }
                return end;
            }
            //! @brief Searches for a time when the i-th coordinates becomes `y` (overload without an end time).
            times_t binary_search(size_t i, times_t start, real_t y) const {
                real_t dt = 1;
                real_t xs = position(i, start);
                if ((y-xs)*m_a[i] < 0) return TIME_MIN; // it never happens
                while ((y-position(i, start+dt))*(y-xs) <= 0) dt *= 2;
                return binary_search(i, start, start+dt, y);
            }

            //! @brief Position, velocity and acceleration.
            position_type m_x, m_v, m_a;

            //! @brief Friction coefficient.
            real_t m_f;

            //! @brief Perceived positions of neighbours as difference vectors.
            fcpp::field<position_type> m_nbr_vec;

            //! @brief Perceived distances from neighbours.
            fcpp::field<real_t> m_nbr_dist;

            //! @brief Time of the last round happened.
            times_t m_last;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_SIMULATION_SIMULATED_POSITIONER_H_
