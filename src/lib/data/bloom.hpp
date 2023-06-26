// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file bloom.hpp
 * @brief Implementation of the `bloom_filter` class template for probabilistic set belonging.
 */

#ifndef FCPP_DATA_BLOOM_H_
#define FCPP_DATA_BLOOM_H_

#include <climits>
#include <cmath>
#include <cstdint>
#include <bitset>
#include <functional>
#include <initializer_list>
#include <limits>
#include <utility>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @cond INTERNAL
namespace details {
//! @brief Constexpr version of the absolute value.
    constexpr double abs(double x) {
        return x > 0 ? x : -x;
    }

    //! @brief Constexpr version of the exponential function.
    constexpr double exp(double x) {
        double sum = 1, n = 1, t = x;
        int i = 2;
        while (abs(t/n) > std::numeric_limits<double>::epsilon()) {
            sum += t/n;
            n *= i;
            ++i;
            t *= x;
        }
        return sum;
    }

    //! @brief Constexpr version of the natural logarithm function.
    constexpr double log(double x) {
        double y = 1, z = 0, t = 0;
        while (x < 1) {
            x *= 2.71828182845904523536;
            t -= 1;
        }
        while (x > 3) {
            x /= 2.71828182845904523536;
            t += 1;
        }
        while (abs(y - z) > 4*std::numeric_limits<double>::epsilon()) {
            y = z;
            z = y + 2 - 4 / (x * exp(-y) + 1);
        }
        return z + t;
    }

    //! @brief The largest integer type that is a divisor of a given type.
    template <typename T>
    using word_type = std::conditional_t<
        sizeof(T) % 4 == 0,
        std::conditional_t<sizeof(T) % 8 == 0, uint64_t, uint32_t>,
        std::conditional_t<sizeof(T) % 2 == 0, uint16_t, uint8_t>
    >;

    //! @brief Hard-coded salts to be used for the various hash functions.
    constexpr uint32_t salts[128] = {
        0xAAAAAAAA, 0x55555555, 0x33333333, 0xCCCCCCCC,
        0x66666666, 0x99999999, 0xB5B5B5B5, 0x4B4B4B4B,
        0xAA55AA55, 0x55335533, 0x33CC33CC, 0xCC66CC66,
        0x66996699, 0x99B599B5, 0xB54BB54B, 0x4BAA4BAA,
        0xAA33AA33, 0x55CC55CC, 0x33663366, 0xCC99CC99,
        0x66B566B5, 0x994B994B, 0xB5AAB5AA, 0xAAAAAA33,
        0x555555CC, 0x33333366, 0xCCCCCC99, 0x666666B5,
        0x9999994B, 0xB5B5B5AA, 0xFFFFFFFF, 0xFFFF0000,
        0xB823D5EB, 0xC1191CDF, 0xF623AEB3, 0xDB58499F,
        0xC8D42E70, 0xB173F616, 0xA91A5967, 0xDA427D63,
        0xB1E8A2EA, 0xF6C0D155, 0x4909FEA3, 0xA68CC6A7,
        0xC395E782, 0xA26057EB, 0x0CD5DA28, 0x467C5492,
        0xF15E6982, 0x61C6FAD3, 0x9615E352, 0x6E9E355A,
        0x689B563E, 0x0C9831A8, 0x6753C18B, 0xA622689B,
        0x8CA63C47, 0x42CC2884, 0x8E89919B, 0x6EDBD7D3,
        0x15B6796C, 0x1D6FDFE4, 0x63FF9092, 0xE7401432,
        0xEFFE9412, 0xAEAEDF79, 0x9F245A31, 0x83C136FC,
        0xC3DA4A8C, 0xA5112C8C, 0x5271F491, 0x9A948DAB,
        0xCEE59A8D, 0xB5F525AB, 0x59D13217, 0x24E7C331,
        0x697C2103, 0x84B0A460, 0x86156DA9, 0xAEF2AC68,
        0x23243DA5, 0x3F649643, 0x5FA495A8, 0x67710DF8,
        0x9A6C499E, 0xDCFB0227, 0x46A43433, 0x1832B07A,
        0xC46AFF3C, 0xB9C8FFF0, 0xC9500467, 0x34431BDF,
        0xB652432B, 0xE367F12B, 0x427F4C1B, 0x224C006E,
        0x2E7E5A89, 0x96F99AA5, 0x0BEB452A, 0x2FD87C39,
        0x74B2E1FB, 0x222EFD24, 0xF357F60C, 0x440FCB1E,
        0x8BBE030F, 0x6704DC29, 0x1144D12F, 0x948B1355,
        0x6D8FD7E9, 0x1C11A014, 0xADD1592F, 0xFB3C712E,
        0xFC77642F, 0xF9C4CE8C, 0x31312FB9, 0x08B0DD79,
        0x318FA6E7, 0xC040D23D, 0xC0589AA7, 0x0CA5C075,
        0xF874B172, 0x0CF914D5, 0x784D3280, 0x4E8CFEBC,
        0xC569F575, 0xCDB2A091, 0x2CC016B4, 0x5C5F4421
    };
}
//! @endcond

//! @brief The optimal number of hash functions in a bloom filter given the number of bits and the expected number of elements that will be inserted.
constexpr size_t optimal_bloom_hashes(size_t bits, size_t elements) {
    return bits * 0.69314718056 / elements + 0.5;
}

//! @brief The required number of bits in a bloom filter to grant a given error probability given the expected number of elements that will be inserted.
constexpr size_t required_bloom_bits(double p, size_t elements) {
    return elements * details::log(p) / -0.48045301391 + 0.5;
}

//! @brief The false positive probability given a certain configuration.
double bloom_error(size_t m, size_t bits, size_t elements);


/**
 * @brief A bloom filter data structure.
 *
 * It allows for insertion of elements and of whole other filters, while
 * providing an approximated relation of set membership.
 *
 * @param m     The number of hash functions (up to 128).
 * @param bits  The size in bits of the bloom filter.
 * @param T     The type of values to be inserted in the structure.
 * @param H     An callable class hashing T objects to a size_t value.
 */
template <size_t m, size_t bits, typename T = size_t, typename H = std::hash<T>>
class bloom_filter {
    static_assert(m <= 128, "too many hash functions");

  public:
    //! @brief The type of elements.
    using key_type = T;
    //! @brief The type of elements.
    using value_type = T;
    //! @brief The hasher type.
    using hasher = H;
    //! @brief The type for sizes.
    using size_type = size_t;

    //! @brief The number of hash functions used.
    constexpr static size_t hashes = m;
    //! @brief The size in bits of the bloom filter.
    constexpr static size_t bit_size = bits;

    //! @brief Constructs an empty filter.
    bloom_filter(H const& h = H()) {
        m_hash = h;
    }

    //! @brief Constructs a filter with an element.
    bloom_filter(T const& val, H const& h = H()) : bloom_filter(h) {
        insert(val);
    }

    //! @brief Constructs a filter with a range of elements.
    template <typename I>
    bloom_filter(I first, I last, H const& h = H()) : bloom_filter(h) {
        insert(first, last);
    }

    //! @brief Constructs a filter with a list of elements.
    bloom_filter(std::initializer_list<T> il, H const& h = H()) : bloom_filter(h) {
        insert(il);
    }

    //! @brief Copy constructor.
    bloom_filter(bloom_filter const&) = default;

    //! @brief Move constructor.
    bloom_filter(bloom_filter&&) = default;

    //! @brief Copy assignment.
    bloom_filter& operator=(bloom_filter const&) = default;

    //! @brief Move assignment.
    bloom_filter& operator=(bloom_filter&&) = default;

    //! @brief List assignment.
    bloom_filter& operator=(std::initializer_list<T> il) {
        clear();
        insert(il);
        return *this;
    }

    //! @brief Returns whether the filter is empty.
    bool empty() const noexcept {
        return m_data.none();
    }

    //! @brief Constructs and inserts a single element.
    template <typename... Ts>
    void emplace(Ts&&... xs) {
        insert(T(std::forward<Ts>(xs)...));
    }

    //! @brief Inserts a single element.
    void insert(T const& val) {
        size_t h = m_hash(val);
        for (size_t i = 0; i < m; ++i)
            m_data.set(jenkins(h, details::salts[i]) % bits);
    }

    //! @brief Inserts collection of elements.
    void insert(bloom_filter const& f) {
        m_data |= f.m_data;
    }

    //! @brief Inserts a range of elements.
    template <typename I>
    void insert(I first, I last) {
        for (I it = first; it != last; ++it)
            insert(*it);
    }

    //! @brief Inserts a list of elements.
    inline void insert(std::initializer_list<T> il) {
        insert(il.begin(), il.end());
    }

    //! @brief Checks whether an element is in the bloom filter.
    size_t count(T const& val) const {
        size_t h = m_hash(val);
        for (size_t i = 0; i < m; ++i)
            if (not m_data.test(jenkins(h, details::salts[i]) % bits))
                return 0;
        return 1;
    }

    //! @brief Clear content.
    void clear() noexcept {
        m_data.reset();
    }

    //! @brief Swap content.
    void swap(bloom_filter& f) {
        swap(m_data, f.m_data);
        swap(m_hash, f.m_hash);
    }

    //! @brief Get the hash function.
    H hash_function() const {
        return m_hash;
    }

    //! @brief Equality operator.
    bool operator==(bloom_filter const& f) const noexcept {
        return m_data == f.m_data;
    }

    //! @brief Inequality operator.
    bool operator!=(bloom_filter const& f) const noexcept {
        return !(*this == f);
    }

    //! @brief Inplace bitwise-or operator merging filter contents.
    bloom_filter& operator|=(bloom_filter const& f) noexcept {
        m_data |= f.m_data;
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        using wtype = details::word_type<std::bitset<bits>>;
        for (size_t i = 0; i < sizeof(m_data)/sizeof(wtype); ++i)
            s & ((wtype*)&m_data)[i];
        return s;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        using wtype = details::word_type<std::bitset<bits>>;
        for (size_t i = 0; i < sizeof(m_data)/sizeof(wtype); ++i)
            s << ((wtype*)&m_data)[i];
        return s;
    }

  private:
    //! @brief Type for jenkins hash.
    using jenkins_type = uint64_t;

    //! @brief The Jenkins hash
    static jenkins_type jenkins(size_t x, uint32_t seed) noexcept {
        // arbitrary starting value
        constexpr jenkins_type golden_ratio = 0x9e3779b97f4a7c13ULL;

        jenkins_type a = seed + x;
        jenkins_type b = seed;
        jenkins_type c = golden_ratio;

        a -= b; a -= c; a ^= (c >> 43);
        b -= c; b -= a; b ^= (a << 9);
        c -= a; c -= b; c ^= (b >> 8);
        a -= b; a -= c; a ^= (c >> 38);
        b -= c; b -= a; b ^= (a << 23);
        c -= a; c -= b; c ^= (b >> 5);
        a -= b; a -= c; a ^= (c >> 35);
        b -= c; b -= a; b ^= (a << 49);
        c -= a; c -= b; c ^= (b >> 11);
        a -= b; a -= c; a ^= (c >> 12);
        b -= c; b -= a; b ^= (a << 18);
        c -= a; c -= b; c ^= (b >> 22);
        return c;
    }

    //! @brief Internal data.
    std::bitset<bits> m_data;

    //! @brief Hashing object.
    H m_hash;
};


//! @brief Exchanges contents of counters.
template <size_t m, size_t bits, typename T, typename H>
void swap(bloom_filter<m,bits,T,H>& l, bloom_filter<m,bits,T,H>& r) {
    l.swap(r);
}

//! @brief Bitwise-or operator merging filter contents (copying the first argument).
template <size_t m, size_t bits, typename T, typename H>
bloom_filter<m,bits,T,H> operator|(bloom_filter<m,bits,T,H> x, bloom_filter<m,bits,T,H> const& y) noexcept {
    return x |= y;
}

//! @brief Bitwise-or operator merging filter contents (copying the second argument).
template <size_t m, size_t bits, typename T, typename H>
bloom_filter<m,bits,T,H> operator|(bloom_filter<m,bits,T,H> const& x, bloom_filter<m,bits,T,H>&& y) noexcept {
    return y |= x;
}


} // fcpp

#endif // FCPP_DATA_BLOOM_H_
