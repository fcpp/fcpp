// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file data_component.hpp
 * @brief Implementation of the `storage_component` and `exporter_component` class templates for handling persistent data and summarisation to file.
 */

#ifndef FCPP_DEVICE_DATA_COMPONENT_H_
#define FCPP_DEVICE_DATA_COMPONENT_H_

#include "lib/settings.hpp"
#include "lib/common/tagged_tuple.hpp"
#include "lib/device/multi_component.hpp"


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


}

#endif // FCPP_DEVICE_DATA_COMPONENT_H_
