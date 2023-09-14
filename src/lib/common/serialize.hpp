// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

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
#include <string>
#include <unordered_map>
#include <unordered_set>
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
#ifndef FCPP_DISABLE_EXCEPTIONS
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
//! @brief Stream-like object for input serialization (alias).
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
//! @brief Stream-like object for output serialization (alias).
using osstream = sstream<true>;
//! @}


/**
 * @brief Stream-like object for hashing data into an integer type `I`.
 *
 * Can hash any type <b>except</b> for unordered containers.
 */
class hstream {
  public:
    //! @brief Default constructor.
    hstream() : m_hash(0xeaa5dab21fc5f67aULL) {};

    //! @brief Conversion to raw data.
    template <typename I, typename = std::enable_if_t<std::is_integral<I>::value>>
    explicit operator I() const {
        return reducer(std::integer_sequence<size_t, sizeof(I)>{});
    }

    //! @brief Writes a trivial type from the stream. Hash taken from: https://github.com/ztanml/fast-hash
    template <typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
    hstream& write(T const& x, size_t l = sizeof(T)) {
        constexpr uint64_t m = 0x880355f21e6d1965ULL;
        uint64_t const* p64 = (uint64_t const*)(&x);
        uint64_t const* end = p64 + (l / 8);
        uint8_t const* p8;
        m_hash ^= l * m;
        uint64_t v;
        while (p64 != end) {
            v  = *p64++;
            m_hash ^= mix(v);
            m_hash *= m;
        }
        p8 = (uint8_t const*)p64;
        v = 0;
        switch (l & 7) {
        case 7: v ^= (uint64_t)p8[6] << 48;
        case 6: v ^= (uint64_t)p8[5] << 40;
        case 5: v ^= (uint64_t)p8[4] << 32;
        case 4: v ^= (uint64_t)p8[3] << 24;
        case 3: v ^= (uint64_t)p8[2] << 16;
        case 2: v ^= (uint64_t)p8[1] << 8;
        case 1: v ^= (uint64_t)p8[0];
            m_hash ^= mix(v);
            m_hash *= m;
        }
        mix(m_hash);
        return *this;
    }

    //! @brief The size of the raw data written so far.
    static constexpr size_t size() {
        return 8;
    }

    //! @brief Access to the raw data.
    uint64_t& data() {
        return m_hash;
    }

    //! @brief Const access to the raw data.
    uint64_t const& data() const {
        return m_hash;
    }

  private:
    //! @brief Compression function for Merkle-Damgard construction.
    inline uint64_t& mix(uint64_t& x) const {
        x ^= x >> 23;
        x *= 0x2127599bf4325c37ULL;
        x ^= x >> 47;
        return x;
    }

    //! @brief Reduces to a 8-bytes hash.
    inline uint64_t reducer(std::integer_sequence<size_t, 8>) const {
        return m_hash;
    }

    //! @brief Reduces to a 4-bytes hash.
    inline uint32_t reducer(std::integer_sequence<size_t, 4>) const {
        return m_hash - (m_hash >> 32);
    }

    //! @brief Reduces to a 2-bytes hash.
    inline uint16_t reducer(std::integer_sequence<size_t, 2>) const {
        uint32_t x = reducer(std::integer_sequence<size_t, 4>{});
        return x - (x >> 16);
    }

    //! @brief Reduces to a 1-byte hash.
    inline uint8_t reducer(std::integer_sequence<size_t, 1>) const {
        uint32_t x = reducer(std::integer_sequence<size_t, 2>{});
        return x - (x >> 8);
    }

    //! @brief The raw data.
    uint64_t m_hash;
};


//! @brief Generic hashing function based on `hstream`.
template <typename I, typename T>
inline I hash_to(T const& x) {
    hstream hs;
    hs << x;
    return I(hs);
}

//! @brief Generic hasher class based on `hstream`, that can be used with standard containers.
template <typename T>
struct hash {
    //! @brief Reduces a
    inline size_t operator()(T const& x) const {
        return hash_to<size_t>(x);
    }
};


//! @brief Checks whether a type is a FCPP stream (default overload).
template <typename S>
struct is_stream : public std::false_type {};

//! @brief Checks whether a type is a FCPP stream (serialising stream overload).
template <bool io>
struct is_stream<sstream<io>> : public std::true_type {};

//! @brief Checks whether a type is a FCPP stream (hashing stream overload).
template <>
struct is_stream<hstream> : public std::true_type {};


//! @cond INTERNAL
namespace details {
    template<typename C>
    struct has_serialize_method;
    template<typename C>
    struct has_serialize_function;
    template <typename C>
    struct has_serialize_trivial;
}

template <typename S, typename T>
std::enable_if_t<details::has_serialize_method<T>::value and is_stream<S>::value, S&>
inline operator&(S& is, T& x);

template <typename S, typename T>
std::enable_if_t<details::has_serialize_function<T>::value and is_stream<S>::value, S&>
inline operator&(S& is, T& x);

template <typename T>
std::enable_if_t<details::has_serialize_trivial<T>::value, isstream&>
inline operator&(isstream& is, T& x);

template <typename T>
std::enable_if_t<details::has_serialize_trivial<T>::value, osstream&>
inline operator&(osstream& os, T& x);

template <typename T>
std::enable_if_t<details::has_serialize_trivial<T>::value, hstream&>
inline operator&(hstream& hs, T& x);

namespace details {
    //! @brief Serialization of indexed classes.
    //! @{
    template <typename S, typename T>
    S& indexed_serialize(S& s, T const&, std::index_sequence<>) {
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

    template <typename S, typename... Ts>
    S& serialize(S& s, std::tuple<Ts...> const& x) {
        return indexed_serialize(s, x, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template <typename S, typename T, size_t n>
    S& serialize(S& s, std::array<T, n>& x) {
        return indexed_serialize(s, x, std::make_index_sequence<n>{});
    }

    template <typename S, typename T, size_t n>
    S& serialize(S& s, std::array<T, n> const& x) {
        return indexed_serialize(s, x, std::make_index_sequence<n>{});
    }

    template <typename S, typename T, typename U>
    S& serialize(S& s, std::pair<T, U>& x) {
        return indexed_serialize(s, x, std::make_index_sequence<2>{});
    }

    template <typename S, typename T, typename U>
    S& serialize(S& s, std::pair<T, U> const& x) {
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
    template <typename S>
    inline void size_variable_write(S& s, size_t v) {
        do {
            uint8_t x = (v & 127) + 128 * (v >= 128);
            s.write(x);
            v >>= 7;
        } while (v > 0);
    }
    //! @}

    //! @brief Inert wrapper of a type.
    template <typename S>
    struct wrapper {};

    //! @brief Serialization of iterable classes.
    //! @{
    template <typename T, typename S, typename U = wrapper<void>>
    isstream& iterable_serialize(isstream& s, T& x, wrapper<S>, U = {}) {
        size_t size = 0;
        size_variable_read(s, size);
        x.clear();
        for (size_t i = 0; i < size; ++i) {
            S v;
            s & v;
            x.insert(x.end(), std::move(v));
        }
        return s;
    }

    template <typename T, typename S, typename U = wrapper<void>>
    osstream& iterable_serialize(osstream& s, T& x, S, U = {}) {
        size_variable_write(s, x.size());
        for (auto& i : x) s & i;
        return s;
    }

    template <typename T, typename S>
    hstream& iterable_serialize(hstream& s, T& x, S, wrapper<void> = {}) {
        s.write(x.size());
        for (auto& i : x) s & i;
        return s;
    }

    template <typename S>
    S& serialize(S& s, std::string& x) {
        return iterable_serialize(s, x, wrapper<char>{});
    }

    template <typename S>
    S& serialize(S& s, std::string const& x) {
        return iterable_serialize(s, x, wrapper<char>{});
    }

    template <typename S, typename K, typename... Ts>
    S& serialize(S& s, std::multiset<K,Ts...>& x) {
        return iterable_serialize(s, x, wrapper<K>{});
    }

    template <typename S, typename K, typename... Ts>
    S& serialize(S& s, std::multiset<K,Ts...> const& x) {
        return iterable_serialize(s, x, wrapper<K>{});
    }

    template <typename S, typename K, typename... Ts>
    S& serialize(S& s, std::set<K,Ts...>& x) {
        return iterable_serialize(s, x, wrapper<K>{});
    }

    template <typename S, typename K, typename... Ts>
    S& serialize(S& s, std::set<K,Ts...> const& x) {
        return iterable_serialize(s, x, wrapper<K>{});
    }

    template <typename S, typename K, typename V, typename... Ts>
    S& serialize(S& s, std::map<K, V, Ts...>& x) {
        return iterable_serialize(s, x, wrapper<std::pair<K,V>>{});
    }

    template <typename S, typename K, typename V, typename... Ts>
    S& serialize(S& s, std::map<K, V, Ts...> const& x) {
        return iterable_serialize(s, x, wrapper<std::pair<K,V>>{});
    }

    template <typename S, typename K, typename... Ts>
    S& serialize(S& s, std::unordered_multiset<K, Ts...>& x) {
        return iterable_serialize(s, x, wrapper<K>{}, wrapper<char>{});
    }

    template <typename S, typename K, typename... Ts>
    S& serialize(S& s, std::unordered_multiset<K, Ts...> const& x) {
        return iterable_serialize(s, x, wrapper<K>{}, wrapper<char>{});
    }

    template <typename S, typename K, typename... Ts>
    S& serialize(S& s, std::unordered_set<K, Ts...>& x) {
        return iterable_serialize(s, x, wrapper<K>{}, wrapper<char>{});
    }

    template <typename S, typename K, typename... Ts>
    S& serialize(S& s, std::unordered_set<K, Ts...> const& x) {
        return iterable_serialize(s, x, wrapper<K>{}, wrapper<char>{});
    }

    template <typename S, typename K, typename V, typename... Ts>
    S& serialize(S& s, std::unordered_map<K, V, Ts...>& x) {
        return iterable_serialize(s, x, wrapper<std::pair<K,V>>{}, wrapper<char>{});
    }

    template <typename S, typename K, typename V, typename... Ts>
    S& serialize(S& s, std::unordered_map<K, V, Ts...> const& x) {
        return iterable_serialize(s, x, wrapper<std::pair<K,V>>{}, wrapper<char>{});
    }

    template <typename S, typename T, typename... Ts>
    S& serialize(S& s, std::vector<T, Ts...>& x) {
        return iterable_serialize(s, x, wrapper<T>{});
    }

    template <typename S, typename T, typename... Ts>
    S& serialize(S& s, std::vector<T, Ts...> const& x) {
        return iterable_serialize(s, x, wrapper<T>{});
    }
    //! @}

    //! @brief Checks whether a type is a bounded char array, that can be trivially serialised.
    template <class> struct is_bounded_char_array : std::false_type {};

    template <size_t N>
    struct is_bounded_char_array<char[N]> : std::true_type {};

    template <size_t N>
    struct is_bounded_char_array<const char[N]> : std::true_type {};

    //! @brief Checks whether a class has a serialize member function.
    template<typename C>
    struct has_serialize_method {
      private:
        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<
            decltype(std::declval<T>().serialize(std::declval<osstream&>())),
            osstream&
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
        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<
            decltype(fcpp::common::details::serialize(std::declval<osstream&>(), std::declval<T&>())),
            osstream&
        >::type;

        template <typename>
        static constexpr std::false_type check(...);

        typedef decltype(check<C>(0)) type;

      public:
        static constexpr bool value = type::value and not has_serialize_method<C>::value and not is_bounded_char_array<C>::value;
    };

    //! @brief Checks whether a class has a trivial serialize.
    template <typename C>
    struct has_serialize_trivial {
        static constexpr bool value = std::is_trivially_copyable<C>::value and not has_serialize_method<C>::value and not has_serialize_function<C>::value;
    };
}
//! @endcond


//! @brief Serialisation from/to user classes.
template <typename S, typename T>
std::enable_if_t<details::has_serialize_method<T>::value and is_stream<S>::value, S&>
inline operator&(S& is, T& x) {
    return x.serialize(is);
}

//! @brief Serialisation from/to standard containers.
template <typename S, typename T>
std::enable_if_t<details::has_serialize_function<T>::value and is_stream<S>::value, S&>
inline operator&(S& is, T& x) {
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

//! @brief Hashing trivial types.
template <typename T>
std::enable_if_t<details::has_serialize_trivial<T>::value, hstream&>
inline operator&(hstream& hs, T& x) {
    return hs.write(x);
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


//! @brief Serialisation to an hashing stream.
template <typename T>
inline hstream& operator<<(hstream& os, T const& x) {
    return os & ((T&)x);
}


}


}

#endif // FCPP_COMMON_SERIALIZE_H_
