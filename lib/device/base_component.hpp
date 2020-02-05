// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file base_component.hpp
 * @brief Implementation of the `extend_component` template for component chaining.
 */

#ifndef FCPP_DEVICE_BASE_COMPONENT_H_
#define FCPP_DEVICE_BASE_COMPONENT_H_

#include <cassert>
#include <algorithm>
#include <array>
#include <limits>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @cond INTERNAL
namespace details {
    /*
     * Empty component (base case for component construction).
     * The type parameter ensures unicity, avoiding the diamond problem.
     * Should only be referred through `extend_component`.
     *
     * C Parent component deriving from base_component.
     */
    template <typename C>
    struct base_component {
        //! @brief No parent component in the chain.
        using parent_t = void;
        
        //! @brief A `tagged_tuple` type used for messages to be exchanged with neighbours.
        using message_t = fcpp::tagged_tuple<>;
        
        //! @brief A manager class to be shared among multiple devices.
        struct manager {
            //! @brief Default constructor.
            template <typename G>
            manager(G&) {}
            
            //! @brief Returns next event to schedule.
            template <typename G>
            times_t next(G&) {
                return std::numeric_limits<times_t>::max(); // no event to schedule
            }
            
            //! @brief Updates the internal status of the manager and returns next event.
            template <typename G>
            times_t update(G&) {
                return std::numeric_limits<times_t>::max(); // no event to schedule
            }
        };
        
        //! @name constructors
        //@{
        //! @brief Default constructor.
        base_component() = default;
        
        //! @brief Constructor from a tagged tuple.
        template <typename... Ts>
        base_component(const fcpp::tagged_tuple<Ts...>&) {}
        
        //! @brief Copy constructor.
        base_component(const base_component<C>&) = default;
        
        //! @brief Move constructor.
        base_component(base_component<C>&&) = default;
        //@}
        
        //! @name assignment operators
        //@{
        //! @brief Copy assignment.
        base_component<C>& operator=(const base_component<C>&) = default;
        
        //! @brief Move assignment.
        base_component<C>& operator=(base_component<C>&&) = default;
        //@}
        
        //! @brief Equality operator.
        bool operator==(const base_component<C>&) const {
            return true;
        }
        
        //! @brief Reads values for sensor on incoming messages (from a certain source).
        template <typename... Ts>
        void insert(const manager&, const tagged_tuple<Ts...>&) {}
        
        //! @brief Reads values from plain sensors.
        void round_start(const manager&) {}
        
        //! @brief Performs actuation, returning data to attach to messages.
        message_t round_end(manager&) const {
            return {};
        }
    };
}
//! @endcond

    
/**
 * @brief Handles component chaining.
 *
 * Every component `D` should have another component `C` as last template
 * argument, and implement chaining by deriving from this.
 *
 * @param C Component to derive from.
 * @param D Component deriving from C.
 */
template <typename C, typename D>
using extend_component = std::conditional_t<std::is_same<C,void>::value, details::base_component<D>, C>;


}

#endif // FCPP_DEVICE_BASE_COMPONENT_H_
