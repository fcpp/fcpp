// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file aggregator.hpp
 * @brief Data structures printing aggregations of data to standard output.
 */

#ifndef FCPP_OPTION_AGGREGATOR_H_
#define FCPP_OPTION_AGGREGATOR_H_

#include <cassert>
#include <cmath>

#include <algorithm>
#include <array>
#include <limits>
#include <ostream>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <tuple>
#include <vector>

#include "lib/settings.hpp"
#include "lib/common/algorithm.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/option/filter.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing data aggregators.
 */
namespace aggregator {


//! @cond INTERNAL
namespace details {
    inline std::string header(std::string const&) {
        return "";
    }
    template <typename... Ts>
    inline std::string header(std::string const& tag, std::string stat, Ts... xs) {
#ifdef ALCHEMIST
        return tag + "[" + stat + "] " + header(tag, xs...);
#else
        return stat + "(" + tag + ") " + header(tag, xs...);
#endif
    }
}
//! @endcond


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
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<count<U>, size_t>;

    //! @brief Default constructor.
    count() = default;

    //! @brief Combines aggregated values.
    count& operator+=(count const& o) {
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
    template <typename U>
    result_type<U> result() const {
        return {m_count};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "count";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(count const& o) const {
        return m_count == o.m_count;
    }

    //! @brief Inequality operator.
    bool operator!=(count const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_count;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_count;
    }

  private:
    //! @brief The counter.
    size_t m_count = 0;
};


//! @brief Aggregates values by counting how many distinct values are present.
template <typename T, int deprecated = 2>
class distinct {
    static_assert(deprecated == 2, "the only_finite template argument to aggregator::distinct should be removed");

  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<distinct<U, deprecated>, size_t>;

    //! @brief Default constructor.
    distinct() = default;

    //! @brief Combines aggregated values.
    distinct& operator+=(distinct const& o) {
        for (auto const& x : o.m_counts)
            m_counts[x.first] += x.second;
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (--m_counts.at(value) == 0) m_counts.erase(value);
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        ++m_counts[value];
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        return {m_counts.size()};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "distinct";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(distinct const& o) const {
        return m_counts == o.m_counts;
    }

    //! @brief Inequality operator.
    bool operator!=(distinct const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_counts;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_counts;
    }

  private:
    //! @brief Counters for every distinct item.
    std::unordered_map<T,size_t> m_counts;
};


//! @brief Aggregates values by listing them in aggregation order (erasing is not supported, combination is only supported sequentially).
template <typename T>
class list {
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<list<U>, std::vector<T>>;

    //! @brief Default constructor.
    list() = default;

    //! @brief Combines aggregated values (cannot combine with a combination).
    list& operator+=(list const& o) {
        assert(o.m_items.size() == 1);
        m_items.push_back(std::move(o.m_items[0]));
        return *this;
    }

    //! @brief Erases a value from the aggregation set (not supported).
    void erase(T) {
        assert(false);
    }

    //! @brief Inserts a new value to be aggregated (disabled on combines lists).
    void insert(T value) {
        if (m_items.empty()) m_items.emplace_back();
        assert(m_items.size() == 1);
        m_items[0].push_back(value);
    }

    //! @brief The results of aggregation (assumes lists have similar lengths).
    template <typename U>
    result_type<U> result() const {
        for (size_t t = 1; t < m_items.size(); ++t)
            assert(m_items[t-1].size() >= m_items[t].size() and m_items[t].size() >= m_items[0].size() - 1);
        std::vector<T> v;
        for (size_t i = 0; i < m_items.back().size(); ++i)
            for (size_t t = 0; t < m_items.size(); ++t)
                v.push_back(m_items[t][i]);
        for (size_t t = 0; m_items.back().size() < m_items[t].size(); ++t)
            v.push_back(m_items[t].back());
        return {std::move(v)};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "list";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(list const& o) const {
        return m_items == o.m_items;
    }

    //! @brief Inequality operator.
    bool operator!=(list const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_items;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_items;
    }

  private:
    //! @brief The list of items (in a per-thread basis).
    std::vector<std::vector<T>> m_items;
};


//! @brief Aggregates values by summing them.
template <typename T, int deprecated = 2>
class sum {
    static_assert(deprecated == 2, "the only_finite template argument to aggregator::sum should be removed, use aggregator::only_finite if needed");
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<sum<U, deprecated>, T>;

    //! @brief Default constructor.
    sum() = default;

    //! @brief Combines aggregated values.
    sum& operator+=(sum const& o) {
        m_sum += o.m_sum;
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        m_sum -= value;
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_sum += value;
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        return {m_sum};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "sum";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(sum const& o) const {
        return m_sum == o.m_sum;
    }

    //! @brief Inequality operator.
    bool operator!=(sum const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_sum;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_sum;
    }

  private:
    T m_sum = 0;
};


//! @brief Aggregates values by averaging.
template <typename T, int deprecated = 2>
class mean {
    static_assert(deprecated == 2, "the only_finite template argument to aggregator::mean should be removed, use aggregator::only_finite if needed");
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<mean<U, deprecated>, T>;

    //! @brief Default constructor.
    mean() = default;

    //! @brief Combines aggregated values.
    mean& operator+=(mean const& o) {
        m_sum += o.m_sum;
        m_count += o.m_count;
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        m_sum -= value;
        m_count--;
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_sum += value;
        m_count++;
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        return {m_count == 0 ? std::numeric_limits<T>::quiet_NaN() : m_sum/m_count};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "mean";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(mean const& o) const {
        return m_sum == o.m_sum and m_count == o.m_count;
    }

    //! @brief Inequality operator.
    bool operator!=(mean const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_sum & m_count;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_sum << m_count;
    }

  private:
    T m_sum = 0;
    size_t m_count = 0;
};


//! @brief Aggregates values by n-th moment.
template <typename T, char n, int deprecated = 2>
class moment {
    static_assert(deprecated == 2, "the only_finite template argument to aggregator::moment should be removed, use aggregator::only_finite if needed");
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<moment<U, n, deprecated>, T>;

    //! @brief Default constructor.
    moment() = default;

    //! @brief Combines aggregated values.
    moment& operator+=(moment const& o) {
        m_sum += o.m_sum;
        m_count += o.m_count;
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        m_sum -= pow(value, n);
        m_count--;
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_sum += pow(value, n);
        m_count++;
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        return {m_count == 0 ? std::numeric_limits<T>::quiet_NaN() : pow(m_sum/m_count, real_t(1)/n)};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "moment" + std::to_string(int{n});
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(moment const& o) const {
        return m_sum == o.m_sum and m_count == o.m_count;
    }

    //! @brief Inequality operator.
    bool operator!=(moment const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_sum & m_count;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_sum << m_count;
    }

  private:
    T m_sum = 0;
    size_t m_count = 0;
};


//! @brief Aggregates values by standard deviation.
template <typename T, int deprecated = 2>
class deviation {
    static_assert(deprecated == 2, "the only_finite template argument to aggregator::deviation should be removed, use aggregator::only_finite if needed");
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<deviation<U, deprecated>, T>;

    //! @brief Default constructor.
    deviation() = default;

    //! @brief Combines aggregated values.
    deviation& operator+=(deviation const& o) {
        m_sum += o.m_sum;
        m_sqsum += o.m_sqsum;
        m_count += o.m_count;
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        m_sum -= value;
        m_sqsum -= value*value;
        m_count--;
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_sum += value;
        m_sqsum += value*value;
        m_count++;
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        if (m_count == 0) return {std::numeric_limits<T>::quiet_NaN()};
        T d2 = (m_sqsum*m_count-m_sum*m_sum)/m_count/m_count;
        T d1 = sqrt(d2);
        if (std::isfinite(d1) and (d1+1)*(d1+1) <= d2) ++d1;
        return {d1};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "dev";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(deviation const& o) const {
        return m_sum == o.m_sum and m_sqsum == o.m_sqsum and m_count == o.m_count;
    }

    //! @brief Inequality operator.
    bool operator!=(deviation const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_sum & m_sqsum & m_count;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_sum << m_sqsum << m_count;
    }

  private:
    T m_sum = 0;
    T m_sqsum = 0;
    size_t m_count = 0;
};


//! @brief Aggregates values by mean and standard deviation.
template <typename T, int deprecated = 2>
class stats {
    static_assert(deprecated == 2, "the only_finite template argument to aggregator::stats should be removed, use aggregator::only_finite if needed");
  public:
    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<mean<U, deprecated>, T, deviation<U, deprecated>, T>;

    //! @brief Default constructor.
    stats() = default;

    //! @brief Combines aggregated values.
    stats& operator+=(stats const& o) {
        m_sum += o.m_sum;
        m_sqsum += o.m_sqsum;
        m_count += o.m_count;
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        m_sum -= value;
        m_sqsum -= value*value;
        m_count--;
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_sum += value;
        m_sqsum += value*value;
        m_count++;
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        if (m_count == 0) return {std::numeric_limits<T>::quiet_NaN(), std::numeric_limits<T>::quiet_NaN()};
        T d2 = (m_sqsum*m_count-m_sum*m_sum)/m_count/m_count;
        T d1 = sqrt(d2);
        if (std::isfinite(d1) and (d1+1)*(d1+1) <= d2) ++d1;
        return {m_sum/m_count, d1};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "mean-dev";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, "mean", "dev");
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        auto res = result<void>();
        os << std::get<0>(res) << " " << std::get<1>(res) << " ";
    }

    //! @brief Equality operator.
    bool operator==(stats const& o) const {
        return m_sum == o.m_sum and m_sqsum == o.m_sqsum and m_count == o.m_count;
    }

    //! @brief Inequality operator.
    bool operator!=(stats const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_sum & m_sqsum & m_count;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_sum << m_sqsum << m_count;
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
template <typename T, int deprecated = 2>
class min {
    static_assert(deprecated == 2, "the only_finite template argument to aggregator::min should be removed, use aggregator::only_finite if needed");
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<min<U, deprecated>, T>;

    //! @brief Default constructor.
    min() = default;

    //! @brief Combines aggregated values.
    min& operator+=(min const& o) {
        m_min = std::min(m_min, o.m_min);
        return *this;
    }

    //! @brief Erases a value from the aggregation set (not supported).
    void erase(T) {
        assert(false);
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_min = std::min(m_min, value);
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        return {m_min};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "min";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(min const& o) const {
        return m_min == o.m_min;
    }

    //! @brief Inequality operator.
    bool operator!=(min const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_min;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_min;
    }

  private:
    T m_min = std::numeric_limits<T>::has_infinity ? std::numeric_limits<T>::infinity() : std::numeric_limits<T>::max();
};


//! @brief Aggregates values by taking the maximum (insert-only).
template <typename T, int deprecated = 2>
class max {
    static_assert(deprecated == 2, "the only_finite template argument to aggregator::max should be removed, use aggregator::only_finite if needed");
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_t<max<U, deprecated>, T>;

    //! @brief Default constructor.
    max() = default;

    //! @brief Combines aggregated values.
    max& operator+=(max const& o) {
        m_max = std::max(m_max, o.m_max);
        return *this;
    }

    //! @brief Erases a value from the aggregation set (not supported).
    void erase(T) {
        assert(false);
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_max = std::max(m_max, value);
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        return {m_max};
    }

    //! @brief The aggregator name.
    static std::string name() {
        return "max";
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, name());
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        os << std::get<0>(result<void>()) << " ";
    }

    //! @brief Equality operator.
    bool operator==(max const& o) const {
        return m_max == o.m_max;
    }

    //! @brief Inequality operator.
    bool operator!=(max const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_max;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_max;
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
template <typename T, bool insert_only, char... qs>
class quantile;

//! @cond INTERNAL
namespace details {
    //! @brief The aggregator description for a quantile.
    inline std::string quant_repr(char q) {
        if (q == 0)   return "min";
        if (q == 100) return "max";
        return "q" + std::to_string(int{q});
    }

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U, typename T, bool insert_only, char... qs>
    using quantile_result_type = common::tagged_tuple<common::type_sequence<quantile<U, insert_only, qs>...>, common::type_sequence<std::enable_if_t<qs==qs, T>...>>;

    //! @brief The results of aggregation for quantile.
    template <typename T, size_t n>
    std::array<T,n> quantiles(std::vector<T>& ev, std::array<char,n> const& quantiles) {
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

    //! @brief The results of aggregation for quantile as tuple.
    template <typename U, typename T, bool insert_only, char... qs, size_t... is>
    quantile_result_type<U, T, insert_only, qs...> quantiles_tuple(quantile<T, insert_only, qs...> const&, std::vector<T>& ev, std::array<char,sizeof...(qs)> const& quantiles, std::index_sequence<is...>) {
        std::array<T,sizeof...(qs)> r = details::quantiles(ev, quantiles);
        return {r[is]...};
    }

    //! @brief Prints the results of aggregation for quantile (empty case).
    template <typename O, typename T>
    void quantile_output(O&, T&&, std::index_sequence<>) {}

    //! @brief Prints the results of aggregation for quantile.
    template <typename O, typename T, size_t i, size_t... is>
    void quantile_output(O& os, T&& r, std::index_sequence<i, is...>) {
        os << std::get<i>(r) << " ";
        quantile_output(os, r, std::index_sequence<is...>{});
    }
}
//! @endcond

//! @brief Implementation supporting erase.
template <typename T, char... qs>
class quantile<T, false, qs...> {
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = details::quantile_result_type<U, T, false, qs...>;

    //! @brief Default constructor.
    quantile() = default;

    //! @brief Combines aggregated values.
    quantile& operator+=(quantile const& o) {
        m_values.insert(o.m_values.begin(), o.m_values.end());
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        m_values.erase(m_values.find(value));
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_values.insert(value);
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        std::vector<T> ev(m_values.begin(), m_values.end());
        return details::quantiles_tuple<U>(*this, ev, m_quantiles, std::make_index_sequence<sizeof...(qs)>{});
    }

    //! @brief The aggregator name.
    static std::string name() {
        std::array<std::string, sizeof...(qs)> v = {details::quant_repr(qs)...};
        for (size_t i = 1; i < v.size(); ++i) v[0] += "-" + v[i];
        return v[0];
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, details::quant_repr(qs)...);
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        details::quantile_output(os, result<void>(), std::make_index_sequence<sizeof...(qs)>{});
    }

    //! @brief Equality operator.
    bool operator==(quantile const& o) const {
        return m_values == o.m_values;
    }

    //! @brief Inequality operator.
    bool operator!=(quantile const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_values;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_values;
    }

  private:
    std::array<char, sizeof...(qs)> const m_quantiles = {qs...};
    std::unordered_multiset<T> m_values;
};

//! @brief Implementation not supporting erase.
template <typename T, char... qs>
class quantile<T, true, qs...> {
  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of values aggregated.
    using type = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = details::quantile_result_type<U, T, true, qs...>;

    //! @brief Default constructor.
    quantile() = default;

    //! @brief Combines aggregated values.
    quantile& operator+=(quantile const& o) {
        m_values.insert(m_values.end(), o.m_values.begin(), o.m_values.end());
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(T) {
        assert(false);
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        m_values.push_back(value);
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        return details::quantiles_tuple<U>(*this, (std::vector<T>&)m_values, m_quantiles, std::make_index_sequence<sizeof...(qs)>{});
    }

    //! @brief The aggregator name.
    static std::string name() {
        std::array<std::string, sizeof...(qs)> v = {details::quant_repr(qs)...};
        for (size_t i = 1; i < v.size(); ++i) v[0] += "-" + v[i];
        return v[0];
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        os << details::header(tag, details::quant_repr(qs)...);
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        details::quantile_output(os, result<void>(), std::make_index_sequence<sizeof...(qs)>{});
    }

    //! @brief Equality operator.
    bool operator==(quantile const& o) const {
        return m_values == o.m_values;
    }

    //! @brief Inequality operator.
    bool operator!=(quantile const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_values;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_values;
    }

  private:
    std::array<char, sizeof...(qs)> const m_quantiles = {qs...};
    std::vector<T> m_values;
};


//! @brief Aggregates values by maintaining their minimum.
template <typename T, bool insert_only = false>
using minimum = quantile<T,insert_only,0>;


//! @brief Aggregates values by maintaining their median.
template <typename T, bool insert_only = false>
using median = quantile<T,insert_only,50>;


//! @brief Aggregates values by maintaining their maximum.
template <typename T, bool insert_only = false>
using maximum = quantile<T,insert_only,100>;


//! @brief Aggregates values by maintaining their minimum, 25% quartile, median, 75% quartile and maximum.
template <typename T, bool insert_only = false>
using quartile = quantile<T,insert_only,0,25,50,75,100>;
//! @}


/**
 * @brief Chains multiple aggregators together into a single object.
 *
 * Uses the value type of the first aggregator.
 * Supports erase only if supported by every aggregator.
 */
template <typename... Ts>
class combine {
  public:
    //! @brief The type of values aggregated.
    using type = typename common::type_sequence<Ts...>::front::type;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = common::tagged_tuple_cat<typename Ts::template result_type<U>...>;

    //! @brief Default constructor.
    combine() = default;

    //! @brief Combines aggregated values.
    combine& operator+=(combine const& o) {
        common::ignore_args((std::get<Ts>(m_aggregators)+=std::get<Ts>(o.m_aggregators))...);
        return *this;
    }

    //! @brief Erases a value from the aggregation set.
    void erase(type value) {
        common::ignore_args((std::get<Ts>(m_aggregators).erase(value),0)...);
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(type value) {
        common::ignore_args((std::get<Ts>(m_aggregators).insert(value),0)...);
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        result_type<U> r;
        common::ignore_args((r = std::get<Ts>(m_aggregators).template result<U>())...);
        return r;
    }

    //! @brief The aggregator name.
    static std::string name() {
        std::array<std::string, sizeof...(Ts)> v = {Ts::name()...};
        for (size_t i = 1; i < v.size(); ++i) v[0] += "-" + v[i];
        return v[0];
    }

    //! @brief Outputs the aggregator description.
    template <typename O>
    void header(O& os, std::string tag) const {
        header_impl(os, tag, common::type_sequence<Ts...>{});
    }

    //! @brief Printed results of aggregation.
    template <typename O>
    void output(O& os) const {
        output_impl(os, common::type_sequence<Ts...>{});
    }

    //! @brief Equality operator.
    bool operator==(combine const& o) const {
        return m_aggregators == o.m_aggregators;
    }

    //! @brief Inequality operator.
    bool operator!=(combine const& o) const {
        return !(*this == o);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_aggregators;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_aggregators;
    }

  private:
    //! @brief Outputs the aggregator description.
    template <typename O, typename S, typename... Ss>
    void header_impl(O& os, std::string& tag, common::type_sequence<S,Ss...>) const {
        std::get<S>(m_aggregators).header(os, tag);
        header_impl(os, tag, common::type_sequence<Ss...>());
    }
    template <typename O>
    void header_impl(O&, std::string&, common::type_sequence<>) const {}

    //! @brief Printed results of aggregation.
    template <typename O, typename S, typename... Ss>
    void output_impl(O& os, common::type_sequence<S,Ss...>) const {
        std::get<S>(m_aggregators).output(os);
        output_impl(os, common::type_sequence<Ss...>());
    }
    template <typename O>
    void output_impl(O&, common::type_sequence<>) const {}

    //! @brief The aggregators to be combined.
    std::tuple<Ts...> m_aggregators;
};


/**
 * @brief Aggregates containers of values through a value aggregator.
 *
 * @tparam T The container type.
 * @tparam A The value aggregator type.
 */
template <typename T, typename A>
class container : public A {
  public:
    //! @brief The type of values aggregated.
    using type = T;

    //! @brief Erases a value from the aggregation set.
    void erase(T const& values) {
        for (auto const& value: values) A::erase(value);
    }

    //! @brief Inserts a new value to be aggregated.
    void insert(T const& values) {
        for (auto const& value: values) A::insert(value);
    }
};


/**
 * @brief Filters only values respecting filter F before feeding them to another aggregator.
 *
 * @tparam F The filter predicate type.
 * @tparam A The value aggregator type.
 */
template <typename F, typename A>
class filter : public A {
    //! @brief The type of values aggregated.
    using T = typename A::type;

    template <typename T>
    struct remap_tuple;

    template <typename... Ss, typename... Ts>
    struct remap_tuple<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Ts...>>> {
        using type = common::tagged_tuple<common::type_sequence<filter<F,Ss>...>, common::type_sequence<Ts...>>;
    };

  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = typename remap_tuple<typename A::template result_type<U>>::type;

    //! @brief Default constructor.
    filter() = default;

    //! @brief Erases a value from the aggregation set.
    inline void erase(T const& value) {
        if (m_filter(value)) A::erase(value);
    }

    //! @brief Inserts a new value to be aggregated.
    inline void insert(T const& value) {
        if (m_filter(value)) A::insert(value);
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        auto t = A::template result<U>();
        return reinterpret_cast<result_type<U>&>(t);
    }

    //! @brief The aggregator name.
    static std::string name() {
        std::string fs = F::name();
        std::string as = A::name();
        std::string s = fs + ' ';
        for (char c : as)
            if (c == '-') s += '-' + fs + ' ';
            else s.push_back(c);
        return s;
    }

  private:
    //! @brief The callable filter class.
    F m_filter;
};


/**
 * @brief Filters only finite values before feeding them to another aggregator.
 *
 * @tparam A The value aggregator type.
 */
template <typename A>
using only_finite = filter<fcpp::filter::finite, A>;


/**
 * @brief Maps a functor to values before feeding them to another aggregator.
 *
 * @tparam F The callable class functor type.
 * @tparam A The value aggregator type.
 */
template <typename F, typename A>
class mapper : public A {
    //! @brief The type of values aggregated.
    using T = typename A::type;

    template <typename T>
    struct remap_tuple;

    template <typename... Ss, typename... Ts>
    struct remap_tuple<common::tagged_tuple<common::type_sequence<Ss...>, common::type_sequence<Ts...>>> {
        using type = common::tagged_tuple<common::type_sequence<mapper<F,Ss>...>, common::type_sequence<Ts...>>;
    };

  public:
    //! @brief The tag aggregated from result type.
    using tag = T;

    //! @brief The type of the aggregation result, given the tag of the aggregated values.
    template <typename U>
    using result_type = typename remap_tuple<typename A::template result_type<U>>::type;

    //! @brief Default constructor.
    mapper() = default;

    //! @brief Erases a value from the aggregation set.
    inline void erase(T const& value) {
        A::erase(m_functor(value));
    }

    //! @brief Inserts a new value to be aggregated.
    inline void insert(T const& value) {
        A::insert(m_functor(value));
    }

    //! @brief The results of aggregation.
    template <typename U>
    result_type<U> result() const {
        auto t = A::template result<U>();
        return reinterpret_cast<result_type<U>&>(t);
    }

    //! @brief The aggregator name.
    static std::string name() {
        std::string fs = common::type_name<F>();
        std::string as = A::name();
        std::string s = fs + ' ';
        for (char c : as)
            if (c == '-') s += '-' + fs + ' ';
            else s.push_back(c);
        return s;
    }

  private:
    //! @brief The callable filter class.
    F m_functor;
};


}


}

#endif // FCPP_OPTION_AGGREGATOR_H_
