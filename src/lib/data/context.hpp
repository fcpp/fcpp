// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file context.hpp
 * @brief Implementation of the `context<M, Ts...>` class template for grouping message data from different neighbours.
 */

#ifndef FCPP_DATA_CONTEXT_H_
#define FCPP_DATA_CONTEXT_H_

#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "lib/common/flat_ptr.hpp"
#include "lib/common/multitype_map.hpp"
#include "lib/common/traits.hpp"
#include "lib/data/field.hpp"
#include "lib/data/trace.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing specific objects for the FCPP library.
 */
namespace data {


/**
 * @brief Keeps associations between devices and export received.
 * Exports are added to neighbours' contexts at end of rounds,
 * possibly triggering filtering of old (or less relevant) exports.
 *
 * @param M Type of the export metrics.
 * @param Ts Types included in the exports.
 */
template <typename M, typename... Ts>
class context {
  public:
    //! @brief The type of the exports contained in the context.
    typedef common::flat_ptr<common::multitype_map<trace_t, Ts...>, FCPP_EXPORTS == 2> export_type;

    //! @brief The type of the metric on exports.
    typedef M metric_type;
    
  private:
    //! @brief Map associating devices to exports.
    std::unordered_map<device_t, export_type> m_data;
    //! @brief Map associating devices to metric results
    std::unordered_map<device_t, metric_type> m_metrics;
    //! @brief Exports ordered by metric results
    std::priority_queue<std::pair<metric_type, device_t>> m_queue;
    //! @brief The identifier of the local device.
    device_t m_self;
    
  public:
    //! @name constructors
    //@{
    //! @brief Default constructor creating an empty context.
    context(device_t self) : m_data(), m_metrics(), m_queue(), m_self(self) {};
    
    //! @brief Copy constructor.
    context(const context<M, Ts...>&) = default;
    
    //! @brief Move constructor.
    context(context<M, Ts...>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    context<M, Ts...>& operator=(const context<M, Ts...>&) = default;
    
    //! @brief Move assignment.
    context<M, Ts...>& operator=(context<M, Ts...>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const context<M, Ts...>& o) const {
        return m_self == o.m_self && m_data == o.m_data && m_metrics == o.m_metrics;
    }

    //! @brief Access to the local device identifier.
    device_t self() const {
        return m_self;
    }
    
    //! @brief Number of exports contained.
    size_t size() const {
        return m_data.size() + 1-m_data.count(m_self);
    }
    
    //! @brief Whether the queue of exports is empty.
    metric_type empty() const {
        return m_queue.empty();
    }
    
    //! @brief The highest metrics contained.
    metric_type top() const {
        return m_queue.top().first;
    }
    
    //! @brief Const access to the context data.
    const std::unordered_map<device_t, export_type>& data() const {
        return m_data;
    }
    
    //! @brief Const access to the context metrics.
    const std::unordered_map<device_t, metric_type>& metrics() const {
        return m_metrics;
    }

    //! @brief Inserts an export for a device.
    void insert(device_t device, export_type e, metric_type m) {
        insert(device, m);
        m_data[device] = std::move(e);
    }
    
    //! @brief Update the metric for an existing export of a device.
    void insert(device_t device, metric_type m) {
        m_queue.emplace(m, device);
        m_metrics[device] = m;
        clean();
    }
    
    //! @brief Erases the worst export.
    void pop() {
        device_t device = m_queue.top().second;
        m_data.erase(device);
        m_metrics.erase(device);
        m_queue.pop();
        clean();
    }
    
    //! @brief Returns list of devices with specified trace.
    std::unordered_set<device_t> align(trace_t trace) const {
        std::unordered_set<device_t> v;
        for (const auto& d : m_data)
            if (d.second->contains(trace))
                v.insert(d.first);
        v.insert(m_self);
        return v;
    }
    
    //! @brief Returns the old value for a certain trace (unaligned).
    template <typename A>
    const A& old(trace_t trace, const A& def) const {
        if (m_data.count(m_self) and m_data.at(m_self)->template count<A>(trace))
            return m_data.at(m_self)->template at<A>(trace);
        return def;
    }

    //! @brief Returns neighbours' values for a certain trace (default from `def`, and also self if not present).
    template <typename A>
    common::add_template<field, A> nbr(trace_t trace, const A& def) const {
        std::unordered_map<device_t, common::del_template<field, A>> m;
        for (const auto& x : m_data)
            if (x.second->template count<A>(trace))
                m[x.first] = details::self(x.second->template at<A>(trace), m_self);
        return details::make_field(other(def), m);
    }
    
  private:
    // Erases invalid elements at the top of m_queue.
    void clean() {
        while (m_metrics.count(m_queue.top().second) == 0 or m_metrics[m_queue.top().second] != m_queue.top().first)
            m_queue.pop();
    }
};


}


}

#endif // FCPP_DATA_CONTEXT_H_
