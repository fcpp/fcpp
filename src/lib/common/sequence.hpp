// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file sequence.hpp
 * @brief Collection of generators of increasing sequences. Contrary to random distributions, sequence generators are stateful, that is, the generation of the next element depend on the previous elements generated (and are generated in increasing order).
 */

#ifndef FCPP_COMMON_SEQUENCE_H_
#define FCPP_COMMON_SEQUENCE_H_

#include <algorithm>
#include <array>
#include <limits>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/distribution.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing classes for random data generation.
 */
namespace random {


//! @brief Empty generator for a sequence of no events.
struct sequence_never {
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    sequence_never(G&&) {}

    //! @brief Default constructor.
    template <typename G, typename S, typename T>
    sequence_never(G&&, const common::tagged_tuple<S,T>&) {}

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


/**
 * @brief Generator of a series of contemporary events.
 * @param D Distribution generating the time of the events.
 * @param n Number of events.
 * @param same Whether the same event should be produced (defaults to true).
 */
//! @{
//! @brief General form.
template <typename D, size_t n, bool same = true>
class sequence_multiple;

//! @brief Case for identical events.
template <typename D, size_t n>
class sequence_multiple<D, n, true> {
    static_assert(std::is_same<typename D::type, times_t>::value, "the distribution D must generate a times_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    sequence_multiple(G&& g) : t(details::call_distr<D>(g)) {}

    //! @brief Default constructor.
    template <typename G, typename S, typename T>
    sequence_multiple(G&& g, const common::tagged_tuple<S,T>& tup) : t(details::call_distr<D>(g,tup)) {}

    //! @brief Returns next event, without stepping over.
    times_t next() const {
        return (i < n) ? t : TIME_MAX;
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
    //! @brief The time of the events.
    times_t t;

    //! @brief Number of calls to next so far.
    size_t i = 0;
};

//! @brief Case for possibly different events.
template <typename D, size_t n>
class sequence_multiple<D, n, false> {
    static_assert(std::is_same<typename D::type, times_t>::value, "the distribution D must generate a times_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    sequence_multiple(G&& g) : sequence_multiple(g, D{g}) {}

    //! @brief Default constructor.
    template <typename G, typename S, typename T>
    sequence_multiple(G&& g, const common::tagged_tuple<S,T>& tup) : sequence_multiple(g, D{g,tup}) {}

    //! @brief Returns next event, without stepping over.
    times_t next() const {
        return (i < n) ? pending[i] : TIME_MAX;
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
    //! @brief The time of the events.
    std::array<times_t, n> pending;

    //! @brief Number of calls to next so far.
    size_t i = 0;

    //! @brief Auxiliary constructor.
    template <typename G>
    sequence_multiple(G&& g, D&& distr) {
        for (size_t j=0; j<n; ++j) pending[j] = distr(g);
        std::sort(pending.begin(), pending.end());
    }
};
//! @}


/**
 * @brief Generator of a series of events at given times.
 * @param Ds Distributions generating the time of the events.
 */
template <typename... Ds>
class sequence_list {
    static_assert(common::all_true<std::is_same<typename Ds::type, times_t>::value...>, "the distributions Ds must generate a times_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    sequence_list(G&& g) : pending({details::call_distr<Ds>(g)...}) {
        std::sort(pending.begin(), pending.end());
    }

    //! @brief Default constructor.
    template <typename G, typename S, typename T>
    sequence_list(G&& g, const common::tagged_tuple<S,T>& tup) : pending({details::call_distr<Ds>(g,tup)...}) {
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
 * @brief Generator of a series of events at given times.
 * The sooner terminating condition between E and N is selected.
 * @param S Distribution for the first event.
 * @param P Distribution regulating the period.
 * @param E Distribution for the last event.
 * @param N Distribution for the number of events to be generated.
 */
template <typename S, typename P = S, typename E = sequence_never, typename N = constant_distribution<size_t, std::numeric_limits<intmax_t>::max()>>
class sequence_periodic {
    static_assert(std::is_same<typename S::type, times_t>::value, "the distribution S must generate a times_t value");
    static_assert(std::is_same<typename P::type, times_t>::value, "the distribution P must generate a times_t value");
    static_assert(std::is_same<typename E::type, times_t>::value, "the distribution E must generate a times_t value");
    static_assert(std::is_same<typename N::type, size_t>::value, "the distribution N must generate a size_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    sequence_periodic(G&& g) : dp(g) {
        n  = details::call_distr<N>(g);
        te = details::call_distr<E>(g);
        t  = details::call_distr<S>(g);
    }

    //! @brief Default constructor.
    template <typename G, typename U, typename T>
    sequence_periodic(G&& g, const common::tagged_tuple<U,T>& tup) : dp(g,tup) {
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
class sequence_merge {
    static_assert(common::all_true<std::is_same<typename Ss::type, times_t>::value...>, "the generators Ss must generate a times_t value");

  public:
    //! @brief The type of results generated.
    using type = times_t;

    //! @brief Default constructor.
    template <typename G>
    sequence_merge(G&& g) : m_generators{details::arg_expander<Ss>(g)...} {
        set_next(std::make_index_sequence<sizeof...(Ss)>{});
    }

    //! @brief Default constructor.
    template <typename G, typename S, typename T>
    sequence_merge(G&& g, const common::tagged_tuple<S,T>& tup) : m_generators{{details::arg_expander<Ss>(g),tup}...} {
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
class sequence_merge<S> : public S {
  public:
    using S::S;

    //! @brief Returns the index of the subsequence generating the next event.
    size_t next_sequence() const {
        return 0;
    }
};

//! @brief Optimisation for no sequences.
template <>
class sequence_merge<> : public sequence_never {
  public:
    using sequence_never::sequence_never;

    //! @brief Returns the index of the subsequence generating the next event.
    size_t next_sequence() const {
        assert(false);
        return -1;
    }
};


//! @brief Merges multiple sequences wrapped in a type sequence.
template <typename T>
using sequence_merge_t = common::apply_templates<T, sequence_merge>;


}


}

#endif // FCPP_COMMON_SEQUENCE_H_
