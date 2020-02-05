// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file data_component.hpp
 * @brief Implementation of the `storage_component` and `exporter_component` class templates for handling persistent data and summarisation to file.
 */

#ifndef FCPP_DEVICE_DATA_COMPONENT_H_
#define FCPP_DEVICE_DATA_COMPONENT_H_

#include <iostream>
#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/device/base_component.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


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


/**
 * @brief Handles summarisation to file of data stored in a `storage_component`.
 *
 * @param T The tag (in `storage_component`) of data to be summarised.
 * @param E An event generator, handling refreshes of the aggregators.
 * @param A A data aggregator.
 * @param C Component to derive from.
 */
template <typename T, typename E, typename A, typename C = void>
class exporter_component : public extend_component<C, exporter_component<T,E,A,C>> {
  public:
    //! @brief The parent component in the chain.
    using parent_t = extend_component<C, exporter_component<T,E,A,C>>;

    //! @brief A manager class to be shared among multiple devices.
    class manager : public parent_t::manager {
      private:
        //! @brief The event generator scheduling updates.
        E scheduler;
        
        //! @brief The aggregator gathering data.
        A aggregator;
        
      public:
        //! @brief Default constructor.
        template <typename G>
        manager(G& g) : E(g), A(typeid(T).name()) {}
        
        //! @brief Returns next event to schedule.
        template <typename G>
        times_t next(G& g) {
            return scheduler.next(g);
        }
        
        //! @brief Updates the internal status of the manager and returns next event.
        template <typename G>
        times_t update(G& g) {
            aggregator.output();
            scheduler(g);
            return scheduler.next(g);
        }
    };
    
  public:
    //! @name constructors
    //@{
    //! @brief Default constructor.
    exporter_component() {
        //m.aggregator.insert(parent_t::template storage<T>());
    }
    
    ~exporter_component() {
        //m.aggregator.erase(parent_t::template storage<T>());
    }

    //! @brief Constructor from a tagged tuple.
    template <typename S, typename U>
    exporter_component(const details::tagged_tuple<S, U>& t) : parent_t(t) {}

    //! @brief Copy constructor.
    exporter_component(const exporter_component<T,E,A,C>&) = default;

    //! @brief Move constructor.
    exporter_component(exporter_component<T,E,A,C>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    exporter_component<T,E,A,C>& operator=(const exporter_component<T,E,A,C>&) = default;

    //! @brief Move assignment.
    exporter_component<T,E,A,C>& operator=(exporter_component<T,E,A,C>&&) = default;
    //@}

    //! @brief Reads values from plain sensors.
    void round_start(const manager& m) {
        m.aggregator.erase(parent_t::template storage<T>());
        parent_t::round_start();
    }

    //! @brief Performs actuation, returning data to attach to messages.
    typename parent_t::message_t round_end(device_t self, manager& m) const {
        m.aggregator.insert(parent_t::template storage<T>());
        return parent_t::round_end(m);
    }
};


}

#endif // FCPP_DEVICE_DATA_COMPONENT_H_
