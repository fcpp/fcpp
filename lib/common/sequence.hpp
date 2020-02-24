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
//{@
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
//@}


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
template <typename S, typename P = S, typename E = sequence_never, typename N = constant_distribution<size_t, std::numeric_limits<size_t>::max()>>
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
        return (i < n and t < te) ? t : TIME_MAX;
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


}


}

#endif // FCPP_COMMON_SEQUENCE_H_
