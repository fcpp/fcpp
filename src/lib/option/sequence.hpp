// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

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
#include <type_traits>
#include <vector>

#include "lib/settings.hpp"
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
}
//! @endcond


//! @brief Empty generator for a sequence of no events.
struct never {
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    never(G&&) {}

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    never(G&&, const common::tagged_tuple<S,T>&) {}

    //! @brief Returns next event, without stepping over.
    times_t next() const {
        return TIME_MAX; // no event to schedule
    }

    //! @brief Steps over to next event, without returning.
    template <typename G>
    void step(G&&) {}

    //! @brief Returns next event, stepping over.
    template <typename G>
    times_t operator()(G&&) {
        return TIME_MAX; // no event to schedule
    }
};


//! @brief Generator of a series of contemporary events.
//! @{
/**
 * @brief With parameters as distributions (general form).
 * @param N The number of events (as distribution).
 * @param E The time of the events (as distribution).
 * @param contemporary Whether independent or contemporary events should be produced (defaults to true).
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

    //! @brief Default constructor.
    template <typename G>
    multiple(G&& g) : t(details::call_distr<E>(g)), i(details::call_distr<N>(g)) {}

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    multiple(G&& g, const common::tagged_tuple<S,T>& tup) : t(details::call_distr<E>(g,tup)), i(details::call_distr<N>(g,tup)) {}

    //! @brief Returns next event, without stepping over.
    times_t next() const {
        return (i > 0) ? t : TIME_MAX;
    }

    //! @brief Steps over to next event, without returning.
    template <typename G>
    void step(G&&) {
        if (i > 0) --i;
    }

    //! @brief Returns next event, stepping over.
    template <typename G>
    times_t operator()(G&&) {
        times_t nt = next();
        step(nullptr);
        return nt;
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

    //! @brief Default constructor.
    template <typename G>
    multiple(G&& g) : multiple(g, E{g}, N{g}) {}

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    multiple(G&& g, const common::tagged_tuple<S,T>& tup) : multiple(g, E{g,tup}, N{g,tup}) {}

    //! @brief Returns next event, without stepping over.
    times_t next() const {
        return pending.empty() ? TIME_MAX : pending.back();
    }

    //! @brief Steps over to next event, without returning.
    template <typename G>
    void step(G&&) {
        if (not pending.empty()) pending.pop_back();
    }

    //! @brief Returns next event, stepping over.
    template <typename G>
    times_t operator()(G&&) {
        times_t nt = next();
        step(nullptr);
        return nt;
    }

  private:
    //! @brief The time of the events.
    std::vector<times_t> pending;

    //! @brief Auxiliary constructor.
    template <typename G>
    multiple(G&& g, E&& distr, N&& n) {
        size_t num = n(g);
        pending.reserve(num);
        for (size_t j=0; j<num; ++j) pending.push_back(distr(g));
        std::sort(pending.begin(), pending.end(), std::greater<int>{});
    }
};
//! @}
/**
 * @brief With parameters as numeric template parameters.
 * @param n The number of events.
 * @param t The (integral) time of the events.
 * @param scale A scale factor by which the times are divided.
 */
template <size_t n, intmax_t t, intmax_t scale = 1>
using multiple_n = multiple<distribution::constant_n<size_t, n>, distribution::constant_n<times_t, t, scale>>;
/**
 * @brief With parameters as initialisation values.
 * @param n_tag The tag corresponding to number of events in initialisation values.
 * @param t_tag The tag corresponding to the time of events in initialisation values.
 */
template <typename n_tag, typename t_tag>
using multiple_i = multiple<distribution::constant_i<size_t, n_tag>, distribution::constant_i<times_t, t_tag>>;
//! @}


//! @brief Generator of a series of events at given times.
//! @{
/**
 * @brief With times as distributions.
 * @param Ds The times of the events (as distributions).
 */
template <typename... Ds>
class list {
    static_assert(common::all_true<std::is_same<typename Ds::type, times_t>::value...>, "the distributions Ds must generate a times_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    list(G&& g) : pending({details::call_distr<Ds>(g)...}) {
        std::sort(pending.begin(), pending.end());
    }

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    list(G&& g, const common::tagged_tuple<S,T>& tup) : pending({details::call_distr<Ds>(g,tup)...}) {
        std::sort(pending.begin(), pending.end());
    }

    //! @brief Returns next event, without stepping over.
    times_t next() const {
        return (i < sizeof...(Ds)) ? pending[i] : TIME_MAX;
    }

    //! @brief Steps over to next event, without returning.
    template <typename G>
    void step(G&&) {
        ++i;
    }

    //! @brief Returns next event, stepping over.
    template <typename G>
    times_t operator()(G&&) {
        times_t nt = next();
        ++i;
        return nt;
    }

  private:
    //! @brief List of events to come.
    std::array<times_t, sizeof...(Ds)> pending;

    //! @brief Number of calls to next so far.
    size_t i = 0;
};
/**
 * @brief With times as numeric template parameters.
 * @param scale A scale factor by which the times are divided.
 * @param x     The (integral) times of the events.
 */
template <intmax_t scale, intmax_t... x>
using list_n = list<distribution::constant_n<times_t,x,scale>...>;
/**
 * @brief With times as initialisation values.
 * @param x_tag The tags corresponding to times in initialisation values.
 */
template <typename... x_tag>
using list_i = list<distribution::constant_i<times_t,x_tag>...>;
//! @}


//! @brief Generator of a series of events at given times.
//! @{
/**
 * @brief With parameters as distributions.
 * @param S The first event (as distribution).
 * @param P The period (as distribution).
 * @param E The maximum admissible time (as distribution).
 * @param N The maximum number of events (as distribution).
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

    //! @brief Default constructor.
    template <typename G>
    periodic(G&& g) : dp(g) {
        n  = details::call_distr<N>(g);
        te = details::call_distr<E>(g);
        t  = details::call_distr<S>(g);
    }

    //! @brief Tagged tuple constructor.
    template <typename G, typename U, typename T>
    periodic(G&& g, const common::tagged_tuple<U,T>& tup) : dp(g,tup) {
        n  = details::call_distr<N>(g,tup);
        te = details::call_distr<E>(g,tup);
        t  = details::call_distr<S>(g,tup);
    }

    //! @brief Returns next event, without stepping over.
    times_t next() const {
        return (i < n and t <= te) ? t : TIME_MAX;
    }

    //! @brief Steps over to next event, without returning.
    template <typename G>
    void step(G&& g) {
        ++i;
        t += dp(g);
    }

    //! @brief Returns next event, stepping over.
    template <typename G>
    times_t operator()(G&& g) {
        times_t nt = next();
        ++i;
        t += dp(g);
        return nt;
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
 * @param scale A scale factor by which the `s`, `p` and `e` parameters are divided.
 * @param s The (integral) first event.
 * @param p The (integral) period.
 * @param e The (integral) maximum admissible time.
 * @param n The maximum number of events.
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
 * @param s_tag The tag corresponding to the first event in initialisation values.
 * @param p_tag The tag corresponding to the period in initialisation values.
 * @param e_tag The tag corresponding to the maximum admissible time in initialisation values.
 * @param n_tag The tag corresponding to the maximum number of events in initialisation values.
 */
template <typename s_tag, typename p_tag = s_tag, typename e_tag = void, typename n_tag = void>
using periodic_i = periodic<
    distribution::constant_i<times_t, s_tag>,
    distribution::constant_i<times_t, p_tag>,
    distribution::constant_n<times_t, +1, 0, e_tag>,
    distribution::constant_n<size_t,  -1, 1, n_tag>
>;
//! @}


//! @cond INTERNAL
namespace details {
    template <typename T, typename U>
    U&& arg_expander(U&& x) {
        return std::forward<U>(x);
    }
}
//! @endcond


/**
 * @brief Merges multiple sequences in a single one.
 * @param Ss Generators of event sequences.
 */
template <typename... Ss>
class merge {
    static_assert(common::all_true<std::is_same<typename Ss::type, times_t>::value...>, "the generators Ss must generate a times_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    merge(G&& g) : m_generators{details::arg_expander<Ss>(g)...} {
        set_next(std::make_index_sequence<sizeof...(Ss)>{});
    }

    //! @brief Tagged tuple constructor.
    template <typename G, typename S, typename T>
    merge(G&& g, const common::tagged_tuple<S,T>& tup) : m_generators{{details::arg_expander<Ss>(g),tup}...} {
        set_next(std::make_index_sequence<sizeof...(Ss)>{});
    }

    //! @brief Returns next event, without stepping over.
    times_t next() const {
        return m_next;
    }

    //! @brief Returns the index of the subsequence generating the next event.
    size_t next_sequence() const {
        return next_sequence(std::make_index_sequence<sizeof...(Ss)>{});
    }

    //! @brief Steps over to next event, without returning.
    template <typename G>
    void step(G&& g) {
        step(std::forward<G>(g), std::make_index_sequence<sizeof...(Ss)>{});
        set_next(std::make_index_sequence<sizeof...(Ss)>{});
    }

    //! @brief Returns next event, stepping over.
    template <typename G>
    times_t operator()(G&& g) {
        times_t nt = next();
        step(std::forward<G>(g));
        return nt;
    }

  private:
    //! @brief Steps over a given sub-sequence.
    template <typename G, size_t i>
    void step(G&& g, std::index_sequence<i>) {
        assert(std::get<i>(m_generators).next() == m_next);
        std::get<i>(m_generators).step(std::forward<G>(g));
    }

    //! @brief Steps over a sub-sequence among a list of possible ones.
    template <typename G, size_t i, size_t... is>
    void step(G&& g, std::index_sequence<i, is...>) {
        if (std::get<i>(m_generators).next() == m_next)
            std::get<i>(m_generators).step(std::forward<G>(g));
        else
            step(std::forward<G>(g), std::index_sequence<is...>{});
    }

    //! @brief Computes the next event.
    template <size_t... is>
    void set_next(std::index_sequence<is...>) {
        std::array<times_t, sizeof...(Ss)> v{std::get<is>(m_generators).next()...};
        m_next = *std::min_element(v.begin(), v.end());
    }

    //! @brief Returns the index of the subsequence generating the next event.
    template <size_t i>
    size_t next_sequence(std::index_sequence<i>) const {
        assert(std::get<i>(m_generators).next() == m_next);
        return i;
    }

    //! @brief Returns the index of the subsequence generating the next event.
    template <size_t i, size_t... is>
    size_t next_sequence(std::index_sequence<i, is...>) const {
        if (std::get<i>(m_generators).next() == m_next)
            return i;
        else
            return next_sequence(std::index_sequence<is...>{});
    }

    //! @brief Tuple of sequence generators.
    std::tuple<Ss...> m_generators;

    //! @brief The next event to come.
    times_t m_next;
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


}


}

#endif // FCPP_OPTION_SEQUENCE_H_
