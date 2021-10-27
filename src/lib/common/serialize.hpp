// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file serialize.hpp
 * @brief Implementation of serialization operations for standard and FCPP classes.
 */

#ifndef FCPP_COMMON_SERIALIZE_H_
#define FCPP_COMMON_SERIALIZE_H_

#include <cstring>
#include <map>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <type_traits>

#include "lib/internal/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing objects of common use.
namespace common {


//! @brief Exception class for format errors in deserialising.
class format_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
    using std::runtime_error::operator=;
};


//! @cond INTERNAL
namespace details {
    //! @brief Proxy function copying memory without unwanted warnings.
    inline void copy(void const* x, void const* y, size_t l) {
        memcpy((void*)x, y, l);
    }
}
//! @endcond


//! @brief Stream-like object for input or output serialization (depending on `io`).
template <bool io>
class sstream;


//! @brief Stream-like object for input serialization.
//! @{
template <>
class sstream<false> {
  public:
    //! @brief Constructor from raw data.
    sstream(std::vector<char> data) : m_data(data), m_idx(0) {}

    //! @brief Reads a trivial type from the stream.
    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    sstream& read(T& x, size_t l = sizeof(T)) {
        #if __cpp_exceptions
        if (m_idx + l > m_data.size())
            throw format_error("format error in deserialisation");
        #endif
        details::copy(&x, m_data.data() + m_idx, l);
        m_idx += l;
        return *this;
    }

    //! @brief The size of the raw data yet to be read.
    size_t size() const {
        return m_data.size() - m_idx;
    }

    //! @brief Access to the raw data.
    std::vector<char>& data() {
        return m_data;
    }

    //! @brief Const access to the raw data.
    std::vector<char> const& data() const {
        return m_data;
    }

  private:
    //! @brief The raw data.
    std::vector<char> m_data;
    //! @brief The read index.
    size_t m_idx;
};
using isstream = sstream<false>;
//! @}


//! @brief Stream-like object for output serialization.
//! @{
template <>
class sstream<true> {
  public:
    //! @brief Default constructor.
    sstream() = default;

    //! @brief Conversion to raw data.
    operator std::vector<char>() {
        return m_data;
    }

    //! @brief Writes a trivial type from the stream.
    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    sstream& write(T const& x, size_t l = sizeof(T)) {
        m_data.resize(m_data.size() + l);
        details::copy(m_data.data() + m_data.size() - l, &x, l);
        return *this;
    }

    //! @brief The size of the raw data written so far.
    size_t size() const {
        return m_data.size();
    }

    //! @brief Access to the raw data.
    std::vector<char>& data() {
        return m_data;
    }

    //! @brief Const access to the raw data.
    std::vector<char> const& data() const {
        return m_data;
    }

  private:
    //! @brief The raw data.
    std::vector<char> m_data;
};
using osstream = sstream<true>;
//! @}


//! @cond INTERNAL
namespace details {
    template<typename C>
    struct has_serialize_method;
    template<typename C>
    struct has_serialize_function;
    template <typename C>
    struct has_serialize_trivial;
}

template <bool io, typename T>
std::enable_if_t<details::has_serialize_method<T>::value, sstream<io>&>
inline operator&(sstream<io>& is, T& x);
template <bool io, typename T>
inline std::enable_if_t<details::has_serialize_function<T>::value, sstream<io>&>
operator&(sstream<io>& is, T& x);
template <typename T>
std::enable_if_t<details::has_serialize_trivial<T>::value, isstream&>
inline operator&(isstream& is, T& x);
template <typename T>
std::enable_if_t<details::has_serialize_trivial<T>::value, osstream&>
inline operator&(osstream& os, T& x);

namespace details {
    //! @brief Serialization of indexed classes.
    //! @{
    template <typename S, typename T>
    S& indexed_serialize(S& s, T&, std::index_sequence<>) {
        return s;
    }

    template <typename S, typename T, size_t i, size_t... is>
    S& indexed_serialize(S& s, T& x, std::index_sequence<i, is...>) {
        s & std::get<i>(x);
        return indexed_serialize(s, x, std::index_sequence<is...>{});
    }

    template <typename S, typename... Ts>
    S& serialize(S& s, std::tuple<Ts...>& x) {
        return indexed_serialize(s, x, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template <typename S, typename T, size_t n>
    S& serialize(S& s, std::array<T, n>& x) {
        return indexed_serialize(s, x, std::make_index_sequence<n>{});
    }

    template <typename S, typename T, typename U>
    S& serialize(S& s, std::pair<T, U>& x) {
        return indexed_serialize(s, x, std::make_index_sequence<2>{});
    }
    //! @}

    //! @brief Variable-length serialization of container sizes.
    //! @{
    inline void size_variable_read(isstream& s, size_t& v) {
        v = 0;
        uint8_t x;
        for (int offs = 0; ; offs += 7) {
            s.read(x);
            v += (x & 127ULL) << offs;
            if (x < 128) break;
        }
    }
    inline void size_variable_write(osstream& s, size_t v) {
        do {
            uint8_t x = (v & 127) + 128 * (v >= 128);
            s.write(x);
            v >>= 7;
        } while (v > 0);
    }
    //! @}

    //! @brief Serialization of iterable classes.
    //! @{
    template <typename T>
    isstream& iterable_serialize(isstream& s, T& x) {
        size_t size = 0;
        size_variable_read(s, size);
        x.clear();
        for (size_t i = 0; i < size; ++i) {
            typename T::value_type v;
            s & v;
            x.insert(x.end(), std::move(v));
        }
        return s;
    }

    template <typename T>
    osstream& iterable_serialize(osstream& s, T& x) {
        size_variable_write(s, x.size());
        for (auto& i : x) s & i;
        return s;
    }

    template <typename S, typename K>
    S& serialize(S& s, std::set<K>& x) {
        return iterable_serialize(s, x);
    }

    template <typename S, typename K, typename V>
    S& serialize(S& s, std::map<K, V>& x) {
        return iterable_serialize(s, x);
    }

    template <typename S, typename K>
    S& serialize(S& s, std::unordered_set<K>& x) {
        return iterable_serialize(s, x);
    }

    template <typename S, typename K, typename V>
    S& serialize(S& s, std::unordered_map<K, V>& x) {
        return iterable_serialize(s, x);
    }

    template <typename S, typename T>
    S& serialize(S& s, std::vector<T>& x) {
        return iterable_serialize(s, x);
    }
    //! @}

    //! @brief Checks whether a class has a serialize member function.
    template<typename C>
    struct has_serialize_method {
      private:
        struct tag;

        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<
            decltype(std::declval<T>().serialize(std::declval<tag&>())),
            tag&
        >::type;

        template <typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

      public:
        static constexpr bool value = type::value;
    };

    //! @brief Checks whether a class has a serialize free function.
    template<typename C>
    struct has_serialize_function {
      private:
        struct tag;

        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<
            decltype(fcpp::common::details::serialize(std::declval<tag&>(), std::declval<T&>())),
            tag&
        >::type;

        template <typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

      public:
        static constexpr bool value = type::value and not has_serialize_method<C>::value;
    };

    //! @brief Checks whether a class has a trivial serialize.
    template <typename C>
    struct has_serialize_trivial {
        static constexpr bool value = std::is_trivially_copyable<C>::value and not has_serialize_method<C>::value and not has_serialize_function<C>::value;
    };
}
//! @endcond


//! @brief Serialisation from/to user classes.
template <bool io, typename T>
std::enable_if_t<details::has_serialize_method<T>::value, sstream<io>&>
inline operator&(sstream<io>& is, T& x) {
    return x.serialize(is);
}

//! @brief Serialisation from/to standard containers.
template <bool io, typename T>
inline std::enable_if_t<details::has_serialize_function<T>::value, sstream<io>&>
operator&(sstream<io>& is, T& x) {
    return details::serialize(is, x);
}

//! @brief Serialisation from trivial types.
template <typename T>
std::enable_if_t<details::has_serialize_trivial<T>::value, isstream&>
inline operator&(isstream& is, T& x) {
    return is.read(x);
}

//! @brief Serialisation to trivial types.
template <typename T>
std::enable_if_t<details::has_serialize_trivial<T>::value, osstream&>
inline operator&(osstream& os, T& x) {
    return os.write(x);
}


//! @brief Serialisation from an input stream.
template <typename T>
inline isstream& operator>>(isstream& is, T& x) {
    return is & x;
}


//! @brief Serialisation to an output stream.
template <typename T>
inline osstream& operator<<(osstream& os, T const& x) {
    return os & ((T&)x);
}


}


}

#endif // FCPP_COMMON_SERIALIZE_H_
