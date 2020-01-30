// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file aggregator.hpp
 * @brief Data structures printing aggregations of data to standard output.
 */

#ifndef FCPP_GENERATE_AGGREGATOR_H_
#define FCPP_GENERATE_AGGREGATOR_H_

#include <cmath>

#include <array>
#include <limits>
#include <ostream>
#include <set>
#include <string>
#include <vector>
#include <iostream>

#include "lib/common/algorithm.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Aggregates values by counting how many are evaluated as being `true`.
template <typename T>
class count_aggregator {
    size_t count = 0;
    
  public:
    //! @brief Constructs the aggregator object and outputs its description.
    count_aggregator(std::ostream& os, std::string tag) {
        os << "count(" << tag << ") ";
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (value) count--;
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (value) count++;
    }
    
    //! @brief The results of aggregation.
    size_t result() const {
        return count;
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
};


//! @brief Aggregates values by summing them.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class sum_aggregator {
    T sum = 0;
    
  public:
    //! @brief Constructs the aggregator object and outputs its description.
    sum_aggregator(std::ostream& os, std::string tag) {
        os << "sum(" << tag << ") ";
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value))
            sum -= value;
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            sum += value;
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        return sum;
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
};


//! @brief Aggregates values by averaging.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class mean_aggregator {
    T sum = 0;
    size_t count = 0;
    
  public:
    //! @brief Constructs the aggregator object and outputs its description.
    mean_aggregator(std::ostream& os, std::string tag) {
        os << "mean(" << tag << ") ";
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value)) {
            sum -= value;
            count--;
        }
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            sum += value;
            count++;
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        return sum/count;
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
};


//! @brief Aggregates values by n-th moment.
template <typename T, char n, bool only_finite = std::numeric_limits<T>::has_infinity>
class moment_aggregator {
    T sum = 0;
    size_t count = 0;
    
  public:
    //! @brief Constructs the aggregator object and outputs its description.
    moment_aggregator(std::ostream& os, std::string tag) {
        os << "dev(" << tag << ") ";
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value)) {
            sum -= pow(value, n);
            count--;
        }
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            sum += pow(value, n);
            count++;
        }
    }
    
    //! @brief The results of aggregation.
    T result() const {
        return pow(sum/count, 1.0/n);
    }

    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        os << result() << " ";
    }
};


//! @brief Aggregates values by mean and standard deviation.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
class dev_aggregator {
    T sum = 0, sqsum = 0;
    size_t count = 0;
    
  public:
    //! @brief Constructs the aggregator object and outputs its description.
    dev_aggregator(std::ostream& os, std::string tag) {
        os << "mean(" << tag << ") " << "dev(" << tag << ") ";
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value)) {
            sum -= value;
            sqsum -= value*value;
            count--;
        }
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value)) {
            sum += value;
            sqsum += value*value;
            count++;
        }
    }
    
    //! @brief The results of aggregation.
    std::tuple<T,T> result() const {
        T d2 = (sqsum*count-sum*sum)/count/count;
        T d1 = sqrt(d2);
        if (std::isfinite(d1) and (d1+1)*(d1+1) <= d2) ++d1;
        return {sum/count, d1};
    }

    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        std::tuple<T,T> res = result();
        os << std::get<0>(res) << " " << std::get<1>(res) << " ";
    }
};


//! @brief Aggregates values by maintaining their quantiles.
template <typename T, bool only_finite, char... qs>
class quantile_aggregator {
    const std::array<char, sizeof...(qs)> quantiles = {qs...};
    std::multiset<T> values;
    
  public:
    //! @brief Constructs the aggregator object and outputs its description.
    quantile_aggregator(std::ostream& os, std::string tag) {
        for (int q : quantiles) {
            if (q == 0) os << "min";
            else if (q == 100) os << "max";
            else os << "q" << (int)q;
        	os << "(" << tag << ") ";
        }
    }
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        if (not only_finite or std::isfinite(value))
            values.erase(values.find(value));
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        if (not only_finite or std::isfinite(value))
            values.insert(value);
    }
    
    //! @brief The results of aggregation.
    std::array<T,sizeof...(qs)> result() const {
        std::array<T,sizeof...(qs)> res;
        std::vector<T> ev(values.begin(), values.end());
        std::vector<size_t> iv;
        for (int q : quantiles) {
            int r = q*(ev.size()-1);
            iv.push_back(r/100);
            if (r % 100 > 0) iv.push_back(r/100 + 1);
        }
        nth_elements(ev.begin(), ev.end(), iv.begin(), iv.end());
        for (size_t i=0; i<quantiles.size(); ++i) {
            int q = quantiles[i];
            int r = q*(ev.size()-1);
            auto v = ev.begin() + (r/100);
            r %= 100;
            res[i] = r > 0 ? ((*v)*(100-r) + (*(v+1))*r)/100 : *v;
        }
        return res;
    }
    
    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        for (T x : result())
            os << x << " ";
    }
};


//! @brief Aggregates values by maintaining their minimum.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
using min_aggregator = quantile_aggregator<T,only_finite,0>;


//! @brief Aggregates values by maintaining their maximum.
template <typename T, bool only_finite = std::numeric_limits<T>::has_infinity>
using max_aggregator = quantile_aggregator<T,only_finite,100>;


//! @brief Chains multiple aggregators together into a single object.
template <typename T, typename... Ts>
class multi_aggregator : public Ts... {
  public:
    //! @brief Constructs the aggregator object and outputs its description.
    multi_aggregator(std::ostream& os, std::string tag) : Ts(os,tag)... {}
    
    //! @brief Erases a value from the aggregation set.
    void erase(T value) {
        details::ignore((Ts::erase(value),0)...);
    }
    
    //! @brief Inserts a new value to be aggregated.
    void insert(T value) {
        details::ignore((Ts::insert(value),0)...);
    }
    
    //! @brief The results of aggregation.
    auto result() const {
        return std::make_tuple(Ts::result()...);
    }

    //! @brief Printed results of aggregation.
    void output(std::ostream& os) const {
        details::ignore((Ts::output(os),0)...);
    }
};

}

#endif // FCPP_GENERATE_AGGREGATOR_H_
