// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file component.hpp
 * @brief Implementation of the `component<M, Ts...>` class template for grouping message data from different neighbours.
 */

#ifndef FCPP_DEVICE_COMPONENT_H_
#define FCPP_DEVICE_COMPONENT_H_

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
            manager() = default;
            
            //! @brief Returns next event to schedule.
            times_t next() {
                return std::numeric_limits<times_t>::max(); // no event to schedule
            }
            
            //! @brief Updates the internal status of the manager and returns next event.
            times_t update() {
                return next();
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

    
/**
 * @brief Models persistent data.
 *
 * @param T A `tagged_tuple` used to store the data.
 * @param C Component to derive from.
 */
template <typename T, typename C = void>
class storage_component : public extend_component<C, storage_component<T,C>> {
  public:
    //! @brief The parent component in the chain.
    using parent_t = extend_component<C, storage_component<T,C>>;
    
  private:
    //! @brief The tagget tuple data.
    T m_data;

  public:
    //! @name constructors
    //@{
    //! @brief Default constructor.
    storage_component() = default;

    //! @brief Constructor from a tagged tuple.
    template <typename S, typename U>
    storage_component(const details::tagged_tuple<S, U>& t) : parent_t(t), m_data(t) {}

    //! @brief Copy constructor.
    storage_component(const storage_component<T,C>&) = default;

    //! @brief Move constructor.
    storage_component(storage_component<T,C>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    storage_component<T,C>& operator=(const storage_component<T,C>&) = default;

    //! @brief Move assignment.
    storage_component<T,C>& operator=(storage_component<T,C>&&) = default;
    //@}
    
    //! @brief Equality operator.
    bool operator==(const storage_component<T,C>& o) const {
        return m_data == o.m_data and static_cast<const parent_t&>(*this).operator==(o);
    }
    
  protected:
    //! @brief Accesses the stored data.
    template <typename S>
    typename T::template tag_type<S>& storage() {
        return fcpp::get<S>(m_data);
    }
};


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
    
    template <typename M>
    times_t call_update(size_t, M&, type_sequence<>) {
        assert(false); // this should not happen
    }
    
    // calls update on the i-th supertype of M
    template <typename M, typename N, typename... Ms>
    times_t call_update(size_t i, M& m, type_sequence<N, Ms...>) {
        if (i == 0) return static_cast<N&>(m).update();
        return call_update(i-1, m, type_sequence<Ms...>());
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
        manager() : Cs::manager()... {}
        
        //! @brief Returns next event to schedule.
        times_t next() {
            return static_cast<type_get<sizeof...(Cs)-1, typename Cs::manager&...>>(*this).next();
        }
        
        //! @brief Updates the internal status of the manager and returns next event.
        times_t update() {
            return static_cast<type_get<sizeof...(Cs)-1, typename Cs::manager&...>>(*this).update();
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
        manager() : unscheduling_component<Cs...>::manager(), pending({Cs::manager::next()...}) {}
        
        //! @brief Returns next event to schedule.
        times_t next() {
            return *std::min_element(pending.begin(), pending.end());
        }
        
        //! @brief Updates the internal status of the manager and returns next event.
        times_t update() {
            size_t i = std::min_element(pending.begin(), pending.end()) - pending.begin();
            pending[i] = details::call_update(i, *this, type_sequence<typename Cs::manager...>());
            return next();
        }
    };
    
    //! @name constructors
    using unscheduling_component<Cs...>::unscheduling_component;
};


}

#endif // FCPP_DEVICE_COMPONENT_H_
