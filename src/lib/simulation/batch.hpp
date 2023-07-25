// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file batch.hpp
 * @brief Helper functions for running a batch of simulations.
 */

#ifndef FCPP_SIMULATION_BATCH_H_
#define FCPP_SIMULATION_BATCH_H_

#include <cassert>

#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#ifdef FCPP_MPI
#include <mpi.h>
#endif

#include "lib/common/algorithm.hpp"
#include "lib/common/option.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/component/logger.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing tools for batch execution of FCPP simulations.
namespace batch {


//! @cond INTERNAL
namespace details {
    //! @brief Class wrapping a generating function with output type and size information.
    template <typename F, typename... Ts>
    class generator {
      public:
        //! @brief The tuple type that the function generates.
        using value_type = common::tagged_tuple<typename common::type_sequence<Ts...>::template slice<0, sizeof...(Ts)/2>, typename common::type_sequence<Ts...>::template slice<sizeof...(Ts)/2, sizeof...(Ts)>>;

        //! @brief Constructor setting up the class members.
        generator(F&& f, size_t core_size, size_t extra_size) : m_core_size(core_size), m_extra_size(extra_size), m_function(std::move(f)) {}

        /**
         * @brief Operator calling the wrapped function.
         *
         * @param t The tuple in which to store the values generated by the function.
         * @param i The index of the element to be generated.
         * @return  A boolean telling whether the given index has to be included (true) or skipped (false).
         */
        template <typename T>
        inline bool operator()(T& t, size_t i) const {
            return m_function(t, i);
        }

        //! @brief Returns the size of the core sequence that has to be expanded with every other value.
        inline size_t core_size() const {
            return m_core_size;
        }

        //! @brief Returns the size of the extra sequence that should be expanded only with core values.
        inline size_t extra_size() const {
            return m_extra_size;
        }

      private:
        //! @brief The size of the core sequence that has to be expanded with every other value.
        const size_t m_core_size;
        //! @brief The size of the extra sequence that should be expanded only with core values.
        const size_t m_extra_size;
        //! @brief The wrapped function.
        const F m_function;
    };

    //! @brief Wraps a generating function with output type and size information.
    template <typename... Ts, typename F>
    inline generator<F, Ts...> make_generator(F&& f, size_t core_size, size_t extra_size) {
        return {std::move(f), core_size, extra_size};
    }
}
//! @endcond


//! @brief Functor generating a single tuple with several constants.
template <typename... Ss, typename... Ts>
auto constant(Ts const&... xs) {
    return details::make_generator<Ss..., Ts...>([=](auto& t, size_t){
        t = common::make_tagged_tuple<Ss...>(xs...);
        return true;
    }, 1, 0);
}

//! @brief Functor generating a sequence of given values of type `char const*`. [DEPRECATED]
template <typename S, typename... Ts>
auto literals(char const* s, Ts const&... xs) {
    static_assert(common::always_false<S>::value, "the batch::literals function has been deprecated and should not be used, use batch::list instead");
    return details::make_generator<S, std::string>([=](auto&, size_t){
        return false;
    }, 0, 0);
}

//! @brief Functor generating a sequence of given values (`char const*` are wrapped as `std::string`).
template<typename S, typename T, typename... Ts>
auto list(T&& x, Ts&&... xs) {
    using DT = std::decay_t<T>;
    using CT = std::conditional_t<std::is_same<DT, char const*>::value, std::string, DT>;
    std::array<CT, sizeof...(Ts)+1> values{x, xs...};
    return details::make_generator<S, CT>([=](auto& t, size_t i){
        common::get<S>(t) = values[i];
        return true;
    }, values.size(), 0);
}

/**
 * @brief Functor generating a sequence of given core and extra values.
 *
 * Core values are expanded with value produced by the other generators.
 * Extra values are expanded only with core values produced by other generators.
 */
template<typename S, typename T>
auto double_list(std::vector<T> core, std::vector<T> extra) {
    for (T const& x : extra) for (T const& y : core)
        assert(x != y && "error: arguments to double_list have non-empty intersection");
    return details::make_generator<S, T>([=](auto& t, size_t i){
        common::get<S>(t) = i < core.size() ? core[i] : extra[i - core.size()];
        return true;
    }, core.size(), extra.size());
}

//! @brief Functor generating an arithmetic sequence of values (inclusive range).
template <typename S, typename T>
auto arithmetic(T min, T max, T step) {
    return details::make_generator<S, T>([=](auto& t, size_t i){
        common::get<S>(t) = min + i * step;
        return true;
    }, (max-min)/step + 1, 0);
}

/**
 * @brief Functor generating an arithmetic sequence (inclusive) with a default value.
 *
 * Only the default value is used when other defaulted parameters are set to extra values.
 */
template <typename S, typename T>
auto arithmetic(T min, T max, T step, T def) {
    size_t id = (def - min) / step;
    bool defextra = def != (min + id * step);
    return details::make_generator<S, T>([=](auto& t, size_t i){
        common::get<S>(t) = i == 0 ? def : defextra or i <= id ? min + (i-1) * step : min + i * step;
        return true;
    }, 1, (max-min)/step + defextra);
}

/**
 * @brief Functor generating an arithmetic sequence (inclusive) with a default range and extra range.
 *
 * Only the range `defmin...defmax` is used when other defaulted parameters are set to extra values.
 * The range `min...max` is used only with core values from other defaulted parameters.
 * The two ranges are assumed not to overlap.
 */
template <typename S, typename T>
auto arithmetic(T min, T max, T step, T defmin, T defmax) {
    size_t core_size = (defmax-defmin)/step + 1;
    return details::make_generator<S, T>([=](auto& t, size_t i){
        common::get<S>(t) = i < core_size ? defmin + i*step : min + (i-core_size) * step;
        return true;
    }, core_size, (max-min)/step + 1);
}

//! @brief Functor generating a geometric sequence of values (inclusive range).
template <typename S, typename T>
auto geometric(T min, T max, T step) {
    std::vector<T> v = {min};
    while (v.back() * step <= max) v.push_back(v.back() * step);
    return details::make_generator<S, T>([=](auto& t, size_t i){
        common::get<S>(t) = v[i];
        return true;
    }, v.size(), 0);
}

/**
 * @brief Functor generating a geometric sequence (inclusive) with a default value.
 *
 * Only the default value is used when other defaulted parameters are set to extra values.
 */
template <typename S, typename T>
auto geometric(T min, T max, T step, T def) {
    std::vector<T> v = {def};
    if (def != min) v.push_back(min);
    while (v.back() * step <= max) {
        if (v.size() > 1 and v.back() == def) v.back() *= step;
        else v.push_back(v.back() * step);
    }
    if (v.size() > 1 and v.back() == def) v.pop_back();
    return details::make_generator<S, T>([=](auto& t, size_t i){
        common::get<S>(t) = v[i];
        return true;
    }, 1, v.size() - 1);
}

/**
 * @brief Functor generating an geometric sequence (inclusive) with a default range and extra range.
 *
 * Only the range `defmin...defmax` is used when other defaulted parameters are set to extra values.
 * The range `min...max` is used only with core values from other defaulted parameters.
 * The two ranges are assumed not to overlap.
 */
template <typename S, typename T>
auto geometric(T min, T max, T step, T defmin, T defmax) {
    std::vector<T> v = {defmin};
    while (v.back() * step <= defmax) v.push_back(v.back() * step);
    size_t core_size = v.size();
    v.push_back(min);
    while (v.back() * step <= max) v.push_back(v.back() * step);
    return details::make_generator<S, T>([=](auto& t, size_t i){
        common::get<S>(t) = v[i];
        return true;
    }, core_size, v.size() - core_size);
}

/**
 * @brief Functor generating a recursively defined sequence.
 *
 * The recursive definition is given from three arguments:
 * - the list index `i` to be generated;
 * - the value previously generated `prev`;
 * - a \ref common::tagged_tuple "tagged_tuple" `tup` of parameters.
 * The recursive definition returns a `common::option<T>`, so that
 * `return {}` stops the recursion while `return v` provides a new item on the list.
 *
 * @param init A initialising value, to be fed to `f` for generating the first element.
 * @param f A function with signature `common::option<T>(size_t i, T prev, auto const& tup)`.
 */
template <typename S, typename T, typename F>
auto recursive(T init, F&& f) {
    std::vector<T> v;
    T prev = init;
    for (size_t i = 0; ; ++i) {
        common::option<T> r = f(i, prev);
        if (r.empty()) break;
        prev = r;
        v.push_back(r);
    }
    return details::make_generator<S, T>([=](auto& t, size_t i){
        common::get<S>(t) = v[i];
        return true;
    }, v.size(), 0);
}


//! @brief Functor generating a single tuple calculated from other values according to a given function.
template <typename S, typename T, typename F>
auto formula(F&& f) {
    return details::make_generator<S, T>([=](auto& t, size_t){
        common::get<S>(t) = f(t);
        return true;
    }, 1, 0);
}

//! @brief Functor generating a single tuple calculated as a representation of previously generated values.
template <typename S>
auto stringify(std::string prefix = "", std::string suffix = "") {
    return details::make_generator<S, std::string>([=](auto& t, size_t){
        using T = std::decay_t<decltype(t)>;
        constexpr size_t idx = T::tags::template find<S>;
        using R = common::tagged_tuple<typename T::tags::template slice<0, idx>, typename T::types::template slice<0, idx>>;
        common::get<S>(t) = prefix;
        R r = t;
        std::stringstream s;
        if (prefix != "")
            s << prefix << "_";
        r.print(s, common::underscore_tuple);
        if (suffix != "")
            s << "." << suffix;
        common::get<S>(t) = s.str();
        return true;
    }, 1, 0);
}

//! @brief Functor filtering out values from a sequence that match a given predicate.
template <typename F>
auto filter(F&& f) {
    return details::make_generator<>([=](auto& t, size_t){
        return not f(t);
    }, 1, 0);
}


/**
 * @brief Class generating a sequence of tagged tuples from generators.
 *
 * @tparam Gs The sequence of generator types for individual tags in the tuples, to be combined together.
 */
template <typename... Gs>
class tagged_tuple_sequence;

//! @brief Class generating a sequence of tagged tuples from generators (empty overload).
template <>
class tagged_tuple_sequence<> {
  public:
    //! @brief The tuple type that the class generates.
    using value_type = common::tagged_tuple_t<>;

    //! @brief Constructor setting up individual generators.
    tagged_tuple_sequence() {}

    //! @brief Returns the total size of the sequence generated (including filtered out values).
    inline size_t size() const {
        return 1;
    }

    //! @brief Reduces the generator to a subsequence.
    inline void slice(size_t start, size_t end, size_t stride = 1) {}

    //! @brief Function testing presence of an item of the sequence.
    inline bool count(size_t i) const {
        return i == 0;
    }

    //! @brief Function generating an item of the sequence.
    inline value_type operator[](size_t) const {
        return {};
    }

    /**
     * @brief Function generating and testing presence of an item of the sequence.
     *
     * @param[out] t The tuple in which to store the item generated.
     * @param[in]  i The index of the item to be generated.
     * @return     A boolean telling whether the given index has to be included (true) or skipped (false).
     */
    template <typename T>
    inline bool assign(T& t, size_t i) const {
        return true;
    }

  protected:
    //! @brief Returns the size of the core sequence that has to be expanded with every other value.
    inline size_t core_size() const {
        return 1;
    }

    //! @brief Returns the size of the extra sequence that should be expanded only with core values.
    inline size_t extra_size() const {
        return 0;
    }
};

//! @brief Class generating a sequence of tagged tuples from generators (recursive overload).
template <typename G, typename... Gs>
class tagged_tuple_sequence<G, Gs...> : public tagged_tuple_sequence<Gs...> {
  public:
    //! @brief The tuple type that the class generates.
    using value_type = common::tagged_tuple_cat<typename G::value_type, typename tagged_tuple_sequence<Gs...>::value_type>;

    //! @brief Constructor setting up individual generators.
    tagged_tuple_sequence(G&& g, Gs&&... gs) :
        tagged_tuple_sequence<Gs...>(std::move(gs)...),
        m_core_extra_size(g.core_size() * tagged_tuple_sequence<Gs...>::extra_size()),
        m_extra_core_size(g.extra_size() * tagged_tuple_sequence<Gs...>::core_size()),
        m_core_size(g.core_size() * tagged_tuple_sequence<Gs...>::core_size()),
        m_extra_size(m_core_extra_size + m_extra_core_size),
        m_size(m_core_size + m_extra_size),
        m_offset(0),
        m_stride(1),
        m_generator(std::move(g)) {}

    //! @brief Returns the total size of the sequence generated (including filtered out values).
    inline size_t size() const {
        return m_size;
    }

    //! @brief Reduces the generator to a subsequence.
    void slice(size_t start, size_t end, size_t stride = 1) {
        m_size = m_core_size + m_extra_size;
        m_offset = start;
        m_stride = stride;
        m_size = (std::min(end, m_size) - start + stride - 1) / stride;
    }

    //! @brief Function testing presence of an item of the sequence.
    bool count(size_t i) const {
        if (i >= m_size) return false;
        value_type t;
        return assign(t, i);
    }

    //! @brief Function generating an item of the sequence.
    value_type operator[](size_t i) const {
        value_type t;
        assign(t, i);
        return t;
    }

    /**
     * @brief Function generating and testing presence of an item of the sequence.
     *
     * @param[out] t The tuple in which to store the item generated.
     * @param[in]  i The index of the item to be generated.
     * @return     A boolean telling whether the given index has to be included (true) or skipped (false).
     */
    template <typename T>
    bool assign(T& t, size_t i) const {
        i = m_offset + m_stride * i;
        if (i < m_core_size) {
            if (not m_generator(t, i / tagged_tuple_sequence<Gs...>::core_size())) return false;
            return tagged_tuple_sequence<Gs...>::assign(t, i % tagged_tuple_sequence<Gs...>::core_size());
        }
        i -= m_core_size;
        if (i < m_core_extra_size) {
            if (not m_generator(t, i / tagged_tuple_sequence<Gs...>::extra_size())) return false;
            return tagged_tuple_sequence<Gs...>::assign(t, i % tagged_tuple_sequence<Gs...>::extra_size() + tagged_tuple_sequence<Gs...>::core_size());
        }
        i -= m_core_extra_size;
        if (not m_generator(t, i / tagged_tuple_sequence<Gs...>::core_size() + m_generator.core_size())) return false;
        return tagged_tuple_sequence<Gs...>::assign(t, i % tagged_tuple_sequence<Gs...>::core_size());
    }

  protected:
    //! @brief Returns the size of the core sequence that has to be expanded with every other value.
    inline size_t core_size() const {
        return m_core_size;
    }

    //! @brief Returns the size of the extra sequence that should be expanded only with core values.
    inline size_t extra_size() const {
        return m_extra_size;
    }

  private:
    //! @brief The size of the sequence that is core for the first generator and extra for others.
    const size_t m_core_extra_size;
    //! @brief The size of the sequence that is extra for the first generator and core for others.
    const size_t m_extra_core_size;
    //! @brief The size of the core sequence that has to be expanded with every other value.
    const size_t m_core_size;
    //! @brief The size of the extra sequence that should be expanded only with core values.
    const size_t m_extra_size;
    //! @brief The total size of the sequence generated (including filtered out values).
    size_t m_size;
    //! @brief The offset to the first element of the sequence.
    size_t m_offset;
    //! @brief The step between consecutive elements of the sequence.
    size_t m_stride;
    //! @brief The first generator.
    const G m_generator;
};

//! @brief Produces a generator of a sequence of tagged tuples, according to provided generators for individual tags.
template <typename... Gs>
inline auto make_tagged_tuple_sequence(Gs&&... gs) {
    return tagged_tuple_sequence<Gs...>(std::move(gs)...);
}


/**
 * @brief Class concatenating multiple tagged tuple sequences.
 *
 * All sequences are assumed to have the same value type (modulo permutation).
 *
 * @tparam Ss The tagged tuple sequences.
 */
template <typename... Ss>
class tagged_tuple_sequences;

//! @brief Class concatenating multiple tagged tuple sequences (empty overload).
template <>
class tagged_tuple_sequences<> {
  public:
    //! @brief The tuple type that the class generates.
    using value_type = common::tagged_tuple_t<>;

    //! @brief Constructor setting up individual generators.
    tagged_tuple_sequences() {}

    //! @brief Returns the total size of the sequence generated (including filtered out values).
    inline size_t size() const {
        return 0;
    }

    //! @brief Reduces the generator to a subsequence.
    inline void slice(size_t start, size_t end, size_t stride = 1) {}

    //! @brief Function testing presence of an item of the sequence.
    inline bool count(size_t i) const {
        return false;
    }

    //! @brief Function generating an item of the sequence.
    inline value_type operator[](size_t) const {
        return {};
    }

    /**
     * @brief Function generating and testing presence of an item of the sequence.
     *
     * @param[out] t The tuple in which to store the item generated.
     * @param[in]  i The index of the item to be generated.
     * @return     A boolean telling whether the given index has to be included (true) or skipped (false).
     */
    template <typename T>
    inline bool assign(T& t, size_t i) const {
        return true;
    }
};

//! @brief Class concatenating multiple tagged tuple sequences (recursive overload).
template <typename S, typename... Ss>
class tagged_tuple_sequences<S, Ss...> : public tagged_tuple_sequences<Ss...> {
  public:
    //! @brief The tuple type that the class generates.
    using value_type = typename S::value_type;

    //! @brief Constructor setting up individual generators.
    tagged_tuple_sequences(S const& s, Ss const&... ss) :
        tagged_tuple_sequences<Ss...>(ss...),
        m_total_size(s.size() + tagged_tuple_sequences<Ss...>::size()),
        m_size(m_total_size),
        m_offset(0),
        m_stride(1),
        m_sequence(s) {}

    //! @brief Returns the total size of the sequence generated (including filtered out values).
    inline size_t size() const {
        return m_size;
    }

    //! @brief Reduces the generator to a subsequence.
    void slice(size_t start, size_t end, size_t stride = 1) {
        m_offset = start;
        m_stride = stride;
        m_size = (std::min(end, m_total_size) - start + stride - 1) / stride;
    }

    //! @brief Function testing presence of an item of the sequence.
    bool count(size_t i) const {
        if (i >= m_size) return false;
        value_type t;
        return assign(t, i);
    }

    //! @brief Function generating an item of the sequence.
    value_type operator[](size_t i) const {
        value_type t;
        assign(t, i);
        return t;
    }

    /**
     * @brief Function generating and testing presence of an item of the sequence.
     *
     * @param[out] t The tuple in which to store the item generated.
     * @param[in]  i The index of the item to be generated.
     * @return     A boolean telling whether the given index has to be included (true) or skipped (false).
     */
    template <typename T>
    inline bool assign(T& t, size_t i) const {
        i = m_offset + m_stride * i;
        if (i < m_sequence.size()) return m_sequence.assign(t, i);
        return tagged_tuple_sequences<Ss...>::assign(t, i - m_sequence.size());
    }

  private:
    //! @brief The total size of the sequence generated (including filtered out values).
    const size_t m_total_size;
    //! @brief The size of the reduced sequence after slicing.
    size_t m_size;
    //! @brief The offset to the first element of the sequence.
    size_t m_offset;
    //! @brief The step between consecutive elements of the sequence.
    size_t m_stride;
    //! @brief The first sequence.
    const S m_sequence;
};

//! @brief Produces a concatenation of multiple tagged tuple sequences.
template <typename... Ss>
inline auto make_tagged_tuple_sequences(Ss const&... ss) {
    return tagged_tuple_sequences<Ss...>(ss...);
}


//! @brief Tag identifying alternative template options for a network type (see \ref option_combine).
template <typename... Ts>
struct options {};

//! @cond INTERNAL
namespace details {
    //! @brief Converts a type into a type sequence.
    //! @{
    template <typename T>
    struct to_type_sequence {
        using type = common::type_sequence<T>;
    };
    template <typename... Ts>
    struct to_type_sequence<common::type_sequence<Ts...>> {
        using type = common::type_sequence<Ts...>;
    };
    template <typename T>
    using to_type_sequence_t = typename to_type_sequence<T>::type;
    //! @}

    //! @brief Manages options and non-options types.
    //! @{
    template <typename T>
    struct option_decay {
        using type = common::type_sequence<to_type_sequence_t<T>>;
    };
    template <typename... Ts>
    struct option_decay<options<Ts...>> {
        using type = common::type_sequence<to_type_sequence_t<Ts>...>;
    };
    template <typename T>
    using option_decay_t = typename option_decay<T>::type;
    //! @}

    //! @brief Maps a template to a sequence of options.
    //! @{
    template <template <class...> class C, typename T>
    struct map_template;
    template <template <class...> class C, typename... Ts>
    struct map_template<C, common::type_sequence<Ts...>> {
        using type = common::type_sequence<common::apply_templates<Ts, C>...>;
    };
    template <template <class...> class C, typename T>
    using map_template_t = typename map_template<C,T>::type;
    //! @}
}
//! @endcond

/**
 * @brief Instantiates a template for every possible combination from a given sequence of options.
 *
 * Fixed options can be given individually or grouped as type sequences.
 * Alternative options to be expanded in every possible combination have to be defined through the \ref options tag class.
 */
template <template <class...> class C, typename... Ts>
using option_combine = details::map_template_t<C, common::type_product<details::option_decay_t<Ts>...>>;


//!  @brief Does not run a series of experiments (no execution policy and parameters)
template <typename T>
void run(T) {}

//!  @brief Does not run a series of experiments (single network type, with execution policy and no parameters)
template <typename T, typename exec_t>
common::ifn_class_template<tagged_tuple_sequence, exec_t, common::ifn_class_template<common::type_sequence, T>>
run(T, exec_t e) {}

/**
 *  @brief Runs a series of experiments (single network type, with execution policy and parameters)
 *
 * @tparam T The network type to be run.
 * @param e An execution policy (see \ref common::tags::sequential_execution "sequential_execution", \ref common::tags::parallel_execution "parallel_execution", \ref common::tags::general_execution "general_execution", \ref common::tags::dynamic_execution "dynamic_execution").
 * @param v  A tagged tuple sequence, used to initialise the various runs.
 * @param vs Further tagged tuple sequences.
 */
template <typename T, typename exec_t, typename... Gs, typename... Ss>
common::ifn_class_template<tagged_tuple_sequence, exec_t, common::ifn_class_template<common::type_sequence, T>>
run(T, exec_t e, tagged_tuple_sequence<Gs...> const& v, Ss const&... vs) {
    using init_tuple = typename tagged_tuple_sequence<Gs...>::value_type;
    auto seq = make_tagged_tuple_sequences(v, vs...);
    std::cerr << common::type_name<T>() << ": running " << seq.size() << " simulations..." << std::flush;
    size_t p = 0;
    common::parallel_for(e, seq.size(), [&](size_t i, size_t t){
        if (t == 0 and i*100/seq.size() > p) {
            p = i*100/seq.size();
            std::cerr << p << "%..." << std::flush;
        }
        init_tuple tup;
        if (seq.assign(tup, i)) {
            typename T::net network{tup};
            network.run();
        }
    });
    std::cerr << "done." << std::endl;
}

//!  @brief Does not run a series of experiments (no network types, with execution policy)
template <typename exec_t, typename... Ss>
common::ifn_class_template<tagged_tuple_sequence, exec_t>
run(common::type_sequence<>, exec_t, Ss const&...) {}

//!  @brief Runs a series of experiments (multiple network types, with execution policy)
template <typename T, typename... Ts, typename exec_t, typename... Ss>
common::ifn_class_template<tagged_tuple_sequence, exec_t>
run(common::type_sequence<T, Ts...>, exec_t e, Ss const&... vs) {
    run(T{}, e, vs...);
    run(common::type_sequence<Ts...>{}, e, vs...);
}

//!  @brief Runs a series of experiments (single network type, assuming dynamic execution policy).
template <typename T, typename... Gs, typename... Ss>
common::ifn_class_template<common::type_sequence, T>
run(T x, tagged_tuple_sequence<Gs...> const& v, Ss const&... vs) {
    run(x, common::tags::dynamic_execution{}, v, vs...);
}

//!  @brief Runs a series of experiments (multiple network types, assuming dynamic execution policy).
template <typename T, typename... Ts, typename... Gs, typename... Ss>
void run(common::type_sequence<T, Ts...> x, tagged_tuple_sequence<Gs...> const& v, Ss const&... vs) {
    run(x, common::tags::dynamic_execution{}, v, vs...);
}

#ifdef FCPP_MPI
template <typename P>
void aggregate_plots(P& p, int n_procs, int rank) {
	int rank_master = 0;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == rank_master) {
        int size;
        int max_size = 128 * 1024 * 1024;
        char* buf = new char[max_size];
        MPI_Status status;
        for (int i = 1; i < n_procs; ++i) {
            P q;
            MPI_Recv(buf, max_size, MPI_CHAR, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_CHAR, &size);
            common::isstream is({buf, buf+size});
            is >> q;
            p += q;
        }
        delete [] buf;
    } else {
        common::osstream os;
        os << p;
        MPI_Send(os.data().data(), os.data().size(), MPI_CHAR, rank_master, 1, MPI_COMM_WORLD);
        p = P{};
    }
}

//! @brief Running a single MPI component combination (static splitting across nodes).
template <typename T, typename exec_t, typename... Gs>
common::ifn_class_template<tagged_tuple_sequence, exec_t, common::ifn_class_template<common::type_sequence, T>>
mpi_run(T x, exec_t e, tagged_tuple_sequence<Gs...> v) {
    int provided, initialized, rank, n_procs;
    MPI_Initialized(&initialized);
    if (not initialized) {
        MPI_Init_thread(NULL, NULL, MPI_THREAD_FUNNELED, &provided);
        assert(provided == MPI_THREAD_MULTIPLE);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    auto p = common::get_or<component::tags::plotter>(v[0], nullptr);
    v.slice(rank, v.size(), n_procs);
    run(x, e, v);
    if (p != nullptr)
	    aggregate_plots(*p, n_procs, rank);
    if (not initialized) MPI_Finalize();
}

//! @brief Running a single MPI component combination (dynamic splitting across nodes).
template <typename T, typename exec_t, typename... Gs>
common::ifn_class_template<tagged_tuple_sequence, exec_t, common::ifn_class_template<common::type_sequence, T>>
mpi_dynamic_run(T x, exec_t e, tagged_tuple_sequence<Gs...> v) {
    constexpr int dynamic_chunks_per_node = 4; // to regulate, but probably not much than this
    // number of simulations per proc that are pre-assigned at start
    int provided, initialized, rank, n_procs;
    MPI_Initialized(&initialized);
    if (not initialized) {
        MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
        assert(provided == MPI_THREAD_MULTIPLE);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int start = dynamic_chunks_per_node * de.size * n_procs;
    start = max(de.size, (v.size() - start + n_procs/2) / n_procs);
    auto p = common::get_or<component::tags::plotter>(v[0], nullptr);
    int maxi = (v.size() - start*n_procs + de.size/2) / de.size;

    std::thread manager;
    if (rank == 0) manager = std::thread([=](){
        int buf;
        int pi[n_procs];
        MPI_Request req;
        MPI_Status status;
        for (int idx = 0; idx < maxi + n_procs; ++idx) {
            MPI_Recv(&buf, 0, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            int source = status.MPI_SOURCE;
            pi[source] = idx;
            MPI_Send(pi + source, 1, MPI_INT, source, 0, MPI_COMM_WORLD);
        }
    });
    v.slice(rank, start*n_procs, n_procs);
    run(x, e, v);
    int idx;
    while (true) {
        MPI_Send(&idx, 0, MPI_INT, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(&idx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        if (idx >= maxi) break;
        v.slice(start*n_procs + idx, -1, maxi);
        run(x, e, v);
    }
    if (rank == 0) manager.join();
    if (p != nullptr)
        aggregate_plots(*p, n_procs, rank);
    if (not initialized) MPI_Finalize();
}
//! @}
#else
template <typename T, typename exec_t, typename... Gs>
common::ifn_class_template<tagged_tuple_sequence, exec_t, common::ifn_class_template<common::type_sequence, T>>
mpi_run(T x, exec_t e, tagged_tuple_sequence<Gs...> v) {}
template <typename T, typename exec_t, typename... Gs>
common::ifn_class_template<tagged_tuple_sequence, exec_t, common::ifn_class_template<common::type_sequence, T>>
mpi_dynamic_run(T x, exec_t e, tagged_tuple_sequence<Gs...> v) {}
#endif


}


}

#endif // FCPP_SIMULATION_BATCH_H_
