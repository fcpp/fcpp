// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file multi_component.hpp
 * @brief Implementation of the `extend_component` template for component chaining, and of the `unscheduling_component` and `multi_component` classes for grouping components together.
 */

#ifndef FCPP_DEVICE_MULTI_COMPONENT_H_
#define FCPP_DEVICE_MULTI_COMPONENT_H_

#include <cassert>
#include <algorithm>
#include <array>
#include <limits>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"
#include "lib/device/base_component.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @cond INTERNAL
namespace details {
    // checks if all arguments are true
    bool all() {
        return true;
    }
    template <typename... Ts>
    bool all(bool v, Ts... vs) {
        return v and all(vs...);
    }
    
    template <typename M, typename G>
    times_t call_update(size_t, M&, G&, type_sequence<>) {
        assert(false); // this should not happen
    }
    
    // calls update on the i-th supertype of M
    template <typename M, typename G, typename N, typename... Ms>
    times_t call_update(size_t i, M& m, G& g, type_sequence<N, Ms...>) {
        if (i == 0) return static_cast<N&>(m).update(g);
        return call_update(i-1, m, g, type_sequence<Ms...>());
    }
}
//! @endcond


/**
 * @brief Gathers multiple components into one, without scheduling refreshes.
 * Individual components includes sensors, actuators, oracles, aggregators,
 * schedulers and connectors. Components can be chained with each other, or
 * combined all together through this template.
 * Only the last component is called for updates.
 *
 * @param Cs Sub-components to be packed together.
 */
template <typename... Cs>
struct unscheduling_component : public Cs... {
    static_assert(sizeof...(Cs) >= 2, "too few arguments");
    static_assert(type_repeated<Cs...>::size == 0, "repeating components disallowed");

    //! @brief No parent component in the chain.
    using parent_t = void;
    
    //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
    using message_t = tagged_tuple_cat<typename Cs::message_t...>;
    
    //! @brief A manager class to be shared among multiple devices.
    struct manager : public Cs::manager... {
        //! @brief Default constructor.
        template <typename G>
        manager(G& g) : Cs::manager(g)... {}
        
        //! @brief Returns next event to schedule.
        template <typename G>
        times_t next(G& g) {
            return static_cast<type_get<sizeof...(Cs)-1, typename Cs::manager&...>>(*this).next(g);
        }
        
        //! @brief Updates the internal status of the manager and returns next event.
        template <typename G>
        times_t update(G& g) {
            return static_cast<type_get<sizeof...(Cs)-1, typename Cs::manager&...>>(*this).update(g);
        }
    };
    
    //! @name constructors
    //@{
    //! @brief Default constructor.
    unscheduling_component() = default;
    
    //! @brief Constructor from a tagged tuple.
    template <typename... Ts>
    unscheduling_component(const tagged_tuple<Ts...>& t) : Cs(t)... {}
    
    //! @brief Copy constructor.
    unscheduling_component(const unscheduling_component<Cs...>&) = default;
    
    //! @brief Move constructor.
    unscheduling_component(unscheduling_component<Cs...>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    unscheduling_component<Cs...>& operator=(const unscheduling_component<Cs...>&) = default;
    
    //! @brief Move assignment.
    unscheduling_component<Cs...>& operator=(unscheduling_component<Cs...>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const unscheduling_component<Cs...>& o) const {
        return details::all((Cs::operator==(o))...);
    }
    
    //! @brief Reads values for sensor on incoming messages (from a certain source).
    template <typename... Ts>
    void insert(const manager& m, const tagged_tuple<Ts...>& source) {
        details::ignore((Cs::insert(m, source),0)...);
    }
    
    //! @brief Reads values from plain sensors.
    void round_start(const manager& m) {
        details::ignore((Cs::round_start(m),0)...);
    }

    //! @brief Performs actuation, returning data to attach to messages.
    message_t round_end(manager& m) const {
        message_t message;
        details::ignore((message = Cs::round_end(m))...);
        return message_t();
    }
};


/**
 * @brief Gathers multiple components into one.
 * Individual components includes sensors, actuators, oracles, aggregators,
 * schedulers and connectors. Components can be chained with each other, or
 * combined all together through this template.
 *
 * @param Cs Sub-components to be packed together.
 */
template <typename... Cs>
struct multi_component : public unscheduling_component<Cs...> {
    //! @brief A manager class to be shared among multiple devices.
    class manager : public unscheduling_component<Cs...>::manager {
      private:
        std::array<times_t, sizeof...(Cs)> pending;
        
      public:
        //! @brief Default constructor.
        template <typename G>
        manager(G& g) : unscheduling_component<Cs...>::manager(g), pending({Cs::manager::next(g)...}) {}
        
        //! @brief Returns next event to schedule.
        template <typename G>
        times_t next(G& g) {
            return *std::min_element(pending.begin(), pending.end());
        }
        
        //! @brief Updates the internal status of the manager and returns next event.
        template <typename G>
        times_t update(G& g) {
            size_t i = std::min_element(pending.begin(), pending.end()) - pending.begin();
            pending[i] = details::call_update(i, *this, g, type_sequence<typename Cs::manager...>());
            return next(g);
        }
    };
    
    //! @name constructors
    using unscheduling_component<Cs...>::unscheduling_component;
};


}

#endif // FCPP_DEVICE_MULTI_COMPONENT_H_
