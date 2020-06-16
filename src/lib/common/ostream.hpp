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

#include "lib/common/flat_ptr.hpp"
#include "lib/common/multitype_map.hpp"
#include "lib/common/random_access_map.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/twin.hpp"
#include "lib/data/context.hpp"
#include "lib/data/tuple.hpp"


/**
 * @brief Namespace of the C++ standard library.
 */
namespace std {
    //! @brief Printing arrays.
    template <typename T, size_t n>
    std::ostream& operator<<(std::ostream& o, const std::array<T, n>& m) {
        o << "[";
        bool first = true;
        for (const auto& x : m) {
            if (not first) o << ", ";
            o << x;
            first = false;
        }
        o << "]";
        return o;
    }

    //! @cond INTERNAL
    namespace details {
        void print(std::ostream&) {}
    
        template <typename T>
        void print(std::ostream& o, const T& x) {
            o << x;
        }

        template <typename T, typename... Ts>
        void print(std::ostream& o, const T& x, const Ts&... xs) {
            o << x << "; ";
            print(o, xs...);
        }
    
        template <typename T, size_t... is>
        void print(std::ostream& o, const T& x, std::index_sequence<is...>) {
            print(o, get<is>(x)...);
        }
    }
    //! @endcond

    //! @brief Printing pairs.
    template <typename T, typename U>
    std::ostream& operator<<(std::ostream& o, const std::pair<T, U>& m) {
        o << "(";
        details::print(o, m.first, m.second);
        o << ")";
        return o;
    }

    //! @brief Printing tuples.
    template <typename... Ts>
    std::ostream& operator<<(std::ostream& o, const std::tuple<Ts...>& m) {
        o << "(";
        details::print(o, m, std::make_index_sequence<sizeof...(Ts)>{});
        o << ")";
        return o;
    }

    //! @brief Printing vectors.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const std::vector<T>& m) {
        o << "[";
        bool first = true;
        for (const auto& x : m) {
            if (not first) o << ", ";
            o << x;
            first = false;
        }
        o << "]";
        return o;
    }

    //! @brief Printing ordered sets.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const std::set<T>& m) {
        o << "{";
        bool first = true;
        for (const auto& x : m) {
            if (not first) o << ", ";
            o << x;
            first = false;
        }
        o << "}";
        return o;
    }

    //! @brief Printing unordered sets.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const std::unordered_set<T>& m) {
        o << "{";
        bool first = true;
        for (const auto& x : m) {
            if (not first) o << ", ";
            o << x;
            first = false;
        }
        o << "}";
        return o;
    }

    //! @brief Printing ordered maps.
    template <typename K, typename V>
    std::ostream& operator<<(std::ostream& o, const std::map<K,V>& m) {
        o << "{";
        bool first = true;
        for (const auto& x : m) {
            if (not first) o << ", ";
            o << x.first << ":" << x.second;
            first = false;
        }
        o << "}";
        return o;
    }

    //! @brief Printing unordered maps.
    template <typename K, typename V>
    std::ostream& operator<<(std::ostream& o, const std::unordered_map<K,V>& m) {
        o << "{";
        bool first = true;
        for (const auto& x : m) {
            if (not first) o << ", ";
            o << x.first << ":" << x.second;
            first = false;
        }
        o << "}";
        return o;
    }
}


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {
//! @brief Printing tuples.
template <typename... Ts>
std::ostream& operator<<(std::ostream& o, const tuple<Ts...>& m) {
    o << "(";
    std::details::print(o, m, std::make_index_sequence<sizeof...(Ts)>{});
    o << ")";
    return o;
}

//! @brief Namespace containing objects of common use.
namespace common {
    //! @brief Printing content of flat pointers.
    template <typename T, bool is_flat>
    std::ostream& operator<<(std::ostream& o, const flat_ptr<T, is_flat>& p) {
        return o << *p;
    }

    //! @brief Printing multitype maps in arrowhead format.
    template <typename T, typename... Ts>
    std::ostream& operator<<(std::ostream& o, const multitype_map<T, Ts...>& m) {
        o << "(";
        m.print(o, arrowhead_tuple);
        o << ")";
        return o;
    }

    //! @brief Printing random access maps.
    template <typename K, typename V>
    std::ostream& operator<<(std::ostream& o, const random_access_map<K,V>& m) {
        o << "{";
        bool first = true;
        for (const auto& x : m) {
            if (not first) o << ", ";
            o << x.first << ":" << x.second;
            first = false;
        }
        o << "}";
        return o;
    }

    //! @brief Printing tagged tuples in arrowhead format.
    template <typename S, typename T>
    std::ostream& operator<<(std::ostream& o, const tagged_tuple<S,T>& t) {
        o << "(";
        t.print(o, arrowhead_tuple);
        o << ")";
        return o;
    }

    //! @brief Printing identical twin objects.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const twin<T,true>& t) {
        return o << "(" << t.first() << ")";
    }

    //! @brief Printing different twin objects.
    template <typename T>
    std::ostream& operator<<(std::ostream& o, const twin<T,false>& t) {
        return o << "(" << t.first() << "; " << t.second() << ")";
    }
}


//! @brief Namespace containing specific objects for the FCPP library.
namespace data {
    //! @brief Printing calculus contexts.
    template <bool b, bool d, typename... Ts>
    std::ostream& operator<<(std::ostream& o, const context<b, d, Ts...>& c) {
        o << "{";
        c.print(o);
        o << "}";
        return o;
    }
}


}


#endif // FCPP_COMMON_OSTREAM_H_
