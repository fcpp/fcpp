// Copyright © 2021 Giorgio Audrito and Gianluca Torta. All Rights Reserved.

/**
 * @file hyperloglog.hpp
 * @brief Implementation of the `hyperloglog_counter` class template for statistical size estimates.
 */

#ifndef FCPP_DATA_HYPERLOGLOG_H_
#define FCPP_DATA_HYPERLOGLOG_H_

#include <climits>
#include <cmath>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <utility>

#include "lib/settings.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

namespace {
    //! @brief Number of bits used by an integer.
    constexpr size_t bit_size(size_t x) {
        size_t r = 0;
        for (; (size_t(1)<<r) < x; ++r);
        return r;
    }

    //! @brief Initialize the msb mask for the broadword code in insert(counter)
    constexpr size_t get_msbMask(size_t register_bit_size, size_t word_bit_size) {
        size_t mask = 0;
        for (size_t i = register_bit_size - 1; i < word_bit_size; i += register_bit_size)
            mask |= size_t(1) << i;
        return mask;
    }

    //! @brief Initialize the lsb mask for the broadword code in insert(counter)
    constexpr size_t get_lsbMask(size_t register_bit_size, size_t word_bit_size) {
        size_t mask = 0;
        for (size_t i = 0; i < word_bit_size-register_bit_size; i += register_bit_size)
            mask |= size_t(1) << i;
        return mask;
    }

    //! @brief Returns 0.5^e through bit manipulations.
    inline real_t half_power(size_t e) {
        constexpr size_t real_bits = sizeof(real_t)*CHAR_BIT;
        constexpr size_t expbit = real_bits == 32 ? 23 : 52;
        constexpr size_t expoff = real_bits == 32 ? 127 : 1023;
        using int_t = std::conditional_t<real_bits == 32, uint32_t, uint64_t>;

        real_t x;
        reinterpret_cast<int_t&>(x) = (expoff-e) << expbit;
        return x;
    }
}

/**
 * @brief A hyperloglog counter data structure.
 *
 * It allows for insertion of elements and of whole counters, while
 * providing approximated set size estimates.
 *
 * @param m     The number of registers.
 * @param bits  The size in bits of each register (defaults to 4).
 * @param seed  The seed for the Jenkins hash (deafults to 0).
 * @param T     The type of values to be inserted in the structure.
 * @param H     An callable class hashing T objects to a size_t value.
 */
template <size_t m, size_t bits = 4, size_t seed = 0, typename T = size_t, typename H = std::hash<T>>
class hyperloglog_counter {
  public:
    //! @brief The type of elements.
    using key_type = T;
    //! @brief The type of elements.
    using value_type = T;
    //! @brief The hasher type.
    using hasher = H;
    //! @brief The type for sizes.
    using size_type = size_t;

    //! @brief The number of registers.
    constexpr static size_t registers = m;
    //! @brief The register size in bits.
    constexpr static size_t register_bit_size = bits;
    //! @brief The word size in bits.
    constexpr static size_t word_bit_size = sizeof(size_t)*CHAR_BIT;
    //! @brief The word size in registers.
    constexpr static size_t word_reg_size = word_bit_size / register_bit_size;
    //! @brief The counter size in words.
    constexpr static size_t counter_word_size = (registers + word_reg_size - 1) / word_reg_size;

    //! @brief Constructs an empty counter.
    hyperloglog_counter(H const& h = H()) {
        m_hash = h;
    }

    //! @brief Constructs a counter with an element.
    hyperloglog_counter(T const& val, H const& h = H()) : hyperloglog_counter(h) {
        insert(val);
    }

    //! @brief Constructs a counter with a range of elements.
    template <typename I>
    hyperloglog_counter(I first, I last, H const& h = H()) : hyperloglog_counter(h) {
        insert(first, last);
    }

    //! @brief Constructs a counter with a list of elements.
    hyperloglog_counter(std::initializer_list<T> il, H const& h = H()) : hyperloglog_counter(h) {
        insert(il);
    }

    //! @brief Copy constructor.
    hyperloglog_counter(hyperloglog_counter const& c) = default;

    //! @brief Move constructor.
    hyperloglog_counter(hyperloglog_counter&& c) = default;

    //! @brief Copy assignment.
    hyperloglog_counter& operator=(hyperloglog_counter const& c) = default;

    //! @brief Move assignment.
    hyperloglog_counter& operator=(hyperloglog_counter&& c) = default;

    //! @brief List assignment.
    hyperloglog_counter& operator=(std::initializer_list<T> il) {
        clear();
        insert(il);
        return *this;
    }

    //! @brief Returns whether the counter is empty.
    bool empty() const noexcept {
        for (size_t i=0; i<counter_word_size; ++i)
            if (m_data[i]) return false;
        return true;
    }

    //! @brief Returns the estimated number of elements.
    real_t size() const noexcept {
        // conversion factor from inverse register value to size estimate.
        constexpr real_t alphaMM =
            m <= 16 ? 0.673 * m * m :
            m <= 32 ? 0.697 * m * m :
            m <= 64 ? 0.709 * m * m : (0.7213 / (1 + 1.079 / m)) * m * m;

        size_t rval;
        size_t zeros = 0;
        real_t s = 0.0;
        for (size_t j=0; j<registers; j++) {
            rval = getreg(j);
            if (rval == 0) zeros++;
            s += half_power(rval);
        }
        s = alphaMM / s;
        if (zeros && s < 2.5 * registers)
            return registers * log((double)registers / zeros);
        else return s;
    }

    //! @brief Constructs and inserts a single element.
    template <typename... Ts>
    void emplace(Ts&&... xs) {
        insert(T(std::forward<Ts>(xs)...));
    }

    //! @brief Inserts a single element.
    void insert(T const& val) {
        // mask ensuring that the number of trailing zeros fits inside a register.
        constexpr jenkins_type sentinelMask = jenkins_type(1) << (regMask - 1);

        jenkins_type rest = jenkins(m_hash(val));
        size_t j = rest % m;
        rest = (rest / m) | sentinelMask;
        size_t r = 1;
        while (!(rest % 2)) {
            rest >>= 1;
            ++r;
        }
        maxreg(j, r);
    }

    //! @brief Inserts collection of elements.
    void insert(hyperloglog_counter const& c) {
        size_t accumulator[counter_word_size];
        size_t mask[counter_word_size];

        for (size_t i=counter_word_size; i-- != 0;) accumulator[i] = c.m_data[i] | msbMask;
        for (size_t i=counter_word_size; i-- != 0;) mask[i] = m_data[i] & ~msbMask;
        subtract(accumulator, mask, counter_word_size);
        for (size_t i=counter_word_size; i-- != 0;)
            accumulator[i] = ((accumulator[i] | (c.m_data[i] ^ m_data[i])) ^ (c.m_data[i] | ~m_data[i])) & msbMask;
        for (size_t i=counter_word_size-1; i-- != 0;) mask[i] = accumulator[i] >> (register_bit_size-1) | accumulator[i+1] << (word_bit_size-register_bit_size+1) | msbMask;
        mask[counter_word_size-1] = accumulator[counter_word_size-1] >> (register_bit_size-1) | msbMask;
        subtract(mask, lsbMask, counter_word_size);
        for (size_t i=counter_word_size; i-- != 0;) mask[i] = (mask[i] | msbMask) ^ accumulator[i];
        for (size_t i=counter_word_size; i-- != 0;) m_data[i] ^= (m_data[i] ^ c.m_data[i]) & mask[i];
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

    //! @brief Clear content.
    void clear() noexcept {
        for (size_t i=0; i<counter_word_size; ++i)
            m_data[i] = 0;
    }

    //! @brief Swap content.
    void swap(hyperloglog_counter& c) {
        swap(m_data, c.m_data);
        swap(m_hash, c.m_hash);
    }

    //! @brief Get the hash function.
    H hash_function() const {
        return m_hash;
    }

    //! @brief Equality operator.
    bool operator==(hyperloglog_counter const& c) const {
        for (size_t i=0; i<counter_word_size; ++i)
            if (m_data[i] != c.m_data[i]) return false;
        return true;
    }

    //! @brief Inequality operator.
    bool operator!=(hyperloglog_counter const& c) const {
        return !(*this == c);
    }

    //! @brief Estimated relative error of the counter.
    constexpr static real_t error() {
        return 1.06/sqrt(m);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_data;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_data;
    }

  protected:
    //! @brief Mask of bits for a register.
    constexpr static size_t regMask = (size_t(1) << register_bit_size) - 1;
    //! @brief Masks with max set bits for each register.
    constexpr static size_t msbMask = get_msbMask(register_bit_size, word_bit_size);
    //! @brief Masks with least set bits for each register.
    constexpr static size_t lsbMask = get_lsbMask(register_bit_size, word_bit_size);

    //! @brief Gets the content of a single register.
    size_t getreg(size_t reg) const {
        size_t idx = reg / word_reg_size;
        size_t offset = reg % word_reg_size;
        offset *= register_bit_size;
        return (m_data[idx] >> offset) & regMask;
    }

    //! @brief Maximises the content of a single register with a value.
    void maxreg(size_t reg, size_t val) {
        size_t idx = reg / word_reg_size;
        size_t offset = reg % word_reg_size;
        offset *= register_bit_size;
        size_t v = (m_data[idx] >> offset) & regMask;
        if (val > v) m_data[idx] ^= (val ^ v) << offset;
    }

    //! @brief Type for jenkins hash.
    using jenkins_type = uint64_t;

    //! @brief The Jenkins hash.
    static jenkins_type jenkins(size_t x) {
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

    //! @brief Performs a multiple precision subtraction, leaving the result in the first operand.
    static void subtract(size_t x[], size_t const y[], size_t l) {
        bool borrow = false;
        for (size_t i=0; i<l; i++) {
            if (!borrow || x[i]--!=0) borrow = x[i]<y[i];
            x[i]-=y[i];
        }
    }

    //! @brief Performs a multiple precision subtraction with a repeating word, leaving the result in the first operand.
    static void subtract(size_t x[], size_t y, size_t l) {
        bool borrow = false;
        for (size_t i=0; i<l; i++) {
            if (!borrow || x[i]--!=0) borrow = x[i]<y;
            x[i]-=y;
        }
    }

  private:
    //! @brief Internal data.
    size_t m_data[counter_word_size] = {};

    //! @brief Hashing object.
    H m_hash;
};


//! @brief Exchanges contents of counters.
template <size_t m, size_t bits, size_t seed, typename T, typename H>
void swap(hyperloglog_counter<m,bits,seed,T,H>& l, hyperloglog_counter<m,bits,seed,T,H>& r) {
    l.swap(r);
}


//! @brief Compute the number of registers to use from an error threshold.
constexpr size_t register_error(real_t error) {
    return 1.12/(error*error) + 1 - 1e9;
}


}

#endif // FCPP_DATA_HYPERLOGLOG_H_
