// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

/**
 * @file context.hpp
 * @brief Implementation of the `context<Ts...>` class template for grouping message data from different neighbours.
 */

#ifndef FCPP_DATA_CONTEXT_H_
#define FCPP_DATA_CONTEXT_H_

#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

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
 * @brief Keeps associations between devices and export received.
 * Exports are added to neighbours' contexts at end of rounds,
 * possibly triggering filtering of old (or less relevant) exports.
 *
 * @param Ts Types included in the exports.
 */
template <typename... Ts>
class context {
  public:
    //! @brief The type of the exports contained in the context.
    typedef flat_ptr<multitype_map<trace_t, Ts...>, FCPP_SETTING_EXPORTS == 2> exports;

  private:
    //! @brief Map associating devices to exports.
    std::unordered_map<device_t, exports> data;
    //! @brief The identifier of the local device.
    device_t self;
    
  public:
    //! @name constructors
    //@{
    //! @brief Default constructor creating an empty context.
    context(device_t device) : data(), self(device) {};
    
    //! @brief Copy constructor.
    context(const context<Ts...>&) = default;
    
    //! @brief Move constructor.
    context(context<Ts...>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    context<Ts...>& operator=(const context<Ts...>&) = default;
    
    //! @brief Move assignment.
    context<Ts...>& operator=(context<Ts...>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const context<Ts...>& o) const {
        return self == o.self && data == o.data;
    }
    
    //! @brief Inserts an export for a device.
    void insert(device_t device, const exports& e) {
        data[device] = e;
    }
    
    //! @brief Inserts an export for a device by moving.
    void insert(device_t device, exports&& e) {
        data[device] = e;
    }
    
    //! @brief Erase exports for a device.
    void erase(device_t device) {
        data.erase(device);
    }
    
    //! @brief Returns list of devices with specified trace.
    std::vector<device_t> align(trace_t trace) const {
        std::vector<device_t> v;
        for (const auto& d : data)
            if (d.second->contains(trace))
                v.push_back(d.first);
        return v;
    }
    
    //! @brief Returns the old value for a certain trace (unaligned).
    template <typename A>
    const A& old(trace_t trace, const A& def) const {
        if (data.count(self) and data.at(self)->template count<A>(trace))
            return data.at(self)->template at<A>(trace);
        return def;
    }

    //! @brief Returns neighbours' values for a certain trace (default from initial, and also self if not present).
    template <typename A>
    typename add_template<field, A>::type nbr(trace_t trace, typename del_template<field, A>::type def) const {
        std::unordered_map<device_t, typename del_template<field, A>::type> m;
        for (const auto& x : data)
            if (x.second->template count<A>(trace))
                m[x.first] = details::self(x.second->template at<A>(trace), self);
        return details::make_field(def, m);
    }
};


}

#endif // FCPP_DATA_CONTEXT_H_
