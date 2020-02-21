// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file aggregator.hpp
 * @brief Data structures printing aggregations of data to standard output.
 */

#ifndef FCPP_COMMON_AGGREGATOR_H_
#define FCPP_COMMON_AGGREGATOR_H_

#include <cassert>
#include <cmath>

#include <algorithm>
#include <array>
#include <limits>
#include <ostream>
#include <unordered_set>
#include <string>
#include <vector>

#include "lib/common/algorithm.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing data aggregators.
 */
namespace aggregator {


/**
 * @brief Invertible aggregators.
 *
 * Every operation is performed in constant time and space.
 * @{
 */
//! @brief Aggregates values by counting how many are evaluated as being `true`.
template <typename T>
class count {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    count() = default;
    
    //! @brief Combines aggregated values.
    count& operator+=(const count& o) {
        m_count += o.m_count;
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (value) m_count--;
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (value) m_count++;
    }
    
    //! @brief The results of aggregation.
    size_t result() const {
        return m_count;
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        os << "count(" << tag << ") ";
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
    
  private:
    //! @brief The counter.
    size_t m_count = 0;
};


//! @brief Aggregates values by summing them.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class sum {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    sum() = default;
    
    //! @brief Combines aggregated values.
    sum& operator+=(const sum& o) {
        m_sum += o.m_sum;
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value))
            m_sum -= value;
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum += value;
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        return m_sum;
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        os << "sum(" << tag << ") ";
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
    
  private:
    T m_sum = 0;
};


//! @brief Aggregates values by averaging.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class mean {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    mean() = default;
    
    //! @brief Combines aggregated values.
    mean& operator+=(const mean& o) {
        m_sum += o.m_sum;
        m_count += o.m_count;
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum -= value;
            m_count--;
        }
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum += value;
            m_count++;
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        return m_sum/m_count;
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        os << "mean(" << tag << ") ";
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
    
  private:
    T m_sum = 0;
    size_t m_count = 0;
};


//! @brief Aggregates values by n-th moment.
template <typename T, char n, bool only_finite = std::numeric_limits<T>::has_infinity>
class moment {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    moment() = default;
    
    //! @brief Combines aggregated values.
    moment& operator+=(const moment& o) {
        m_sum += o.m_sum;
        m_count += o.m_count;
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum -= pow(value, n);
            m_count--;
        }
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum += pow(value, n);
            m_count++;
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        return pow(m_sum/m_count, 1.0/n);
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        os << "moment" << int(n) << "(" << tag << ") ";
    }

    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
    
  private:
    T m_sum = 0;
    size_t m_count = 0;
};


//! @brief Aggregates values by mean and standard deviation.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class deviation {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    deviation() = default;
    
    //! @brief Combines aggregated values.
    deviation& operator+=(const deviation& o) {
        m_sum += o.m_sum;
        m_sqsum += o.m_sqsum;
        m_count += o.m_count;
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum -= value;
            m_sqsum -= value*value;
            m_count--;
        }
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum += value;
            m_sqsum += value*value;
            m_count++;
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        T d2 = (m_sqsum*m_count-m_sum*m_sum)/m_count/m_count;
        T d1 = sqrt(d2);
        if (std::isfinite(d1) and (d1+1)*(d1+1) <= d2) ++d1;
        return d1;
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        os << "dev(" << tag << ") ";
    }

    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
    
  private:
    T m_sum = 0;
    T m_sqsum = 0;
    size_t m_count = 0;
};


//! @brief Aggregates values by mean and standard deviation.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class stats {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    stats() = default;
    
    //! @brief Combines aggregated values.
    stats& operator+=(const stats& o) {
        m_sum += o.m_sum;
        m_sqsum += o.m_sqsum;
        m_count += o.m_count;
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum -= value;
            m_sqsum -= value*value;
            m_count--;
        }
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_sum += value;
            m_sqsum += value*value;
            m_count++;
        }
    }
    
    //! @brief The results of aggregation.
    std::tuple<T,T> result() const {
        T d2 = (m_sqsum*m_count-m_sum*m_sum)/m_count/m_count;
        T d1 = sqrt(d2);
        if (std::isfinite(d1) and (d1+1)*(d1+1) <= d2) ++d1;
        return {m_sum/m_count, d1};
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        os << "mean(" << tag << ") " << "dev(" << tag << ") ";
    }

    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        std::tuple<T,T> res = result();
        os << std::get<0>(res) << " " << std::get<1>(res) << " ";
    }
    
  private:
    T m_sum = 0;
    T m_sqsum = 0;
    size_t m_count = 0;
};
//! @}


/**
 * @brief Associative aggregators.
 *
 * Every operation is performed in constant time and space, but erasing is not supported.
 * @{
 */
//! @brief Aggregates values by taking the minimum (insert-only).
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class min {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    min() = default;
    
    //! @brief Combines aggregated values.
    min& operator+=(const min& o) {
        m_min = std::min(m_min, o.m_min);
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set (not supported).
    void erase(T) {
        assert(false);
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_min = std::min(m_min, value);
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        return m_min;
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        os << "min(" << tag << ") ";
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
    
  private:
    T m_min = std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max();
};


//! @brief Aggregates values by taking the minimum (insert-only).
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class max {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    max() = default;
    
    //! @brief Combines aggregated values.
    max& operator+=(const max& o) {
        m_max = std::max(m_max, o.m_max);
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set (not supported).
    void erase(T) {
        assert(false);
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            m_max = std::max(m_max, value);
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        return m_max;
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        os << "max(" << tag << ") ";
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
    
  private:
    T m_max = std::numeric_limits<T>::has_infinity ? -std::numeric_limits<T>::infinity() : std::numeric_limits<T>::lowest();
};
//! @}


/**
 * @brief Non-associative aggregators.
 *
 * Insert/erase are performed in constant time. Space and output operations are linear.
 * @{
 */
/**
 * @brief Aggregates values by maintaining their quantiles.
 *
 * @param insert_only Whether erase should be supported or not.
 * @param qs The required quantiles.
 */
template <typename T, bool only_finite, bool insert_only, char... qs>
class quantile;

//! @cond INTERNAL
namespace details {
    //! @brief Outputs the aggregator description for quantile.
    template <size_t n>
    void quant_header(std::ostream& os, std::string& tag, const std::array<char,n>& quantiles) {
        for (int q : quantiles) {
            if (q == 0) os << "min";
            else if (q == 100) os << "max";
            else os << "q" << int(q);
        	os << "(" << tag << ") ";
        }
    }

    //! @brief The results of aggregation for quantile.
    template <typename T, size_t n>
    std::array<T,n> quantiles(std::vector<T>& ev, const std::array<char,n>& quantiles) {
        std::array<T,n> res;
        std::vector<size_t> iv;
        for (int q : quantiles) {
            int r = q*(ev.size()-1);
            iv.push_back(r/100);
            if (r % 100 > 0) iv.push_back(r/100 + 1);
        }
        common::nth_elements(ev.begin(), ev.end(), iv.begin(), iv.end());
        for (size_t i=0; i<quantiles.size(); ++i) {
            int q = quantiles[i];
            int r = q*(ev.size()-1);
            auto v = ev.begin() + (r/100);
            r %= 100;
            res[i] = r > 0 ? ((*v)*(100-r) + (*(v+1))*r)/100 : *v;
        }
        return res;
    }
}
//! @endcond

//! @brief Implementation supporting erase.
template <typename T, bool only_finite, char... qs>
class quantile<T, only_finite, false, qs...> {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    quantile() = default;
    
    //! @brief Combines aggregated values.
    quantile& operator+=(const quantile& o) {
        m_values.insert(o.m_values.begin(), o.m_values.end());
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value))
            m_values.erase(m_values.find(value));
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value))
            m_values.insert(value);
    }
    
    //! @brief The results of aggregation.
    std::array<T,sizeof...(qs)> result() const {
        std::vector<T> ev(m_values.begin(), m_values.end());
        return details::quantiles(ev, m_quantiles);
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        details::quant_header(os, tag, m_quantiles);
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        for (T x : result())
            os << x << " ";
    }
    
  private:
    const std::array<char, sizeof...(qs)> m_quantiles = {qs...};
    std::unordered_multiset<T> m_values;
};

//! @brief Implementation not supporting erase.
template <typename T, bool only_finite, char... qs>
class quantile<T, only_finite, true, qs...> {
  public:
    //! @brief The type of values aggregated.
    using type = T;
    
    //! @brief Default constructor.
    quantile() = default;
    
    //! @brief Combines aggregated values.
    quantile& operator+=(const quantile& o) {
        m_values.insert(m_values.end(), o.m_values.begin(), o.m_values.end());
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T) {
        assert(false);
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value))
            m_values.push_back(value);
    }
    
    //! @brief The results of aggregation.
    std::array<T,sizeof...(qs)> result() {
        return details::quantiles(m_values, m_quantiles);
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        details::quant_header(os, tag, m_quantiles);
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        for (T x : result())
            os << x << " ";
    }
    
  private:
    const std::array<char, sizeof...(qs)> m_quantiles = {qs...};
    std::vector<T> m_values;
};


//! @brief Aggregates values by maintaining their minimum.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity, bool insert_only = false>
using minimum = quantile<T,only_finite,insert_only,0>;


//! @brief Aggregates values by maintaining their median.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity, bool insert_only = false>
using median = quantile<T,only_finite,insert_only,50>;


//! @brief Aggregates values by maintaining their maximum.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity, bool insert_only = false>
using maximum = quantile<T,only_finite,insert_only,100>;


//! @brief Aggregates values by maintaining their minimum, 25% quartile, median, 75% quartile and maximum.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity, bool insert_only = false>
using quartile = quantile<T,only_finite,insert_only,0,25,50,75,100>;
//! @}


/**
 * @brief Chains multiple aggregators together into a single object.
 *
 * Uses the value type of the first aggregator.
 * Supports erase only if supported by every aggregator.
 */
template <typename... Ts>
class combine : public Ts... {
  public:
    using type = typename common::type_sequence<Ts...>::front::type;
    
    //! @brief Default constructor.
    combine() = default;
    
    //! @brief Combines aggregated values.
    combine& operator+=(const combine& o) {
        common::details::ignore(Ts::operator+=(o)...);
        return *this;
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(type value) {
        common::details::ignore((Ts::erase(value),0)...);
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(type value) {
        common::details::ignore((Ts::insert(value),0)...);
    }
    
    //! @brief The results of aggregation.
    auto result() const {
        return std::make_tuple(Ts::result()...);
    }
    
    //! @brief Outputs the aggregator description.
    void header(std::ostream& os, std::string tag) const {
        header_impl(os, tag, common::type_sequence<Ts...>());
    }

    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        output_impl(os, common::type_sequence<Ts...>());
    }
    
  private:
    //! @brief Outputs the aggregator description.
    template <typename S, typename... Ss>
    void header_impl(std::ostream& os, std::string& tag, common::type_sequence<S,Ss...>) const {
        S::header(os, tag);
        header_impl(os, tag, common::type_sequence<Ss...>());
    }
    void header_impl(std::ostream&, std::string&, common::type_sequence<>) const {}
    
    //! @brief Printed results of aggregation.
    template <typename S, typename... Ss>
    void output_impl(std::ostream& os, common::type_sequence<S,Ss...>) const {
        S::output(os);
        header_impl(os, common::type_sequence<Ss...>());
    }
    void output_impl(std::ostream&, common::type_sequence<>) const {}
};


}


}

#endif // FCPP_COMMON_AGGREGATOR_H_
