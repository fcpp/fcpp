// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file batch.hpp
 * @brief Helper functions for running a batch of simulations.
 */

#ifndef FCPP_SIMULATION_BATCH_H_
#define FCPP_SIMULATION_BATCH_H_

#include <algorithm>
#include <array>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "lib/common/algorithm.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing tools for batch execution of FCPP simulations.
namespace batch {


//! @brief Namespace of tags to be used for batch description.
namespace tags {
    //! @brief Tag associating to a filtering function.
    struct filter {};
}


//! @brief Functor generating a list of given values (general case).
template<typename T, typename... Ts>
auto list(T const& x, Ts const&... xs) {
    return [=](auto const&){
        return std::array<T, sizeof...(Ts)+1>{x, xs...};
    };
}

//! @brief Functor generating a list of given values (string case).
template <typename... Ts>
auto list(char const* s, Ts const&... xs) {
    return list(std::string(s), xs...);
}

//! @brief Functor generating a list of values following an arithmetic sequence.
template <typename T>
auto arithmetic(T min, T max, T step) {
    return [=](auto const&){
        std::vector<T> v;
        v.push_back(min);
        while (v.back() + step <= max) v.push_back(v.back() + step);
        return v;
    };
}

//! @brief Functor generating a list of values following a geometric sequence.
template <typename T>
auto geometric(T min, T max, T step) {
    return [=](auto const&){
        std::vector<T> v;
        v.push_back(min);
        while (v.back() * step <= max) v.push_back(v.back() * step);
        return v;
    };
}

//! @brief Class representing an optional value of type `T`.
template <typename T>
class option {
  public:
    //! @brief The underlying type.
    using type = T;
    
    //! @brief Constructor with no value.
    option() : m_val(), m_none(true) {}
    
    //! @brief Constructor with a value.
    option(T v) : m_val(v), m_none(false) {}
    
    //! @brief Value extraction (returns `T{}` if no value is contained)
    operator T() const {
        return m_val;
    }
    
    //! @brief Returns whether a value is present.
    bool none() const {
        return m_none;
    }
    
  private:
    //! @brief The stored value.
    T m_val;
    //! @brief Whether there is a value.
    bool m_none;
};

/**
 * @brief Functor generating a recursively defined list.
 *
 * The recursive definition is given from three arguments:
 * - the list index `i` to be generated;
 * - the value previously generated `prev`;
 * - a @ref tagged_tuple `tup` of parameters.
 * The recursive definition returns an `option<T>`, so that
 * `return {}` stops the recursion while `return v` provides a new item on the list.
 *
 * @param init A initialising value, to be fed to `f` for generating the first element.
 * @param f A function with signature `option<T>(size_t i, T prev, auto const& tup)`.
 */
template <typename T, typename F>
auto recursive(T init, F&& f) {
    return [=](auto const& x) {
        std::vector<T> v;
        T prev = init;
        for (size_t i = 0; ; ++i) {
            option<T> r = f(i, prev, x);
            if (r.none()) break;
            prev = r;
            v.push_back(r);
        }
        return v;
    };
}


//! @brief Functor generating a list comprising a single value according to a given function.
template <typename F>
auto formula(F&& f) {
    return [=](auto const& x) {
        using value_t = std::decay_t<decltype(f(x))>;
        return std::array<value_t, 1>{f(x)};
    };
}

//! @brief Functor generating a list comprising a single string value, representing the provided argument.
auto stringify(std::string prefix = "", std::string suffix = "") {
    return [=](auto const& x) {
        std::stringstream s;
        if (prefix != "")
            s << prefix << "_";
        x.print(s, common::underscore_tuple);
        if (suffix != "")
            s << "." << suffix;
        return std::array<std::string,1>{s.str()};
    };
}


//! @cond INTERNAL
namespace details {
    //! @brief Base case, forwarding a given tuple sequence.
    template <typename... Ss, typename... Us>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&& v) {
        return v;
    }
    //! @brief Enable mutual recursion.
    template <typename... Ss, typename... Us, typename gen_t, typename... Ts>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&&, tags::filter, gen_t&&, Ts&&...);
    //! @brief Inductive case, expanding a tuple sequence with an additional tag and generator.
    template <typename... Ss, typename... Us, typename tag_t, typename gen_t, typename... Ts>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&& v, tag_t, gen_t&& g, Ts&&... xs) {
        using value_t = std::decay_t<decltype(*g(v[0]).begin())>;
        std::vector<common::tagged_tuple<common::type_sequence<Ss..., tag_t>, common::type_sequence<Us..., value_t>>> w;
        for (size_t i = 0; i < v.size(); ++i) {
            for (value_t const& x : g(v[i])) {
                w.emplace_back(v[i]);
                common::get<tag_t>(w.back()) = x;
            }
        }
        return make_tagged_tuple_sequence(std::move(w), std::forward<Ts>(xs)...);
    }
    //! @brief Inductive case, filtering a tuple sequence according to a given predicate.
    template <typename... Ss, typename... Us, typename gen_t, typename... Ts>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&& v, tags::filter, gen_t&& g, Ts&&... xs) {
        v.resize(std::remove_if(v.begin(), v.end(), std::forward<gen_t>(g)) - v.begin());
        return make_tagged_tuple_sequence(std::move(v), std::forward<Ts>(xs)...);
    }
}
//! @endcond

/**
 * @brief Produces a sequence of tagged tuples, according to provided tags and generators.
 *
 * Tags and generators should be interleaved as arguments to this function.
 * If a tag is `tags::filter`, the following function is interpreted as a filter rather than
 * a generator, returning `true` on elements to be removed (as per `std::remove_if`).
 */
template <typename... Ts>
auto make_tagged_tuple_sequence(Ts&&... xs) {
    return details::make_tagged_tuple_sequence(std::vector<common::tagged_tuple_t<>>{1}, std::forward<Ts>(xs)...);
}


//! @cond INTERNAL
namespace details {
    //! @brief Base case, forwarding a single vector.
    template <typename T>
    std::vector<T> join_vectors(std::vector<T> v) {
        return v;
    }
    //! @brief Inductive case, joining a sequence of vectors.
    template <typename T, typename... Ts>
    std::vector<T> join_vectors(std::vector<T> v, std::vector<Ts>... vs) {
        auto w = join_vectors(vs...);
        v.insert(v.end(), w.begin(), w.end());
        return v;
    }
}
//! @endcond

/**
 * @brief Runs a series of experiments.
 *
 * @param T The combination of components to be tested.
 * @param e An execution policy (see @ref sequential_execution, @ref parallel_execution, @ref general_execution).
 * @param vs Sequences of tagged tuples, to be used to initialise the various runs.
 */
template <typename T, typename exec_t, typename... S, typename... U>
void run(exec_t e, std::vector<common::tagged_tuple<S,U>> const&... vs) {
    auto v = details::join_vectors(vs...);
    common::parallel_for(e, v.size(), [&](size_t i, size_t){
        typename T::net network{v[i]};
        network.run();
    });
}


}


}

#endif // FCPP_SIMULATION_BATCH_H_
