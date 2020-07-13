// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file ostream.hpp
 * @brief Implementation of output stream operations for standard and FCPP classes.
 */

#ifndef FCPP_COMMON_OSTREAM_H_
#define FCPP_COMMON_OSTREAM_H_

#include <map>
#include <ostream>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "lib/common/multitype_map.hpp"
#include "lib/common/random_access_map.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/data/tuple.hpp"
#include "lib/data/vec.hpp"
#include "lib/internal/context.hpp"
#include "lib/internal/flat_ptr.hpp"
#include "lib/internal/twin.hpp"


//! @cond INTERNAL
namespace fcpp {
namespace details {
    //! @brief Escapes a value for clear printing.
    //! @{
    std::string escape(bool x) {
        return x ? "true" : "false";
    }
    std::string escape(char x) {
        return std::string("'") + x + "'";
    }
    std::string escape(std::string x) {
        return '"' + x + '"';
    }
    std::string escape(const char* x) {
        return '"' + std::string(x) + '"';
    }
    template <typename T, typename = std::enable_if_t<std::is_empty<T>::value>>
    std::string escape(T) {
        return fcpp::common::type_name<T>();
    }
    template <typename T, typename = std::enable_if_t<fcpp::common::type_count<bool, char, std::string, const char*> >= 0 and not std::is_empty<T>::value>>
    T const& escape(T const& x) {
        return x;
    }
    //! @}

    //! @brief Prints an iterable container.
    template <typename T>
    std::ostream& iterable_print(std::ostream& o, const char* delim, T const& c) {
        o << delim[0];
        bool first = true;
        for (auto const& x : c) {
            if (first) first = false;
            else o << ", ";
            o << escape(x);
        }
        return o << delim[1];
    }

    //! @brief Prints an iterable container.
    template <typename T>
    std::ostream& pair_iterable_print(std::ostream& o, const char* delim, T const& c) {
        o << delim[0];
        bool first = true;
        for (auto const& x : c) {
            if (first) first = false;
            else o << ", ";
            o << escape(x.first) << ":" << escape(x.second);
        }
        return o << delim[1];
    }

    //! @brief Generic get for any structure.
    //! @{
    struct std_tag {};
    struct fcpp_tag {};
    template <size_t i, typename T>
    auto const& get(T const& x, std_tag) {
        return std::get<i>(x);
    }
    template <size_t i, typename T>
    auto const& get(T const& x, fcpp_tag) {
        return fcpp::get<i>(x);
    }
    //! @}

    //! @brief Prints an indexable structure.
    //! @{
    void indexed_print(std::ostream&) {}
    template <typename T>
    void indexed_print(std::ostream& o, const T& x) {
        o << x;
    }
    template <typename T, typename... Ts>
    void indexed_print(std::ostream& o, const T& x, const Ts&... xs) {
        o << x << "; ";
        indexed_print(o, xs...);
    }
    template <typename T, size_t... is, typename tag>
    std::ostream& indexed_print(std::ostream& o, const char* delim, const T& x, std::index_sequence<is...>, tag) {
        o << delim[0];
        indexed_print(o, escape(get<is>(x, tag{}))...);
        return o << delim[1];
    }
    //! @}

    //! @brief Prints a self-printable structure.
    template <typename T, typename... tags>
    std::ostream& printable_print(std::ostream& o, const char* delim, T const& c, tags...) {
        o << delim[0];
        c.print(o, tags{}...);
        return o << delim[1];
    }
}
}
//! @endcond


/**
 * @brief Namespace of the C++ standard library.
 */
namespace std {
    //! @brief Printing arrays.
    template <typename T, size_t n>
    std::ostream& operator<<(std::ostream& o, const std::array<T, n>& v) {
        return fcpp::details::iterable_print(o, "[]", v);
    }

    //! @brief Printing pairs.
    template <typename T, typename U>
    std::ostream& operator<<(std::ostream& o, const std::pair<T, U>& p) {
        return fcpp::details::indexed_print(o, "()", p, std::make_index_sequence<2>{}, fcpp::details::std_tag{});
    }

    //! @brief Printing tuples.
    template <typename... Ts>
    std::ostream& operator<<(std::ostream& o, const std::tuple<Ts...>& t) {
        return fcpp::details::indexed_print(o, "()", t, std::make_index_sequence<sizeof...(Ts)>{}, fcpp::details::std_tag{});
    }

    //! @brief Printing vectors.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const std::vector<T>& v) {
        return fcpp::details::iterable_print(o, "[]", v);
    }

    //! @brief Printing ordered sets.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const std::set<T>& s) {
        return fcpp::details::iterable_print(o, "{}", s);
    }

    //! @brief Printing unordered sets.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const std::unordered_set<T>& s) {
        return fcpp::details::iterable_print(o, "{}", s);
    }

    //! @brief Printing ordered maps.
    template <typename K, typename V>
    std::ostream& operator<<(std::ostream& o, const std::map<K,V>& m) {
        return fcpp::details::pair_iterable_print(o, "{}", m);
    }

    //! @brief Printing unordered maps.
    template <typename K, typename V>
    std::ostream& operator<<(std::ostream& o, const std::unordered_map<K,V>& m) {
        return fcpp::details::pair_iterable_print(o, "{}", m);
    }
}


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {
    //! @brief Printing vectors.
    template <size_t n>
    std::ostream& operator<<(std::ostream& o, const vec<n>& p) {
        return fcpp::details::iterable_print(o, "[]", p);
    }

    //! @brief Printing tuples.
    template <typename... Ts>
    std::ostream& operator<<(std::ostream& o, const tuple<Ts...>& t) {
        return fcpp::details::indexed_print(o, "()", t, std::make_index_sequence<sizeof...(Ts)>{}, fcpp::details::fcpp_tag{});
    }

    //! @brief Namespace containing objects of common use.
    namespace common {
        //! @brief Printing multitype maps in arrowhead format.
        template <typename T, typename... Ts>
        std::ostream& operator<<(std::ostream& o, const multitype_map<T, Ts...>& m) {
            return fcpp::details::printable_print(o, "()", m, arrowhead_tuple);
        }

        //! @brief Printing random access maps.
        template <typename K, typename V>
        std::ostream& operator<<(std::ostream& o, const random_access_map<K,V>& m) {
            return fcpp::details::pair_iterable_print(o, "{}", m);
        }

        //! @brief Printing tagged tuples in arrowhead format.
        template <typename S, typename T>
        std::ostream& operator<<(std::ostream& o, const tagged_tuple<S,T>& t) {
            return fcpp::details::printable_print(o, "()", t, arrowhead_tuple);
        }
    }

    //! @brief Namespace containing objects of internal use.
    namespace internal {
        //! @brief Printing calculus contexts.
        template <bool b, bool d, typename... Ts>
        std::ostream& operator<<(std::ostream& o, const context<b, d, Ts...>& c) {
            return fcpp::details::printable_print(o, "()", c);
        }

        //! @brief Printing content of flat pointers.
        template <typename T, bool is_flat>
        std::ostream& operator<<(std::ostream& o, const flat_ptr<T, is_flat>& p) {
            return o << fcpp::details::escape(*p);
        }

        //! @brief Printing identical twin objects.
        template <typename T>
        std::ostream& operator<<(std::ostream& o, const twin<T,true>& t) {
            return o << "(" << fcpp::details::escape(t.first()) << ")";
        }

        //! @brief Printing different twin objects.
        template <typename T>
        std::ostream& operator<<(std::ostream& o, const twin<T,false>& t) {
            return o << "(" << fcpp::details::escape(t.first()) << "; " << fcpp::details::escape(t.second()) << ")";
        }
    }
}


#endif // FCPP_COMMON_OSTREAM_H_
