// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file context.hpp
 * @brief Implementation of the `context<M, Ts...>` class template for grouping message data from different neighbours.
 */

#ifndef FCPP_INTERNAL_CONTEXT_H_
#define FCPP_INTERNAL_CONTEXT_H_

#include <algorithm>
#include <ostream>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

#include "lib/common/multitype_map.hpp"
#include "lib/data/field.hpp"
#include "lib/internal/flat_ptr.hpp"
#include "lib/internal/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @cond INTERNAL
//! @brief Namespace containing objects of common use.
namespace common {

//! @brief Stream-like object for input or output serialization (depending on `io`).
template <bool io>
class sstream;
}
//! @endcond


//! @brief Namespace containing objects of internal use.
namespace internal {


/**
 * @brief Keeps associations between devices and export received.
 *
 * Exports are added to neighbours' contexts at end of rounds,
 * possibly triggering filtering of old (or less relevant) exports.
 *
 * @param online Whether the number of stored exports should be kept cleaned as exports are inserted.
 * @param pointer Whether the exports should be stored in pointers or not.
 * @param M Type of the export metrics.
 * @param Ts Types included in the exports.
 */
template <bool online, bool pointer, typename M, typename... Ts>
class context;


/**
 * @brief Keeps associations between devices and export received.
 *
 * Specialisation for online cleaning of export as they are inserted.
 */
template <bool pointer, typename M, typename... Ts>
class context<true, pointer, M, Ts...> {
  public:
    //! @brief The type of the exports contained in the context.
    typedef internal::flat_ptr<common::multitype_map<trace_t, Ts...>, not pointer> export_type;

    //! @brief The type of the metric on exports.
    typedef M metric_type;

    //! @brief Default constructor creating an empty context.
    context() = default;

    //! @brief Copy constructor.
    context(context const&) = default;

    //! @brief Move constructor.
    context(context&&) = default;

    //! @brief Copy assignment.
    context& operator=(context const&) = default;

    //! @brief Move assignment.
    context& operator=(context&&) = default;

    //! @brief Equality operator.
    bool operator==(context const& o) const {
        return m_data == o.m_data && m_metrics == o.m_metrics;
    }

    //! @brief Number of exports contained.
    size_t size(device_t self) const {
        return m_data.size() + 1-m_data.count(self);
    }

    //! @brief Inserts an export for a device with a certain metric, possibly cleaning up.
    void insert(device_t d, export_type e, metric_type m, metric_type threshold, device_t hoodsize) {
        assert(m_sorted_data.size() == 0);
        if (m <= threshold) {
            if (m_metrics.count(d) == 0 or m_metrics[d] != m)
                m_queue.emplace(m, d);
            m_metrics[d] = m;
            m_data[d] = std::move(e);
            if (m_data.size() > hoodsize) pop();
            else clean();
        }
    }

    //! @brief The worst export currently in context.
    device_t top() {
        assert(m_sorted_data.size() == 0);
        clean();
        return m_queue.top().second;
    }

    //! @brief Erases the worst export.
    void pop() {
        assert(m_sorted_data.size() == 0);
        clean();
        m_data.erase(m_queue.top().second);
        m_metrics.erase(m_queue.top().second);
        m_queue.pop();
    }

    //! @brief Changes the status of the context from "modify" to "query".
    void freeze(device_t, device_t) {
        assert(m_sorted_data.size() == 0);
        for (auto const& x : m_data)
            m_sorted_data.emplace_back(x.first, &x.second);
        std::sort(m_sorted_data.begin(), m_sorted_data.end());
        assert(m_sorted_data.size() == m_data.size());
    }

    //! @brief Changes the status of the context from "query" to "modify", updating metrics.
    template <typename N, typename T>
    void unfreeze(N const& node, T const& metric, metric_type threshold) {
        assert(m_sorted_data.size() == m_data.size());
        m_sorted_data.clear();
        m_queue = {};
        for (auto it = m_metrics.begin(); it != m_metrics.end(); ) {
            it->second = metric.update(it->second, node);
            if (it->second > threshold) {
                m_data.erase(it->first);
                it = m_metrics.erase(it);
            } else {
                m_queue.emplace(it->second, it->first);
                ++it;
            }
        }
        assert(m_sorted_data.size() == 0);
    }

    //! @brief Returns list of all devices.
    std::vector<device_t> align(device_t self) const {
        assert(m_sorted_data.size() == m_data.size());
        std::vector<device_t> v;
        auto it = m_sorted_data.begin();
        for (; it != m_sorted_data.end() and it->first < self; ++it)
            v.push_back(it->first);
        v.push_back(self);
        if (it != m_sorted_data.end() and it->first == self) ++it;
        for (; it != m_sorted_data.end(); ++it)
            v.push_back(it->first);
        return v;
    }

    //! @brief Returns list of devices with specified trace.
    std::vector<device_t> align(trace_t trace, device_t self) const {
        assert(m_sorted_data.size() == m_data.size());
        std::vector<device_t> v;
        auto it = m_sorted_data.begin();
        for (; it != m_sorted_data.end() and it->first < self; ++it)
            if ((*(it->second))->contains(trace)) {
                v.push_back(it->first);
            }
        v.push_back(self);
        if (it != m_sorted_data.end() and it->first == self) ++it;
        for (; it != m_sorted_data.end(); ++it)
            if ((*(it->second))->contains(trace)) {
                v.push_back(it->first);
            }
        return v;
    }

    //! @brief Returns the old value for a certain trace (unaligned).
    template <typename A>
    A const& old(trace_t trace, A const& def, device_t self) const {
        assert(m_sorted_data.size() == m_data.size() or m_data.size() == 1);
        if (m_data.count(self) and m_data.at(self)->template count<A>(trace))
            return m_data.at(self)->template at<A>(trace);
        return def;
    }

    //! @brief Returns neighbours' values for a certain trace (default from `def`, and also self if not present).
    template <typename A>
    to_field<A> nbr(trace_t trace, A const& def, device_t self) const {
        assert(m_sorted_data.size() == m_data.size());
        std::vector<device_t> ids;
        std::vector<to_local<A>> vals;
        vals.push_back(fcpp::details::other(def));
        for (auto const& x : m_sorted_data)
            if ((*x.second)->template count<A>(trace)) {
                ids.push_back(x.first);
                vals.push_back(fcpp::details::self(static_cast<A const&>((*x.second)->template at<A>(trace)), self));
            }
        return fcpp::details::make_field(std::move(ids), std::move(vals));
    }

    //! @brief Prints the context in a stream.
    template <typename O>
    void print(O& o) const {
        bool first = true;
        for (auto const& x : m_metrics) {
            if (first) first = false;
            else o << ", ";
            o << x.first << ":" << m_data.at(x.first) << "@" << 0+x.second;
        }
    }

    //! @brief Serialises the content from/to a given input/output stream.
    common::sstream<false>& serialize(common::sstream<false>& s) {
        s >> m_data >> m_metrics;
        m_sorted_data.clear();
        for (auto const& x : m_metrics)
            m_queue.emplace(x.second, x.first);
        return s;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    common::sstream<true>& serialize(common::sstream<true>& s) const {
        return s << m_data << m_metrics;
    }

  private:
    //! @brief Erases invalid values from the queue.
    void clean() {
        while (m_metrics.count(m_queue.top().second) == 0 or m_metrics.at(m_queue.top().second) != m_queue.top().first)
            m_queue.pop();
    }

    //! @brief Map associating devices to exports.
    std::unordered_map<device_t, export_type> m_data;
    //! @brief Map associating devices to metric results.
    std::unordered_map<device_t, metric_type> m_metrics;
    //! @brief Exports ordered by metric results.
    std::priority_queue<std::pair<metric_type, device_t>> m_queue;
    //! @brief Exports ordered by device.
    std::vector<std::pair<device_t, export_type const*>> m_sorted_data;
};


/**
 * @brief Keeps associations between devices and export received.
 *
 * Specialisation for cleaning of exports only at round start.
 */
template <bool pointer, typename M, typename... Ts>
class context<false, pointer, M, Ts...> {
  public:
    //! @brief The type of the exports contained in the context.
    typedef internal::flat_ptr<common::multitype_map<trace_t, Ts...>, not pointer> export_type;

    //! @brief The type of the metric on exports.
    typedef M metric_type;

    //! @brief Default constructor creating an empty context.
    context() = default;

    //! @brief Copy constructor.
    context(context const&) = default;

    //! @brief Move constructor.
    context(context&&) = default;

    //! @brief Copy assignment.
    context& operator=(context const&) = default;

    //! @brief Move assignment.
    context& operator=(context&&) = default;

    //! @brief Equality operator.
    bool operator==(context const& o) const {
        return m_data == o.m_data;
    }

    //! @brief Number of exports contained.
    size_t size(device_t self) const {
        return m_data.size() + (m_self == m_data.size() or get<0>(m_data[m_self]) != self);
    }

    //! @brief Inserts an export for a device with a certain metric, possibly cleaning up.
    void insert(device_t d, export_type e, metric_type m, metric_type threshold, device_t) {
        if (m <= threshold) {
            if (m_data.size() > 0 and get<0>(m_data.back()) == d)
                m_data.back() = data_type{d, m, std::move(e)};
            else m_data.emplace_back(d, m, std::move(e));
        }
    }

    //! @brief Changes the status of the context from "modify" to "query".
    void freeze(device_t hoodsize, device_t self) {
        std::stable_sort(m_data.begin(), m_data.end(), [](data_type const& x, data_type const& y){
            return get<0>(x) < get<0>(y);
        });
        size_t w = 0;
        for (size_t r = 0; r+1 < m_data.size(); ++r) {
            if (get<0>(m_data[r]) < get<0>(m_data[r+1])) {
                if (r > w) m_data[w] = std::move(m_data[r]);
                ++w;
            }
        }
        if (m_data.size() > w+1) m_data[w] = std::move(m_data.back());
        m_data.resize(w+1);
        if (m_data.size() > hoodsize) {
            std::vector<size_t> v(m_data.size());
            for (size_t i=0; i<m_data.size(); ++i) v[i] = i;
            std::nth_element(v.begin(), v.begin()+hoodsize, v.end(), [this](size_t i, size_t j){
                if (get<1>(m_data[i]) < get<1>(m_data[j])) return true;
                if (get<1>(m_data[i]) > get<1>(m_data[j])) return false;
                return get<0>(m_data[i]) < get<0>(m_data[j]);
            });
            size_t i = v[hoodsize];
            m_data.resize(std::remove_if(m_data.begin(), m_data.end(), [this, i](data_type const& x){
                if (get<1>(x) > get<1>(m_data[i])) return true;
                if (get<1>(x) < get<1>(m_data[i])) return false;
                return get<0>(x) >= get<0>(m_data[i]);
            }) - m_data.begin());
        }
        m_self = std::lower_bound(m_data.begin(), m_data.end(), data_type{self, metric_type{}, export_type{}}, [](data_type const& x, data_type const& y) {
            return get<0>(x) < get<0>(y);
        }) - m_data.begin();
    }

    //! @brief Changes the status of the context from "query" to "modify", updating metrics.
    template <typename N, typename T>
    void unfreeze(N const& node, T const& metric, metric_type threshold) {
        size_t w = 0;
        for (size_t r = 0; r < m_data.size(); ++r) {
            get<1>(m_data[r]) = metric.update(get<1>(m_data[r]), node);
            if (get<1>(m_data[r]) < threshold) {
                if (r > w) m_data[w] = std::move(m_data[r]);
                ++w;
            }
        }
        m_data.resize(w);
    }

    //! @brief Returns list of all devices.
    std::vector<device_t> align(device_t self) const {
        std::vector<device_t> v;
        size_t i = 0;
        for (; i < m_self; ++i)
            v.push_back(get<0>(m_data[i]));
        v.push_back(self);
        if (i < m_data.size() and get<0>(m_data[i]) == self) ++i;
        for (; i < m_data.size(); ++i)
            v.push_back(get<0>(m_data[i]));
        return v;
    }

    //! @brief Returns list of devices with specified trace.
    std::vector<device_t> align(trace_t trace, device_t self) const {
        std::vector<device_t> v;
        size_t i = 0;
        for (; i < m_self; ++i)
            if (get<2>(m_data[i])->contains(trace))
                v.push_back(get<0>(m_data[i]));
        v.push_back(self);
        if (i < m_data.size() and get<0>(m_data[i]) == self) ++i;
        for (; i < m_data.size(); ++i)
            if (get<2>(m_data[i])->contains(trace))
                v.push_back(get<0>(m_data[i]));
        return v;
    }

    //! @brief Returns the old value for a certain trace (unaligned).
    template <typename A>
    A const& old(trace_t trace, A const& def, device_t self) const {
        if (m_self < m_data.size() and get<0>(m_data[m_self]) == self and get<2>(m_data[m_self])->template count<A>(trace))
            return get<2>(m_data[m_self])->template at<A>(trace);
        return def;
    }

    //! @brief Returns neighbours' values for a certain trace (default from `def`, and also self if not present).
    template <typename A>
    to_field<A> nbr(trace_t trace, A const& def, device_t self) const {
        std::vector<device_t> ids;
        std::vector<to_local<A>> vals;
        vals.push_back(fcpp::details::other(def));
        for (auto const& x : m_data)
            if (get<2>(x)->template count<A>(trace)) {
                ids.push_back(get<0>(x));
                vals.push_back(fcpp::details::self(static_cast<A const&>(get<2>(x)->template at<A>(trace)), self));
            }
        return fcpp::details::make_field(std::move(ids), std::move(vals));
    }

    //! @brief Prints the context in a stream.
    template <typename O>
    void print(O& o) const {
        bool first = true;
        for (auto const& x : m_data) {
            if (first) first = false;
            else o << ", ";
            o << get<0>(x) << ":" << get<2>(x) << "@" << 0+get<1>(x);
        }
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_data & m_self;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_data << m_self;
    }

  private:
    //! @brief The type of elements stored.
    using data_type = std::tuple<device_t, metric_type, export_type>;

    //! @brief Sequence of exports stored.
    std::vector<data_type> m_data;

    //! @brief Index of self in @ref m_data.
    size_t m_self;
};


//! @cond INTERNAL
namespace details {
    // General form.
    template <bool online, bool pointer, typename M, typename T>
    struct context_t;

    // Unpacking form.
    template <bool online, bool pointer, typename M, typename... Ts>
    struct context_t<online, pointer, M, common::type_sequence<Ts...>> {
        using type = context<online, pointer, M, Ts...>;
    };
}
//! @endcond

//! @brief Context built with a type sequence of types.
template <bool online, bool pointer, typename M, typename T>
using context_t = typename details::context_t<online,pointer,M,T>::type;


}


}

#endif // FCPP_INTERNAL_CONTEXT_H_
