// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file sequence.hpp
 * @brief Collection of generators of increasing sequences. Contrary to random distributions, sequence generators are stateful, that is, the generation of the next element depend on the previous elements generated (and are generated in increasing order).
 */

#ifndef FCPP_OPTION_SEQUENCE_H_
#define FCPP_OPTION_SEQUENCE_H_

#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <queue>
#include <type_traits>
#include <vector>

#include "lib/settings.hpp"
#include "lib/common/quaternion.hpp"
#include "lib/option/distribution.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing classes for random sequence generation.
namespace sequence {


//! @cond INTERNAL
namespace details {
    using distribution::details::call_distr;

    struct none_tag {};
    constexpr none_tag none{};
}
//! @endcond


//! @brief Empty generator for a sequence of no events.
struct never {
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Tagged tuple constructor.
    template <typename G, typename T>
    never(G&&, T&&) {}

    //! @brief Check whether the sequence is finished.
    bool empty() const {
        return true;
    }

    //! @brief Returns next event, without stepping over.
    type next() const {
        return TIME_MAX; // no event to schedule
    }

    //! @brief Steps over to next event, without returning.
    template <typename G, typename T>
    void step(G&&, T&&) {}

    //! @brief Returns next event, stepping over.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        return TIME_MAX; // no event to schedule
    }
};


//! @brief Generator of a series of contemporary events.
//! @{
/**
 * @brief With parameters as distributions (general form).
 * @tparam N The number of events (as distribution).
 * @tparam E The time of the events (as distribution).
 * @tparam contemporary Whether independent or contemporary events should be produced (defaults to true).
 */
template <typename N, typename E, bool contemporary = true>
class multiple;
//! @brief With parameters as distributions (contemporary events).
template <typename N, typename E>
class multiple<N, E, true> {
    static_assert(std::is_same<typename E::type, times_t>::value, "the distribution E must generate a times_t value");
    static_assert(std::is_same<typename N::type,  size_t>::value, "the distribution N must generate a size_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    multiple(G&& g, common::tagged_tuple<S,T> const& tup) : t(details::call_distr<E>(g,tup)), i(details::call_distr<N>(g,tup)) {}

    //! @brief Check whether the sequence is finished.
    bool empty() const {
        return i == 0;
    }

    //! @brief Returns next event, without stepping over.
    type next() const {
        return (i > 0) ? t : TIME_MAX;
    }

    //! @brief Steps over to next event, without returning.
    template <typename G, typename T>
    void step(G&&, T&&) {
        if (i > 0) --i;
    }

    //! @brief Returns next event, stepping over.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        type x = next();
        step(details::none, details::none);
        return x;
    }

  private:
    //! @brief The time of the events.
    times_t t;

    //! @brief Number of calls to next remaining.
    size_t i;
};
//! @brief With parameters as distributions (independent events).
template <typename N, typename E>
class multiple<N, E, false> {
    static_assert(std::is_same<typename E::type, times_t>::value, "the distribution E must generate a times_t value");
    static_assert(std::is_same<typename N::type,  size_t>::value, "the distribution N must generate a size_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    multiple(G&& g, common::tagged_tuple<S,T> const& tup) : multiple(g, tup, E{g,tup}, N{g,tup}) {}

    //! @brief Check whether the sequence is finished.
    bool empty() const {
        return pending.empty();
    }

    //! @brief Returns next event, without stepping over.
    type next() const {
        return pending.empty() ? TIME_MAX : pending.back();
    }

    //! @brief Steps over to next event, without returning.
    template <typename G, typename T>
    void step(G&&, T&&) {
        if (not pending.empty()) pending.pop_back();
    }

    //! @brief Returns next event, stepping over.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        type x = next();
        step(details::none, details::none);
        return x;
    }

  private:
    //! @brief The time of the events.
    std::vector<times_t> pending;

    //! @brief Auxiliary constructor.
    template <typename G, typename T>
    multiple(G&& g, T const& tup, E&& distr, N&& n) {
        size_t num = n(g, tup);
        pending.reserve(num);
        for (size_t j=0; j<num; ++j) pending.push_back(distr(g, tup));
        std::sort(pending.begin(), pending.end(), std::greater<times_t>{});
    }
};
/**
 * @brief With parameters as numeric template parameters.
 * @tparam n The number of events.
 * @tparam t The (integral) time of the events.
 * @tparam scale A scale factor by which the times are divided.
 */
template <size_t n, intmax_t t, intmax_t scale = 1>
using multiple_n = multiple<distribution::constant_n<size_t, n>, distribution::constant_n<times_t, t, scale>>;
/**
 * @brief With parameters as initialisation values.
 * @tparam n_tag The tag corresponding to number of events in initialisation values.
 * @tparam t_tag The tag corresponding to the time of events in initialisation values.
 */
template <typename n_tag, typename t_tag>
using multiple_i = multiple<distribution::constant_i<size_t, n_tag>, distribution::constant_i<times_t, t_tag>>;
//! @}


//! @brief Generator of a series of events at given times.
//! @{
/**
 * @brief With times as distributions.
 * @tparam Ds The times of the events (as distributions).
 */
template <typename... Ds>
class list {
    static_assert(common::number_all_true<std::is_same<typename Ds::type, times_t>::value...>, "the distributions Ds must generate a times_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    list(G&& g, common::tagged_tuple<S,T> const& tup) : pending({details::call_distr<Ds>(g,tup)...}) {
        std::sort(pending.begin(), pending.end());
    }

    //! @brief Check whether the sequence is finished.
    bool empty() const {
        return i >= sizeof...(Ds);
    }

    //! @brief Returns next event, without stepping over.
    type next() const {
        return (i < sizeof...(Ds)) ? pending[i] : TIME_MAX;
    }

    //! @brief Steps over to next event, without returning.
    template <typename G, typename T>
    void step(G&&, T&&) {
        ++i;
    }

    //! @brief Returns next event, stepping over.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        type x = next();
        step(details::none, details::none);
        return x;
    }

  private:
    //! @brief List of events to come.
    std::array<times_t, sizeof...(Ds)> pending;

    //! @brief Number of calls to next so far.
    size_t i = 0;
};
/**
 * @brief With times as numeric template parameters.
 * @tparam scale A scale factor by which the times are divided.
 * @tparam x     The (integral) times of the events.
 */
template <intmax_t scale, intmax_t... x>
using list_n = list<distribution::constant_n<times_t,x,scale>...>;
/**
 * @brief With times as initialisation values.
 * @tparam x_tag The tags corresponding to times in initialisation values.
 */
template <typename... x_tag>
using list_i = list<distribution::constant_i<times_t,x_tag>...>;
//! @}


//! @brief Generator of a series of events at given times.
//! @{
/**
 * @brief With parameters as distributions.
 * @tparam S The first event (as distribution).
 * @tparam P The period (as distribution).
 * @tparam E The maximum admissible time (as distribution).
 * @tparam N The maximum number of events (as distribution).
 */
template <typename S, typename P = S, typename E = never, typename N = distribution::constant_n<size_t, -1>>
class periodic {
    static_assert(std::is_same<typename S::type, times_t>::value, "the distribution S must generate a times_t value");
    static_assert(std::is_same<typename P::type, times_t>::value, "the distribution P must generate a times_t value");
    static_assert(std::is_same<typename E::type, times_t>::value, "the distribution E must generate a times_t value");
    static_assert(std::is_same<typename N::type, size_t>::value, "the distribution N must generate a size_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Tagged tuple constructor.
    template <typename G, typename U, typename T>
    periodic(G&& g, common::tagged_tuple<U,T> const& tup) : dp(g,tup) {
        n  = details::call_distr<N>(g,tup);
        te = details::call_distr<E>(g,tup);
        t  = details::call_distr<S>(g,tup);
    }

    //! @brief Check whether the sequence is finished.
    bool empty() const {
        return i >= n or t > te;
    }

    //! @brief Returns next event, without stepping over.
    type next() const {
        return (i < n and t <= te) ? t : TIME_MAX;
    }

    //! @brief Steps over to next event, without returning.
    template <typename G, typename U, typename T>
    void step(G&& g, common::tagged_tuple<U,T> const& tup) {
        ++i;
        t += dp(g, t);
    }

    //! @brief Returns next event, stepping over.
    template <typename G, typename U, typename T>
    type operator()(G&& g, common::tagged_tuple<U,T> const& tup) {
        type x = next();
        step(std::forward<G>(g), tup);
        return x;
    }

  private:
    //! @brief Period distribution;
    P dp;

    //! @brief Last event happened and terminal time.
    times_t t, te;

    //! @brief Number of calls to next so far.
    size_t n, i = 0;
};
/**
 * @brief With parameters as numeric template parameters.
 * @tparam scale A scale factor by which the `s`, `p` and `e` parameters are divided.
 * @tparam s The (integral) first event.
 * @tparam p The (integral) period.
 * @tparam e The (integral) maximum admissible time.
 * @tparam n The maximum number of events.
 */
template <intmax_t scale, intmax_t s, intmax_t p = s, intmax_t e = std::numeric_limits<intmax_t>::max(), intmax_t n = -1>
using periodic_n = periodic<
    distribution::constant_n<times_t, s, scale>,
    distribution::constant_n<times_t, p, scale>,
    distribution::constant_n<times_t, e, e == std::numeric_limits<intmax_t>::max() ? 0 : scale>,
    distribution::constant_n<size_t,  n>
>;
/**
 * @brief With parameters as initialisation values.
 * @tparam s_tag The tag corresponding to the first event in initialisation values.
 * @tparam p_tag The tag corresponding to the period in initialisation values.
 * @tparam e_tag The tag corresponding to the maximum admissible time in initialisation values.
 * @tparam n_tag The tag corresponding to the maximum number of events in initialisation values.
 */
template <typename s_tag, typename p_tag = s_tag, typename e_tag = void, typename n_tag = void>
using periodic_i = periodic<
    distribution::constant_i<times_t, s_tag>,
    distribution::constant_i<times_t, p_tag>,
    distribution::constant_n<times_t, +1, 0, e_tag>,
    distribution::constant_n<size_t,  -1, 1, n_tag>
>;
//! @}


/**
 * @brief Merges multiple sequences in a single one.
 * @tparam Ss Generators of event sequences.
 */
template <typename... Ss>
class merge {
    static_assert(common::number_all_true<std::is_same<typename Ss::type, times_t>::value...>, "the generators Ss must generate a times_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief The number of sequences merged.
    constexpr static size_t size = sizeof...(Ss);

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    merge(G&& g, common::tagged_tuple<S,T> const& tup) : m_generators{{common::type_pack_wrapper<Ss>(g),tup}...} {
        fill_queue(std::make_index_sequence<size>{});
    }

    //! @brief Check whether the sequence is finished.
    bool empty() const {
        return next() == TIME_MAX;
    }

    //! @brief Returns next event, without stepping over.
    type next() const {
        return m_queue.top().first;
    }

    //! @brief Returns the index of the subsequence generating the next event.
    size_t next_sequence() const {
        return m_queue.top().second;
    }

    //! @brief Steps over to next event, without returning.
    template <typename G, typename S, typename T>
    void step(G&& g, common::tagged_tuple<S,T> const& tup) {
        step(std::forward<G>(g), tup, std::index_sequence<0, size>{});
    }

    //! @brief Returns next event, stepping over.
    template <typename G, typename S, typename T>
    type operator()(G&& g, common::tagged_tuple<S,T> const& tup) {
        type x = next();
        step(std::forward<G>(g), tup);
        return x;
    }

  private:
    //! @brief The type used to indicate an event in the queue.
    using event_t = std::pair<times_t, size_t>;

    //! @brief Initially fills up the queue.
    template <size_t... is>
    inline void fill_queue(std::index_sequence<is...>) {
        common::ignore_args((m_queue.emplace(std::get<is>(m_generators).next(), is),0)...);
    }

    //! @brief Steps over a given sub-sequence.
    template <typename G, typename T, size_t i, size_t j>
    inline std::enable_if_t<j == i+1> step(G&& g, T const& tup, std::index_sequence<i, j>) {
        assert(m_queue.top().second == i);
        m_queue.pop();
        std::get<i>(m_generators).step(std::forward<G>(g), tup);
        m_queue.emplace(std::get<i>(m_generators).next(), i);
    }

    //! @brief Steps over a sub-sequence among an interval of possible ones.
    template <typename G, typename T, size_t i, size_t j>
    inline std::enable_if_t<(j > i+1)> step(G&& g, T const& tup, std::index_sequence<i, j>) {
        constexpr size_t k = (i + j) / 2;
        if (m_queue.top().second < k)
            step(std::forward<G>(g), tup, std::index_sequence<i, k>{});
        else
            step(std::forward<G>(g), tup, std::index_sequence<k, j>{});
    }

    //! @brief Tuple of sequence generators.
    std::tuple<Ss...> m_generators;

    //! @brief The queue of events to come by generator.
    std::priority_queue<event_t, std::vector<event_t>, std::greater<event_t>> m_queue;
};

//! @brief Optimisation for a single sequence.
template <typename S>
class merge<S> : public S {
  public:
    using S::S;

    //! @brief Returns the index of the subsequence generating the next event.
    size_t next_sequence() const {
        return 0;
    }
};

//! @brief Optimisation for no sequences.
template <>
class merge<> : public never {
  public:
    using never::never;

    //! @brief Returns the index of the subsequence generating the next event.
    size_t next_sequence() const {
        assert(false);
        return -1;
    }
};


//! @brief Merges multiple sequences wrapped in a type sequence.
template <typename T>
using merge_t = common::apply_templates<T, merge>;


//! @brief Generates points in a grid given its extremes and the number of steps per axis.
//! @{
//! @cond INTERNAL
namespace details {
    template <typename L, typename U, typename N>
    struct grid;
    template <typename... Ls, typename... Us, typename... Ns>
    struct grid<common::type_sequence<Ls...>, common::type_sequence<Us...>, common::type_sequence<Ns...>> {
        //! @brief The dimensionality of the space.
        static constexpr size_t n = sizeof...(Ls);

        //! @brief The type of results generated.
        using type = vec<n>;

        //! @brief Tagged tuple constructor.
        template <typename G, typename S, typename T>
        grid(G&& g, common::tagged_tuple<S,T> const& tup) : m_i(0) {
            m_init = {details::call_distr<Ls>(g, tup)...};
            m_step = {details::call_distr<Us>(g, tup)...};
            m_mods = {details::call_distr<Ns>(g, tup)...};
            for (size_t i=0; i<n; ++i) m_step[i] = (m_step[i]-m_init[i])/std::max(m_mods[i]-1, size_t(1));
            m_divs[0] = 1;
            for (size_t i=1; i<n; ++i) m_divs[i] = m_divs[i-1]*m_mods[i-1];
        }

        //! @brief Check whether the sequence is finished.
        bool empty() const {
            return m_i >= m_divs.back()*m_mods.back();
        }

        //! @brief Returns next element, without stepping over.
        type next() const {
            vec<n> p = m_init;
            for (size_t i=0; i<n; ++i) p[i] += m_step[i] * ((m_i/m_divs[i]) % m_mods[i]);
            return p;
        }

        //! @brief Steps over to next element, without returning.
        template <typename G, typename T>
        void step(G&&, T&&) {
            ++m_i;
        }

        //! @brief Returns next element, stepping over.
        template <typename G, typename T>
        type operator()(G&&, T&&) {
            type x = next();
            step(details::none, details::none);
            return x;
        }

      private:
        //! @brief The lower point and step of the grid.
        vec<n> m_init, m_step;

        //! @brief The divisors and modules for each dimension.
        std::array<size_t, n> m_divs, m_mods;

        //! @brief Number of calls to next remaining.
        size_t m_i;
    };
    template <intmax_t x>
    struct num_wrap {};
    template <intmax_t scale, typename L, typename U, typename N>
    struct grid_n;
    template <intmax_t scale, intmax_t... ls, intmax_t... us, intmax_t... ns>
    struct grid_n<scale, common::type_sequence<num_wrap<ls>...>, common::type_sequence<num_wrap<us>...>, common::type_sequence<num_wrap<ns>...>> {
        using type = grid<
            common::type_sequence<distribution::constant_n<real_t, ls, scale>...>,
            common::type_sequence<distribution::constant_n<real_t, us, scale>...>,
            common::type_sequence<distribution::constant_n<size_t, ns>...>
        >;
    };
    template <intmax_t scale, typename... Ds>
    using grid_n_splitter = typename grid_n<scale,
        common::type_slice<0,                sizeof...(Ds)*1/3,1,Ds...>,
        common::type_slice<sizeof...(Ds)*1/3,sizeof...(Ds)*2/3,1,Ds...>,
        common::type_slice<sizeof...(Ds)*2/3,sizeof...(Ds),    1,Ds...>
    >::type;
    template <typename L, typename U, typename N>
    struct grid_i;
    template <typename... l_tags, typename... u_tags, typename... n_tags>
    struct grid_i<common::type_sequence<l_tags...>, common::type_sequence<u_tags...>, common::type_sequence<n_tags...>> {
        using type = grid<
            common::type_sequence<distribution::constant_i<real_t, l_tags>...>,
            common::type_sequence<distribution::constant_i<real_t, u_tags>...>,
            common::type_sequence<distribution::constant_i<size_t, n_tags>...>
        >;
    };
}
//! @endcond

/**
 * @brief With extremes and numerosities as distributions.
 *
 * The first two-thirds of `Ds::type` must be convertible to `real_t`.
 * The last third must be convertible to `size_t`.
 *
 * @tparam Ds The extremes and numerosities (as distributions).
 */
template <typename... Ds>
using grid = details::grid<
    common::type_slice<0,                sizeof...(Ds)*1/3,1,Ds...>,
    common::type_slice<sizeof...(Ds)*1/3,sizeof...(Ds)*2/3,1,Ds...>,
    common::type_slice<sizeof...(Ds)*2/3,sizeof...(Ds),    1,Ds...>
>;
/**
 * @brief With extremes and numerosities as numeric template parameters.
 * @tparam scale A scale factor by which the extremes are divided.
 * @tparam x The (integral) extremes.
 */
template <intmax_t scale, intmax_t... x>
using grid_n = details::grid_n_splitter<scale, details::num_wrap<x>...>;
/**
 * @brief With extremes and numerosities as initialisation values.
 * @tparam x_tag The tags corresponding to extremes and numerosities in initialisation values.
 */
template <typename... x_tag>
using grid_i = typename details::grid_i<
    common::type_slice<0,                   sizeof...(x_tag)*1/3,1,x_tag...>,
    common::type_slice<sizeof...(x_tag)*1/3,sizeof...(x_tag)*2/3,1,x_tag...>,
    common::type_slice<sizeof...(x_tag)*2/3,sizeof...(x_tag),    1,x_tag...>
>::type;
//! @}


//! @brief Generates points in a circle given its center, radius and orientation, and the number of points.
//! @{
//! @cond INTERNAL
namespace details {
    struct angle {
        angle(real_t r = 1) : data(0) {
            assert(r == 1);
        }

        angle(real_t a, real_t const*) : data(a) {}

        real_t data;
    };

    inline angle& operator*=(angle& x, angle const& y) {
        x.data += y.data;
        return x;
    }

    using common::quaternion;

    inline vec<3> rotate(quaternion const& r, vec<3> const& v) {
        quaternion q = r * quaternion(v.data) * ~r;
        return vec<3>{q[1], q[2], q[3]};
    }

    inline vec<2> rotate(angle const& r, vec<2> const& v) {
        real_t c = cos(r.data);
        real_t s = sin(r.data);
        return {v[0]*c-v[1]*s, v[0]*s+v[1]*c};
    }

    inline vec<3> perpendicular(vec<3> const& v) {
        size_t i = 0;
        for (int j = 1; j < 3; ++j) if (abs(v[j]) < abs(v[i])) i = j;
        vec<3> w;
        w[i] = 0;
        w[(i+1)%3] = -v[(i+2)%3];
        w[(i+2)%3] = +v[(i+1)%3];
        w *= norm(v) / norm(w);
        return w;
    }

    inline vec<2> perpendicular(vec<1> const& v) {
        return {v[0], 0};
    }
}
//! @endcond

/**
 * @brief With center, radius and numerosity as distributions.
 *
 * `C::type` and `R::type` must either be both `vec<3>`, or `vec<2>` and `vec<1>` respectively.
 * `N::type` must be convertible to `size_t`.
 *
 * @tparam C The center (as distribution).
 * @tparam R The radius (as distribution).
 * @tparam N The numerosity (as distribution).
 */
template <typename C, typename R, typename N>
class circle {
  public:
    //! @brief The type of results generated.
    using type = typename C::type;

    //! @brief The dimensionality of the space.
    static constexpr size_t n = type::dimension;

    //! @brief The type for rotations.
    using rotation_type = std::conditional_t<n == 2, details::angle, common::quaternion>;

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    circle(G&& g, common::tagged_tuple<S,T> const& t) {
        m_c = details::call_distr<C>(g,t);
        auto r = details::call_distr<R>(g,t);
        m_p = details::perpendicular(r);
        m_i = details::call_distr<N>(g,t);
        m_r0 = rotation_type(2*acos(-1)/m_i, r.data);
        m_r = rotation_type(1);
    }

    //! @brief Check whether the sequence is finished.
    bool empty() const {
        return m_i <= 0;
    }

    //! @brief Returns next element, without stepping over.
    type next() const {
        return m_c + details::rotate(m_r, m_p);
    }

    //! @brief Steps over to next element, without returning.
    template <typename G, typename T>
    void step(G&&, T&&) {
        m_r *= m_r0;
        --m_i;
    }

    //! @brief Returns next element, stepping over.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        type x = next();
        step(details::none, details::none);
        return x;
    }

  private:
    type m_c, m_p;
    rotation_type m_r0, m_r;
    int m_i;
};


/**
 * @brief With center, radius and numerosity as numeric template parameters.
 *
 * @tparam scale A scale factor by which the coordinates are divided.
 * @tparam xs The (integral) center, radius and numerosity.
 */
template <intmax_t scale, intmax_t... xs>
struct circle_n;

template <intmax_t scale, intmax_t cx, intmax_t cy, intmax_t r, intmax_t num>
struct circle_n<scale, cx, cy, r, num> : public circle<distribution::point_n<scale, cx, cy>, distribution::point_n<scale, r>, distribution::constant_n<size_t, num, 1>> {
    using circle<distribution::point_n<scale, cx, cy>, distribution::point_n<scale, r>, distribution::constant_n<size_t, num, 1>>::circle;
};

template <intmax_t scale, intmax_t cx, intmax_t cy, intmax_t cz, intmax_t rx, intmax_t ry, intmax_t rz, intmax_t num>
struct circle_n<scale, cx, cy, cz, rx, ry, rz, num> : public circle<distribution::point_n<scale, cx, cy, cz>, distribution::point_n<scale, rx, ry, rz>, distribution::constant_n<size_t, num, 1>> {
    using circle<distribution::point_n<scale, cx, cy, cz>, distribution::point_n<scale, rx, ry, rz>, distribution::constant_n<size_t, num, 1>>::circle;
};

/**
 * @brief With center, radius and numerosity as initialisation values.
 *
 * @tparam c_tag The center tag.
 * @tparam r_tag The radius tag.
 * @tparam n_tag The numerosity tag.
 * @tparam n     The dimensionality.
 */
template <typename c_tag, typename r_tag, typename n_tag, intmax_t n>
struct circle_i;

template <typename c_tag, typename r_tag, typename n_tag>
struct circle_i<c_tag, r_tag, n_tag, 2> : public circle<distribution::constant_i<vec<2>, c_tag>, distribution::constant_i<vec<1>, r_tag>, distribution::constant_i<size_t, n_tag>> {
    using circle<distribution::constant_i<vec<2>, c_tag>, distribution::constant_i<vec<1>, r_tag>, distribution::constant_i<size_t, n_tag>>::circle;
};

template <typename c_tag, typename r_tag, typename n_tag>
struct circle_i<c_tag, r_tag, n_tag, 3> : public circle<distribution::constant_i<vec<3>, c_tag>, distribution::constant_i<vec<3>, r_tag>, distribution::constant_i<size_t, n_tag>> {
    using circle<distribution::constant_i<vec<3>, c_tag>, distribution::constant_i<vec<3>, r_tag>, distribution::constant_i<size_t, n_tag>>::circle;
};
//! @}


}


}

#endif // FCPP_OPTION_SEQUENCE_H_
