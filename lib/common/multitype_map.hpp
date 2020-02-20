// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

/**
 * @file multitype_map.hpp
 * @brief Implementation of the `multitype_map<T, Ts...>` class template for handling heterogeneous indexed data.
 */

#ifndef FCPP_COMMON_MULTITYPE_MAP_H_
#define FCPP_COMMON_MULTITYPE_MAP_H_

#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "lib/common/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


/**
 * @brief Class for handling heterogeneous indexed data.
 *
 * @param T Key type.
 * @param Ts Admissible value types (must avoid repetitions).
 */
template <typename T, typename... Ts>
class multitype_map {
    static_assert(type_repeated<Ts...>::size == 0, "cannot instantiate multitype_map with repeated types");
    
  public:
    //! @brief The type of the keys.
    typedef T key_type;
    
  private:
    //! @brief Map associating keys to data.
    std::tuple<std::unordered_map<T, Ts>...> m_data;
    //! @brief Set of keys (for void data).
    std::unordered_set<T> m_keys;
    
  public:
    //! @name constructors
    //@{
    /**
     * @brief Default constructor (creates an empty structure).
     */
    multitype_map() = default;
    
    //! @brief Copy constructor.
    multitype_map(const multitype_map<T, Ts...>&) = default;
    
    //! @brief Move constructor.
    multitype_map(multitype_map<T, Ts...>&&) = default;
    //@}

    //! @name assignment operators
    //@{
    //! @brief Copy assignment.
    multitype_map<T, Ts...>& operator=(const multitype_map<T, Ts...>&) = default;
    
    //! @brief Move assignment.
    multitype_map<T, Ts...>& operator=(multitype_map<T, Ts...>&&) = default;
    //@}

    //! @brief Equality operator.
    bool operator==(const multitype_map<T, Ts...>& o) const {
        return m_keys == o.m_keys && m_data == o.m_data;
    }
    
    //! @brief Inserts value at corresponding key.
    template<typename A>
    void insert(T key, const A& value) {
        static_assert(type_count<typename std::remove_reference<A>::type, Ts...> != 0, "non-supported type access");
        std::get<type_find<typename std::remove_reference<A>::type, Ts...>>(m_data)[key] = value;
    }
    
    //! @brief Inserts value at corresponding key by moving.
    template<typename A>
    void insert(T key, A&& value) {
        static_assert(type_count<typename std::remove_reference<A>::type, Ts...> != 0, "non-supported type access");
        std::get<type_find<typename std::remove_reference<A>::type, Ts...>>(m_data)[key] = value;
    }

    //! @brief Inserts void value at corresponding key.
    void insert(T key) {
        m_keys.insert(key);
    }
    
    //! @brief Deletes value at corresponding key.
    template<typename A>
    void erase(T key) {
        static_assert(type_count<typename std::remove_reference<A>::type, Ts...> != 0, "non-supported type access");
        std::get<type_find<typename std::remove_reference<A>::type, Ts...>>(m_data).erase(key);
    }

    //! @brief Deletes void value at corresponding key.
    void remove(T key) {
        m_keys.erase(key);
    }

    //! @brief Immutable reference to the value of a certain type at a given key.
    template<typename A>
    const A& at(T key) const {
        static_assert(type_count<typename std::remove_reference<A>::type, Ts...> != 0, "non-supported type access");
        return std::get<type_find<typename std::remove_reference<A>::type, Ts...>>(m_data).at(key);
    }

    //! @brief Mutable reference to the value of a certain type at a given key.
    template<typename A>
    A& at(T key) {
        static_assert(type_count<typename std::remove_reference<A>::type, Ts...> != 0, "non-supported type access");
        return std::get<type_find<typename std::remove_reference<A>::type, Ts...>>(m_data).at(key);
    }

    //! @brief Whether the key is present in the value map or not for a certain type.
    template<typename A>
    bool count(T key) const {
        static_assert(type_count<typename std::remove_reference<A>::type, Ts...> != 0, "non-supported type access");
        return std::get<type_find<typename std::remove_reference<A>::type, Ts...>>(m_data).count(key);
    }
    
    //! @brief Whether the key is present in the value map or not for the void type.
    bool contains(T key) const {
        return m_keys.count(key);
    }
};


}


}

#endif // FCPP_COMMON_MULTITYPE_MAP_H_
