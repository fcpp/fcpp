// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

/**
 * @file multitype_map.hpp
 * @brief Implementation of the `multitype_map<T, Ts...>` class template for handling heterogeneous indexed data.
 */

#ifndef FCPP_DATA_MULTITYPE_MAP_H_
#define FCPP_DATA_MULTITYPE_MAP_H_

#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "lib/util/traits.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Class for handling heterogeneous indexed data.
 *
 * @param T Key type.
 * @param Ts Admissible value types (must avoid repetitions).
 */
template <typename T, typename... Ts>
class multitype_map {
    static_assert(not type_repeated<Ts...>, "cannot instantiate multitype_map with repeated types");
    
    //! @brief Map associating keys to data.
    std::tuple<std::unordered_map<T, Ts>...> data;
    //! @brief Set of keys (for void data).
    std::unordered_set<T> keys;
    
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
        return keys == o.keys && data == o.data;
    }
    
    //! @brief Inserts value at corresponding key.
    template<typename A>
    void insert(T key, const A& value) {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data)[key] = value;
    }

    //! @brief Whether the key is present in the value map or not for a certain type.
    template<typename A>
    bool count(T key) const {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        return std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data).count(key);
    }

    //! @brief Immutable reference to the value of a certain type at a given key.
    template<typename A>
    const A& at(T key) const {
        static_assert(type_contains<typename std::remove_reference<A>::type, Ts...>, "non-supported type access");
        return std::get<type_index<typename std::remove_reference<A>::type, Ts...>>(data).at(key);
    }
    
    //! @brief Inserts void value at corresponding key.
    void insert(T key) {
        keys.insert(key);
    }
    
    //! @brief Whether the key is present in the value map or not for the void type.
    bool contains(T key) {
        return keys.count(key);
    }
};


}

#endif // FCPP_DATA_MULTITYPE_MAP_H_
