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

#include "lib/settings.hpp"
#include "lib/common/traits.hpp"


//! @cond INTERNAL
namespace fcpp {

template <typename T>
class field;
template <typename... Ts>
class tuple;
template <size_t n>
struct vec;

template <size_t n, typename... Ts>
typename std::tuple_element<n, std::tuple<Ts...>>::type const& get(const tuple<Ts...>&) noexcept;

namespace details {
    template <typename T>
    std::vector<device_t> const& get_ids(field<T> const&);
    template <typename T>
    std::vector<T> const& get_vals(field<T> const&);
}

namespace common {
    template <typename T, typename... Ts>
    class multitype_map;
    template <typename K, typename T, typename H, typename P, typename A>
    class random_access_map;
    template<typename S, typename T>
    struct tagged_tuple;
}

namespace internal {
    template <bool online, bool pointer, typename M, typename... Ts>
    class context;
    template <typename T, bool is_flat>
    class flat_ptr;
    template <typename T, bool is_twin>
    class twin;
}

namespace details {
    //! @brief Prints an iterable container.
    template <typename T>
    std::ostream& iterable_print(std::ostream& o, const char* delim, T const& c) {
        o << delim[0];
        bool first = true;
        for (auto const& x : c) {
            if (first) first = false;
            else o << ", ";
            o << common::escape(x);
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
            o << common::escape(x.first) << ":" << common::escape(x.second);
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
    inline void indexed_print(std::ostream&) {}
    template <typename T>
    inline void indexed_print(std::ostream& o, const T& x) {
        o << x;
    }
    template <typename T, typename... Ts>
    inline void indexed_print(std::ostream& o, const T& x, const Ts&... xs) {
        o << x << "; ";
        indexed_print(o, xs...);
    }
    template <typename T, size_t... is, typename tag>
    std::ostream& indexed_print(std::ostream& o, const char* delim, const T& x, std::index_sequence<is...>, tag) {
        o << delim[0];
        indexed_print(o, common::escape(get<is>(x, tag{}))...);
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
    //! @brief Printing fields.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const field<T>& x) {
        o << "{";
        for (size_t i = 0; i < details::get_ids(x).size(); ++i) {
            o << details::get_ids(x)[i] << ":" << common::escape(details::get_vals(x)[i+1]) << ", ";
        }
        return o << "*:" << common::escape(details::get_vals(x)[0]) << "}";
    }

    //! @brief Printing tuples.
    template <typename... Ts>
    std::ostream& operator<<(std::ostream& o, const tuple<Ts...>& t) {
        return fcpp::details::indexed_print(o, "()", t, std::make_index_sequence<sizeof...(Ts)>{}, fcpp::details::fcpp_tag{});
    }

    //! @brief Printing vectors.
    template <size_t n>
    std::ostream& operator<<(std::ostream& o, const vec<n>& p) {
        return fcpp::details::iterable_print(o, "[]", p);
    }

    //! @brief Namespace containing objects of common use.
    namespace common {
        //! @brief Printing multitype maps in arrowhead format.
        template <typename T, typename... Ts>
        std::ostream& operator<<(std::ostream& o, const multitype_map<T, Ts...>& m) {
            return fcpp::details::printable_print(o, "()", m);
        }

        //! @brief Printing random access maps.
        template <typename K, typename T, typename H, typename P, typename A>
        std::ostream& operator<<(std::ostream& o, const random_access_map<K,T,H,P,A>& m) {
            return fcpp::details::pair_iterable_print(o, "{}", m);
        }

        //! @brief Printing tagged tuples in arrowhead format.
        template <typename S, typename T>
        std::ostream& operator<<(std::ostream& o, const tagged_tuple<S,T>& t) {
            return fcpp::details::printable_print(o, "()", t);
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
            return o << common::escape(*p);
        }

        //! @brief Printing identical twin objects.
        template <typename T>
        std::ostream& operator<<(std::ostream& o, const twin<T,true>& t) {
            return o << "(" << common::escape(t.first()) << ")";
        }

        //! @brief Printing different twin objects.
        template <typename T>
        std::ostream& operator<<(std::ostream& o, const twin<T,false>& t) {
            return o << "(" << common::escape(t.first()) << "; " << common::escape(t.second()) << ")";
        }
    }
}


#endif // FCPP_COMMON_OSTREAM_H_
