// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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
#include "lib/common/option.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing tools for batch execution of FCPP simulations.
namespace batch {


//! @cond INTERNAL
namespace details {
    //! @brief Tag associating to a filtering function.
    struct filter_tag {};

    //! @brief Tag associating to a injecting constants.
    struct constant_tag {};

    //! @brief Class wrapping a function with a tag.
    template <typename S, typename F>
    class functor {
      public:
        functor(F&& f) : m_function(std::move(f)) {}

        template <typename T>
        inline auto operator()(T const& x) const {
            return m_function(x);
        }

      private:
        F m_function;
    };

    //! @brief Wraps a function object with a tag.
    template <typename S, typename F>
    functor<S,F> make_functor(F&& f) {
        return std::move(f);
    }
}
//! @endcond


//! @brief Functor for appending a list of constants.
template <typename... Ss, typename... Ts>
auto constant(Ts const&... xs) {
    return details::make_functor<details::constant_tag>([=](auto const&){
        return common::make_tagged_tuple<Ss...>(xs...);
    });
}

//! @brief Functor generating a list of given values.
template<typename S, typename T, typename... Ts>
auto list(T const& x, Ts const&... xs) {
    return details::make_functor<S>([=](auto const&){
        return std::array<T, sizeof...(Ts)+1>{x, xs...};
    });
}

//! @brief Functor generating a list of given literal values.
template <typename S, typename... Ts>
auto literals(char const* s, Ts const&... xs) {
    return list<S>(std::string(s), xs...);
}

//! @brief Functor generating a list of values following an arithmetic sequence.
template <typename S, typename T>
auto arithmetic(T min, T max, T step) {
    return details::make_functor<S>([=](auto const&){
        std::vector<T> v;
        v.push_back(min);
        while (v.back() + step <= max) v.push_back(v.back() + step);
        return v;
    });
}

//! @brief Functor generating a list of values following a geometric sequence.
template <typename S, typename T>
auto geometric(T min, T max, T step) {
    return details::make_functor<S>([=](auto const&){
        std::vector<T> v;
        v.push_back(min);
        while (v.back() * step <= max) v.push_back(v.back() * step);
        return v;
    });
}

/**
 * @brief Functor generating a recursively defined list.
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
    return details::make_functor<S>([=](auto const& x) {
        std::vector<T> v;
        T prev = init;
        for (size_t i = 0; ; ++i) {
            common::option<T> r = f(i, prev, x);
            if (r.empty()) break;
            prev = r;
            v.push_back(r);
        }
        return v;
    });
}


//! @brief Functor generating a list comprising a single value according to a given function.
template <typename S, typename F>
auto formula(F&& f) {
    return details::make_functor<S>([=](auto const& x) {
        using value_t = std::decay_t<decltype(f(x))>;
        return std::array<value_t, 1>{f(x)};
    });
}

//! @brief Functor generating a list comprising a single string value, representing the provided argument.
template <typename S>
auto stringify(std::string prefix = "", std::string suffix = "") {
    return details::make_functor<S>([=](auto const& x) {
        std::stringstream s;
        if (prefix != "")
            s << prefix << "_";
        x.print(s, common::underscore_tuple);
        if (suffix != "")
            s << "." << suffix;
        return std::array<std::string,1>{s.str()};
    });
}

//! @brief Functor filtering a list by a given predicate.
template <typename F>
auto filter(F&& f) {
    return details::make_functor<details::filter_tag>(std::move(f));
}


//! @cond INTERNAL
namespace details {
    //! @brief Enable mutual recursion.
    template <typename... Ss, typename... Us, typename gen_t, typename... Ts>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&&, details::functor<details::filter_tag, gen_t> const&, Ts const&...);
    template <typename... Ss, typename... Us, typename gen_t, typename... Ts>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&&, details::functor<details::constant_tag, gen_t> const&, Ts const&...);

    //! @brief Base case, forwarding a given tuple sequence.
    template <typename... Ss, typename... Us>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&& v) {
        return std::move(v);
    }
    //! @brief Inductive case, expanding a tuple sequence with an additional tag and generator.
    template <typename... Ss, typename... Us, typename tag_t, typename gen_t, typename... Ts>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&& v, details::functor<tag_t, gen_t> const& g, Ts const&... xs) {
        using value_t = std::decay_t<decltype(*g(v[0]).begin())>;
        std::vector<common::tagged_tuple<common::type_sequence<Ss..., tag_t>, common::type_sequence<Us..., value_t>>> w;
        for (size_t i = 0; i < v.size(); ++i) {
            for (value_t const& x : g(v[i])) {
                w.emplace_back(v[i]);
                common::get<tag_t>(w.back()) = x;
            }
        }
        return make_tagged_tuple_sequence(std::move(w), xs...);
    }
    //! @brief Inductive case, filtering a tuple sequence according to a given predicate.
    template <typename... Ss, typename... Us, typename gen_t, typename... Ts>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&& v, details::functor<details::filter_tag, gen_t> const& g, Ts const&... xs) {
        v.resize(std::remove_if(v.begin(), v.end(), g) - v.begin());
        return make_tagged_tuple_sequence(std::move(v), xs...);
    }
    //! @brief Inductive case, adding constants to a tuple sequence.
    template <typename... Ss, typename... Us, typename gen_t, typename... Ts>
    auto make_tagged_tuple_sequence(std::vector<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>>&& v, details::functor<details::constant_tag, gen_t> const& g, Ts const&... xs) {
        using value_t = std::decay_t<decltype(g(nullptr))>;
        using combined_t = common::tagged_tuple_cat<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Us...>>, value_t>;
        std::vector<combined_t> w;
        value_t t = g(nullptr);
        for (size_t i = 0; i < v.size(); ++i) {
            w.emplace_back(v[i]);
            w.back() = t;
        }
        return make_tagged_tuple_sequence(std::move(w), xs...);
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
auto make_tagged_tuple_sequence(Ts const&... xs) {
    return details::make_tagged_tuple_sequence(std::vector<common::tagged_tuple_t<>>{1}, xs...);
}


//! @brief Tag identifying alternative options (see \ref option_combine).
template <typename... Ts>
struct options {};

//! @cond INTERNAL
namespace details {
    //! @brief Converts a type into a type sequence.
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

    //! @brief Manages options and non-options types.
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

    //! @brief Maps a template to a sequence of options.
    template <template <class...> class C, typename T>
    struct map_template;
    template <template <class...> class C, typename... Ts>
    struct map_template<C, common::type_sequence<Ts...>> {
        using type = common::type_sequence<common::apply_templates<Ts, C>...>;
    };
    template <template <class...> class C, typename T>
    using map_template_t = typename map_template<C,T>::type;
}
//! @endcond

/**
 * @brief Instantiates a template for every possible combination from a given sequence of options.
 *
 * Alternative options have to be defined through the \ref options tag class.
 * Non-alternative options can be also added as parameters.
 */
template <template <class...> class C, typename... Ts>
using option_combine = details::map_template_t<C, common::type_product<details::option_decay_t<Ts>...>>;


//! @cond INTERNAL
namespace details {
    //! @brief Converts a tagged tuple into a type sequence of type pairs.
    //! @{
    template <typename T>
    struct tt_paired;
    template <typename... Ss, typename... Ts>
    struct tt_paired<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Ts...>>> {
        using type = common::type_sequence<common::type_sequence<Ss,Ts>...>;
    };
    template <typename T>
    using tt_paired_t = typename tt_paired<std::decay_t<T>>::type;
    //! @}

    //! @brief Checks whether two tagged tuples have the same tags and types (possibly reordered).
    template <typename T, typename U>
    constexpr bool same_tuple = std::is_same<
        common::type_intersect<tt_paired_t<T>, tt_paired_t<U>>,
        tt_paired_t<T>
    >::value and std::is_same<
        common::type_intersect<tt_paired_t<U>, tt_paired_t<T>>,
        tt_paired_t<U>
    >::value;

    //! @brief Base case with no vectors.
    inline std::vector<common::tagged_tuple_t<>> join_vectors() {
        return std::vector<common::tagged_tuple_t<>>{1};
    }

    //! @brief Base case, forwarding a single vector.
    template <typename T>
    inline std::vector<T> join_vectors(std::vector<T> v) {
        return v;
    }
    //! @brief Inductive case, joining a sequence of vectors.
    template <typename T, typename... Ts>
    inline std::vector<T> join_vectors(std::vector<T> v, std::vector<Ts>... vs) {
        auto w = join_vectors(vs...);
        static_assert(same_tuple<decltype(w[0]),T>, "tagged tuple sequences of different types in the same batch run");
        v.insert(v.end(), w.begin(), w.end());
        return v;
    }
}
//! @endcond

/**
 * @brief Runs a series of experiments.
 *
 * @param T The combination(s) of components to be tested.
 * @param e An execution policy (see \ref common::tags::sequential_execution "sequential_execution", \ref common::tags::parallel_execution "parallel_execution", \ref common::tags::general_execution "general_execution", \ref common::tags::dynamic_execution "dynamic_execution").
 * @param vs Sequences of tagged tuples, to be used to initialise the various runs.
 */
//! @{
//! @brief Running a single component combination.
template <typename T, typename exec_t, typename... S, typename... U>
common::ifn_class_template<std::vector, exec_t>
run(T, exec_t e, std::vector<common::tagged_tuple<S,U>> const&... vs) {
    auto v = details::join_vectors(vs...);
    common::parallel_for(e, v.size(), [&](size_t i, size_t){
        typename T::net network{v[i]};
        network.run();
    });
}

//! @brief No running, given an empty sequence of component combinations.
template <typename exec_t, typename... S, typename... U>
void run(common::type_sequence<>, exec_t, std::vector<common::tagged_tuple<S,U>> const&...) {}

//! @brief Running a non-empty sequence of component combinations.
template <typename T, typename... Ts, typename exec_t, typename... S, typename... U>
common::ifn_class_template<std::vector, exec_t>
run(common::type_sequence<T, Ts...>, exec_t e, std::vector<common::tagged_tuple<S,U>> const&... vs) {
    run(T{}, e, vs...);
    run(common::type_sequence<Ts...>{}, e, vs...);
}

//! @brief Running a single component combination (assuming dynamic execution policy).
template <typename T, typename... S, typename... U>
void run(T x, std::vector<common::tagged_tuple<S,U>> const&... vs) {
    using exec_t = std::conditional_t<
        sizeof...(vs) >= 1,
        common::tags::dynamic_execution,
        common::tags::sequential_execution
    >;
    run(x, exec_t{}, vs...);
}

//! @brief Running a non-empty sequence of component combinations (assuming dynamic execution policy).
template <typename T, typename... Ts, typename... S, typename... U>
void run(common::type_sequence<T, Ts...> x, std::vector<common::tagged_tuple<S,U>> const&... vs) {
    using exec_t = std::conditional_t<
        sizeof...(vs) >= 1,
        common::tags::dynamic_execution,
        common::tags::sequential_execution
    >;
    run(x, exec_t{}, vs...);
}
//! @}


}


}

#endif // FCPP_SIMULATION_BATCH_H_
