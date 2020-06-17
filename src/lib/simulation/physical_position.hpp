// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file physical_position.hpp
 * @brief Implementation of the `physical_position` component handling physical evolution of a position through time.
 */

#ifndef FCPP_SIMULATION_PHYSICAL_POSITION_H_
#define FCPP_SIMULATION_PHYSICAL_POSITION_H_

#include <cmath>

#include <array>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/array.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/component/base.hpp"
#include "lib/data/field.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to the dimensionality of the space.
    template <size_t n>
    struct dimension {};

    //! @brief Node initialisation tag associating to a starting position.
    struct x {};

    //! @brief Node initialisation tag associating to a starting velocity.
    struct v {};

    //! @brief Node initialisation tag associating to a starting acceleration.
    struct a {};

    //! @brief Node initialisation tag associating to a starting friction coefficient.
    struct f {};
}


//! @cond INTERNAL
namespace details {
    //! @brief Returns a vector of NaNs.
    template <size_t n>
    std::array<double, n> nan_vec() {
        std::array<double, n> v;
        for (size_t i=0; i<n; ++i) v[i] = std::numeric_limits<double>::quiet_NaN();
        return v;
    }
}
//! @endcond


/**
 * @brief Component handling physical evolution of a position through time.
 *
 * Must be unique in a composition of components.
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
 * Vectors are modelled as `std::array<double,n>` objects. Position \f$ x \f$ evolves as per the differential equation \f$ x'' = a - f x' \f$ of uniformily accelerated viscous motion.
 */
template <class... Ts>
struct physical_position {
    //! @brief The dimensionality of the space.
    constexpr static size_t dimension = common::option_num<tags::dimension, 2, Ts...>;

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
            //! @brief Type for representing a position.
            using position_type = std::array<double, dimension>;

            //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
            using message_t = typename P::node::message_t::template push_back<position_tag, position_type>;

            //@{
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_x(common::get<tags::x>(t)), m_v(common::get_or<tags::v>(t, position_type{})), m_a(common::get_or<tags::a>(t, position_type{})), m_f(common::get_or<tags::f>(t, 0.0)), m_nbr_vec{details::nan_vec<dimension>()}, m_nbr_dist{std::numeric_limits<double>::infinity()} {
                m_last = TIME_MIN;
            }

            //! @brief Position now (const access).
            const position_type& position() const {
                return m_x;
            }

            //! @brief Position at a given time.
            position_type position(times_t t) const {
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
            position_type& velocity() {
                return m_v;
            }

            //! @brief Velocity now (const access).
            const position_type& velocity() const {
                return m_v;
            }

            //! @brief Velocity at a given time.
            position_type velocity(times_t t) const {
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
            position_type& propulsion() {
                return m_a;
            }

            //! @brief Personal acceleration (const access).
            const position_type& propulsion() const {
                return m_a;
            }

            //! @brief Total acceleration now.
            position_type acceleration() const {
                if (m_f == 0) {
                    return m_a;
                }
                if (m_f == std::numeric_limits<double>::infinity()) {
                    return {};
                }
                return m_a - m_f * m_v;
            }

            //! @brief Total acceleration at a given time.
            position_type acceleration(times_t t) const {
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

            //! @brief First time after `t` when a value `y` will be reached on a certain coordinate `i`.
            times_t reach_time(size_t i, double y, times_t t) {
                y -= m_x[i];
                t -= m_last;
                if (m_a[i] == m_v[i] * m_f) { // limit velocity reached, linear motion
                    double sol = y / m_v[i];
                    return sol > t ? m_last + sol : TIME_MAX;
                }
                if (m_f == 0) { // no friction, uniformily accelerated motion
                    double delta = m_v[i] * m_v[i] + 2 * y * m_a[i];
                    if (delta < 0) return TIME_MAX;
                    delta = sqrt(delta);
                    delta = m_a[i] > 0 ? delta : -delta;
                    double sol1 = (-m_v[i] - delta) / m_a[i]; // lower solution
                    double sol2 = (-m_v[i] + delta) / m_a[i]; // higher solution
                    return sol1 > t ? m_last + sol1 : sol2 > t ? m_last + sol2 : TIME_MAX;
                }
                if (m_f == std::numeric_limits<double>::infinity()) return TIME_MAX; // infinite friction, no motion
                if (m_a[i] == 0) { // no acceleration, exponentially decreasing motion
                    double sol = 1 - y * m_f / m_v[i];
                    sol = sol <= 0 ? TIME_MIN : -log(sol) / m_f;
                    return sol > t ? m_last + sol : TIME_MAX;
                }
                double inv = m_v[i]*m_a[i] > 0 ? TIME_MIN : log(1 - m_v[i] / m_a[i] * m_f) / m_f; // inversion time
                if (inv <= t) { // unidirectional motion
                    double vt = inv > 0 ? -m_v[i] : inv == 0 ? m_a[i] : m_v[i];
                    double xt = position(i, t);
                    return vt * (y-xt) > 0 ? m_last + binary_search(i, t, y) : TIME_MAX;
                }
                // motion with inversion
                double xt = position(i, t);
                double xi = position(i, inv);
                if ((y-xt)*(y-xi) < 0) return m_last + binary_search(i, t, inv, y); // meeting before inversion
                if ((y-xi)*m_v[i] < 0) return m_last + binary_search(i, inv, y); // meeting after inversion
                return TIME_MAX;
            }

            //! @brief Performs computations at round start with current time `t`.
            void round_start(times_t t) {
                P::node::round_start(t);
                PROFILE_COUNT("positioner");
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

            //! @brief Receives an incoming message (possibly reading values from sensors).
            template <typename S, typename T>
            void receive(times_t t, device_t d, const common::tagged_tuple<S,T>& m) {
                P::node::receive(t, d, m);
                position_type v = common::get<position_tag>(m) - position(t);
                fcpp::details::self(m_nbr_vec, d) = v;
                fcpp::details::self(m_nbr_dist, d) = norm(v);
            }

            //! @brief Produces a message to send to a target, both storing it in its argument and returning it.
            template <typename S, typename T>
            common::tagged_tuple<S,T>& send(times_t t, device_t d, common::tagged_tuple<S,T>& m) const {
                P::node::send(t, d, m);
                common::get<position_tag>(m) = position(t);
                return m;
            }

            //! @brief Perceived positions of neighbours as difference vectors.
            const fcpp::field<position_type>& nbr_vec() const {
                return m_nbr_vec;
            }

            //! @brief Perceived distances from neighbours.
            const fcpp::field<double>& nbr_dist() const {
                return m_nbr_dist;
            }

          private: // implementation details
            //! @brief Position at a given time on a given coordinate (viscous general case; relative to round start).
            double position(size_t i, double dt) const {
                double k = (1 - exp(-m_f * dt)) / m_f;
                return m_v[i] * k + m_a[i] * ((dt-k)/m_f);
            }

            //! @brief Searches for a time when the i-th coordinates becomes `y` (under several assumptions, with coordinates relative to round start), assuming motion is monotonic and viscous general case.
            times_t binary_search(size_t i, times_t start, times_t end, double y) const {
                double xs = position(i, start);
                double xe = position(i, end);
                bool dir = xe > xs;
                if ((y-xe)*(y-xs) > 0) return TIME_MIN; // it never happens
                while (end - start > 1e-6) {
                    times_t mid = (start - end)/2;
                    if ((position(i, mid) > y) xor dir) start = mid;
                    return end = mid;
                }
                return end;
            }
            times_t binary_search(size_t i, times_t start, double y) const {
                double dt = 1;
                double xs = position(i, start);
                if ((y-xs)*m_a[i] < 0) return TIME_MIN; // it never happens
                while ((y-position(i, start+dt))*(y-xs) <= 0) dt *= 2;
                return binary_search(i, start, start+dt, y);
            }

            //! @brief Position, velocity and acceleration.
            position_type m_x, m_v, m_a;

            //! @brief Friction coefficient.
            double m_f;

            //! @brief Perceived positions of neighbours as difference vectors.
            fcpp::field<position_type> m_nbr_vec;

            //! @brief Perceived distances from neighbours.
            fcpp::field<double> m_nbr_dist;

            //! @brief Time of the last round happened.
            times_t m_last;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_SIMULATION_PHYSICAL_POSITION_H_
