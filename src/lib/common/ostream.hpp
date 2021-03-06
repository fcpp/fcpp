// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file ostream.hpp
 * @brief Implementation of output stream operations for standard and FCPP classes.
 */

#ifndef FCPP_COMMON_OSTREAM_H_
#define FCPP_COMMON_OSTREAM_H_

#include <cmath>

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "lib/settings.hpp"
#include "lib/common/traits.hpp"


//! @cond INTERNAL
namespace fcpp {

namespace details {
    template <typename T>
    std::string real2str(T x) {
        if (std::isfinite(x)) {
            std::string s;
            if (x == 0) return "0";
            if (x < 0) {
                s.push_back('-');
                x = -x;
            }
            int e = 0;
            if (0.1 > x or x >= 10000) {
                e = log10(x);
                x /= pow(10, e);
            }
            int i = x;
            s += std::to_string(i);
            s.push_back('.');
            i = (x-i)*100;
            s.push_back('0'+i/10);
            s.push_back('0'+i%10);
            if (e != 0) {
                s.push_back(' ');
                s.push_back('1');
                s.push_back('0');
                s.push_back('^');
                s += std::to_string(e);
            }
            return s;
        }
        if (std::isnan(x)) return "nan";
        return x < 0 ? "-inf" : "inf";
    }
}

//! @brief String conversion of numeric types.
//! @{
inline std::string to_string(int x) {
    return std::to_string(x);
}
inline std::string to_string(long x) {
    return std::to_string(x);
}
inline std::string to_string(long long x) {
    return std::to_string(x);
}
inline std::string to_string(unsigned x) {
    return std::to_string(x);
}
inline std::string to_string(unsigned long x) {
    return std::to_string(x);
}
inline std::string to_string(unsigned long long x) {
    return std::to_string(x);
}
inline std::string to_string(float x) {
    return details::real2str<float>(x);
}
inline std::string to_string(double x) {
    return details::real2str<double>(x);
}
inline std::string to_string(long double x) {
    return details::real2str<long double>(x);
}
//! @}


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
    //! @brief Converts to string an iterable container.
    template <typename T>
    inline std::string iterable_stringify(const char* delim, T const& c, const char* sep = ", ") {
        std::string s;
        s.push_back(delim[0]);
        bool first = true;
        for (auto const& x : c) {
            if (first) first = false;
            else {
                s.push_back(sep[0]);
                s.push_back(sep[1]);
            }
            s += to_string(common::escape(x));
        }
        s.push_back(delim[1]);
        return s;
    }

    //! @brief Prints an iterable container.
    template <typename O, typename T>
    O& iterable_print(O& o, const char* delim, T const& c) {
        o << delim[0];
        bool first = true;
        for (auto const& x : c) {
            if (first) first = false;
            else o << ", ";
            o << common::escape(x);
        }
        o << delim[1];
        return o;
    }
    //! @brief Converts to string an iterable container.
    template <typename T>
    inline std::string pair_iterable_stringify(const char* delim, T const& c) {
        std::string s;
        s.push_back(delim[0]);
        bool first = true;
        for (auto const& x : c) {
            if (first) first = false;
            else {
                s.push_back(',');
                s.push_back(' ');
            }
            s += to_string(common::escape(x.first));
            s.push_back(':');
            s += to_string(common::escape(x.second));
        }
        s.push_back(delim[1]);
        return s;
    }


    //! @brief Prints an iterable container.
    template <typename O, typename T>
    O& pair_iterable_print(O& o, const char* delim, T const& c) {
        o << delim[0];
        bool first = true;
        for (auto const& x : c) {
            if (first) first = false;
            else o << ", ";
            o << common::escape(x.first) << ":" << common::escape(x.second);
        }
        o << delim[1];
        return o;
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

    //! @brief Converts to string an indexable structure.
    template <typename T, size_t... is, typename tag>
    inline std::string indexed_stringify(const char* delim, const T& x, std::index_sequence<is...>, tag) {
        return iterable_stringify(delim, std::vector<std::string>{to_string(common::escape(get<is>(x, tag{})))...}, "; ");
    }

    //! @brief Prints an indexable structure.
    //! @{
    template <typename O>
    inline void indexed_print(O&) {}
    template <typename O, typename T>
    inline void indexed_print(O& o, const T& x) {
        o << x;
    }
    template <typename O, typename T, typename... Ts>
    inline void indexed_print(O& o, const T& x, const Ts&... xs) {
        o << x << "; ";
        indexed_print(o, xs...);
    }
    template <typename O, typename T, size_t... is, typename tag>
    O& indexed_print(O& o, const char* delim, const T& x, std::index_sequence<is...>, tag) {
        o << delim[0];
        indexed_print(o, common::escape(get<is>(x, tag{}))...);
        o << delim[1];
        return o;
    }
    //! @}

    //! @brief Prints a self-printable structure.
    template <typename O, typename T, typename... tags>
    O& printable_print(O& o, const char* delim, T const& c, tags...) {
        o << delim[0];
        c.print(o, tags{}...);
        o << delim[1];
        return o;
    }
}
}
//! @endcond


/**
 * @brief Namespace of the C++ standard library.
 */
namespace std {
    //! @brief Printing arrays.
    template <typename O, typename T, size_t n, typename = fcpp::common::if_ostream<O>>
    O& operator<<(O& o, const std::array<T, n>& v) {
        return fcpp::details::iterable_print(o, "[]", v);
    }

    //! @brief Converting arrays to strings.
    template <typename T, size_t n>
    std::string to_string(std::array<T, n> const& v) {
        return fcpp::details::iterable_stringify("[]", v);
    }

    //! @brief Printing pairs.
    template <typename O, typename T, typename U, typename = fcpp::common::if_ostream<O>>
    O& operator<<(O& o, const std::pair<T, U>& p) {
        return fcpp::details::indexed_print(o, "()", p, std::make_index_sequence<2>{}, fcpp::details::std_tag{});
    }

    //! @brief Converting pairs to strings.
    template <typename T, typename U>
    std::string to_string(std::pair<T, U> const& p) {
        return fcpp::details::indexed_stringify("()", p, std::make_index_sequence<2>{}, fcpp::details::std_tag{});
    }

    //! @brief Printing tuples.
    template <typename O, typename... Ts, typename = fcpp::common::if_ostream<O>>
    O& operator<<(O& o, const std::tuple<Ts...>& t) {
        return fcpp::details::indexed_print(o, "()", t, std::make_index_sequence<sizeof...(Ts)>{}, fcpp::details::std_tag{});
    }

    //! @brief Converting tuples to strings.
    template <typename... Ts>
    std::string to_string(std::tuple<Ts...> const& t) {
        return fcpp::details::indexed_stringify("()", t, std::make_index_sequence<sizeof...(Ts)>{}, fcpp::details::std_tag{});
    }

    //! @brief Printing vectors.
    template <typename O, typename T, typename = fcpp::common::if_ostream<O>>
    O& operator<<(O& o, const std::vector<T>& v) {
        return fcpp::details::iterable_print(o, "[]", v);
    }

    //! @brief Converting vectors to strings.
    template <typename T>
    std::string to_string(std::vector<T> const& v) {
        return fcpp::details::iterable_stringify("[]", v);
    }

    //! @brief Printing ordered sets.
    template <typename O, typename T, typename = fcpp::common::if_ostream<O>>
    O& operator<<(O& o, const std::set<T>& s) {
        return fcpp::details::iterable_print(o, "{}", s);
    }

    //! @brief Converting ordered sets to strings.
    template <typename T>
    std::string to_string(std::set<T> const& s) {
        return fcpp::details::iterable_stringify("{}", s);
    }

    //! @brief Printing unordered sets.
    template <typename O, typename T, typename = fcpp::common::if_ostream<O>>
    O& operator<<(O& o, const std::unordered_set<T>& s) {
        return fcpp::details::iterable_print(o, "{}", s);
    }

    //! @brief Converting unordered sets to strings.
    template <typename T>
    std::string to_string(std::unordered_set<T> const& s) {
        return fcpp::details::iterable_stringify("{}", s);
    }

    //! @brief Printing ordered maps.
    template <typename O, typename K, typename V, typename = fcpp::common::if_ostream<O>>
    O& operator<<(O& o, const std::map<K,V>& m) {
        return fcpp::details::pair_iterable_print(o, "{}", m);
    }

    //! @brief Converting maps to strings.
    template <typename K, typename V>
    std::string to_string(std::map<K,V> const& m) {
        return fcpp::details::pair_iterable_stringify("{}", m);
    }

    //! @brief Printing unordered maps.
    template <typename O, typename K, typename V, typename = fcpp::common::if_ostream<O>>
    O& operator<<(O& o, const std::unordered_map<K,V>& m) {
        return fcpp::details::pair_iterable_print(o, "{}", m);
    }

    //! @brief Converting unordered maps to strings.
    template <typename K, typename V>
    std::string to_string(std::unordered_map<K,V> const& m) {
        return fcpp::details::pair_iterable_stringify("{}", m);
    }
}


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {
    //! @brief Printing fields.
    template <typename O, typename T, typename = common::if_ostream<O>>
    O& operator<<(O& o, const field<T>& x) {
        o << "{";
        for (size_t i = 0; i < details::get_ids(x).size(); ++i) {
            o << details::get_ids(x)[i] << ":" << common::escape(details::get_vals(x)[i+1]) << ", ";
        }
        o << "*:" << common::escape(details::get_vals(x)[0]) << "}";
        return o;
    }

    //! @brief Converting fields to strings.
    template <typename T>
    std::string to_string(field<T> const& x) {
        std::string s = "{";
        for (size_t i = 0; i < details::get_ids(x).size() and i < FCPP_FIELD_DRAW_LIMIT and s.size() < 10*FCPP_FIELD_DRAW_LIMIT; ++i) {
            s += to_string(details::get_ids(x)[i]);
            s.push_back(':');
            s += to_string(common::escape(details::get_vals(x)[i+1]));
            if (i+1 == details::get_ids(x).size() or (i < FCPP_FIELD_DRAW_LIMIT-1 and s.size() < 10*FCPP_FIELD_DRAW_LIMIT-2)) s.push_back(',');
            else for (int j=0; j<3; ++j) s.push_back('.');
            s.push_back(' ');
        }
        s.push_back('*');
        s.push_back(':');
        s += to_string(common::escape(details::get_vals(x)[0]));
        s.push_back('}');
        return s;
    }

    //! @brief Printing tuples.
    template <typename O, typename... Ts, typename = common::if_ostream<O>>
    O& operator<<(O& o, const tuple<Ts...>& t) {
        return fcpp::details::indexed_print(o, "()", t, std::make_index_sequence<sizeof...(Ts)>{}, fcpp::details::fcpp_tag{});
    }

    //! @brief Converting tuples to strings.
    template <typename... Ts>
    std::string to_string(tuple<Ts...> const& t) {
        return fcpp::details::indexed_stringify("()", t, std::make_index_sequence<sizeof...(Ts)>{}, fcpp::details::fcpp_tag{});
    }

    //! @brief Printing vectors.
    template <typename O, size_t n, typename = common::if_ostream<O>>
    O& operator<<(O& o, const vec<n>& p) {
        return fcpp::details::iterable_print(o, "[]", p);
    }

    //! @brief Converting vectors to strings.
    template <size_t n>
    std::string to_string(vec<n> const& p) {
        return fcpp::details::iterable_stringify("[]", p);
    }

    //! @brief Namespace containing objects of common use.
    namespace common {
        //! @brief Printing multitype maps in arrowhead format.
        template <typename O, typename T, typename... Ts, typename = if_ostream<O>>
        O& operator<<(O& o, const multitype_map<T, Ts...>& m) {
            return fcpp::details::printable_print(o, "()", m);
        }

        //! @brief Printing random access maps.
        template <typename O, typename K, typename T, typename H, typename P, typename A, typename = if_ostream<O>>
        O& operator<<(O& o, const random_access_map<K,T,H,P,A>& m) {
            return fcpp::details::pair_iterable_print(o, "{}", m);
        }

        //! @brief Printing tagged tuples in arrowhead format.
        template <typename O, typename S, typename T, typename = if_ostream<O>>
        O& operator<<(O& o, const tagged_tuple<S,T>& t) {
            return fcpp::details::printable_print(o, "()", t);
        }
    }

    //! @brief Namespace containing objects of internal use.
    namespace internal {
        //! @brief Printing calculus contexts.
        template <typename O, bool b, bool d, typename... Ts, typename = common::if_ostream<O>>
        O& operator<<(O& o, const context<b, d, Ts...>& c) {
            return fcpp::details::printable_print(o, "()", c);
        }

        //! @brief Printing content of flat pointers.
        template <typename O, typename T, bool is_flat, typename = common::if_ostream<O>>
        O& operator<<(O& o, const flat_ptr<T, is_flat>& p) {
            o << common::escape(*p);
            return o;
        }

        //! @brief Printing identical twin objects.
        template <typename O, typename T, typename = common::if_ostream<O>>
        O& operator<<(O& o, const twin<T,true>& t) {
            o << "(" << common::escape(t.first()) << ")";
            return o;
        }

        //! @brief Printing different twin objects.
        template <typename O, typename T, typename = common::if_ostream<O>>
        O& operator<<(O& o, const twin<T,false>& t) {
            o << "(" << common::escape(t.first()) << "; " << common::escape(t.second()) << ")";
            return o;
        }
    }
}


#endif // FCPP_COMMON_OSTREAM_H_
