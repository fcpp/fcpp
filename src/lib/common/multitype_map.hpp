// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file multitype_map.hpp
 * @brief Implementation of the `multitype_map<T, Ts...>` class template for handling heterogeneous indexed data.
 */

#ifndef FCPP_COMMON_MULTITYPE_MAP_H_
#define FCPP_COMMON_MULTITYPE_MAP_H_

#include <cassert>
#include <ostream>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "lib/common/tagged_tuple.hpp"
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
 * \ref insert "Inserting" elements of types outside `Ts...` produces a compile-time error. Other kinds of access are supported on any types, where maps on unsupported types are assumed to be always empty (since it is not allowed to insert in them).
 *
 * @param T Key type.
 * @param Ts Admissible value types.
 */
template <typename T, typename... Ts>
class multitype_map {
    //! @brief Checks whether a type is supported by the map.
    template <typename A>
    constexpr static bool type_supported = type_count<std::remove_reference_t<A>, Ts...> != 0;

  public:
    //! @brief The type of the keys.
    typedef T key_type;

    //! @brief List of admissible types (without repetitions).
    using value_types = type_uniq<Ts...>;

    //! @brief List of map types (without repetitions).
    using map_types = type_uniq<std::unordered_map<T, Ts>...>;

    //! @name constructors
    //! @{
    /**
     * @brief Default constructor (creates an empty structure).
     */
    multitype_map() = default;

    //! @brief Copy constructor.
    multitype_map(multitype_map const&) = default;

    //! @brief Move constructor.
    multitype_map(multitype_map&&) = default;
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Copy assignment.
    multitype_map& operator=(multitype_map const&) = default;

    //! @brief Move assignment.
    multitype_map& operator=(multitype_map&&) = default;
    //! @}

    //! @brief Exchanges contents of multitype maps.
    void swap(multitype_map& m) {
        m_keys.swap(m.m_keys);
        m_data.swap(m.m_data);
    }

    //! @brief Equality operator.
    bool operator==(multitype_map const& o) const {
        return m_keys == o.m_keys and maps_compare(m_data, o.m_data, value_types{});
    }

    //! @cond INTERNAL
    #define MISSING_TYPE_MESSAGE ANSI_START "unsupported type access (add type A to exports type list)" ANSI_END
    //! @endcond

    //! @brief Inserts value at corresponding key.
    template<typename A>
    void insert(T key, A const& value) {
        get_map<A>(number_sequence<type_supported<A>>{})[key] = value;
        static_assert(type_supported<A>, MISSING_TYPE_MESSAGE);
    }

    //! @brief Inserts value at corresponding key by moving.
    template<typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
    void insert(T key, A&& value) {
        get_map<A>(number_sequence<type_supported<A>>{})[key] = std::move(value);
        static_assert(type_supported<A>, MISSING_TYPE_MESSAGE);
    }

    #undef MISSING_TYPE_MESSAGE

    //! @brief Inserts void value at corresponding key.
    void insert(T key) {
        m_keys.insert(key);
    }

    //! @brief Inserts the contents of another multitype_map.
    void insert(multitype_map const& m) {
        m_keys.insert(m.m_keys.begin(), m.m_keys.end());
        multi_insert(m, value_types{});
    }

    //! @brief Deletes value at corresponding key.
    template<typename A>
    void erase(T key) {
        get_map<A>(number_sequence<type_supported<A>>{}).erase(key);
    }

    //! @brief Deletes void value at corresponding key.
    void remove(T key) {
        m_keys.erase(key);
    }

    //! @brief Immutable reference to the value of a certain type at a given key.
    template<typename A>
    A const& at(T key) const {
        return get_map<A>(number_sequence<type_supported<A>>{}).at(key);
    }

    //! @brief Mutable reference to the value of a certain type at a given key.
    template<typename A>
    A& at(T key) {
        return get_map<A>(number_sequence<type_supported<A>>{}).at(key);
    }

    //! @brief Whether the key is present in the value map or not for a certain type.
    template<typename A>
    bool count(T key) const {
        return get_map<A>(number_sequence<type_supported<A>>{}).count(key);
    }

    //! @brief Whether the key is present in the value map or not for the void type.
    bool contains(T key) const {
        return m_keys.count(key);
    }

    //! @brief Prints the content of the multitype map.
    template <typename O, typename... Ss>
    void print(O& o, Ss... xs) const {
        m_data.print(o, xs...);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & m_data & m_keys;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << m_data << m_keys;
    }

  private:
    //! @brief Access to the map corresponding to a type.
    template <typename A>
    std::unordered_map<T, std::remove_reference_t<A>>& get_map(number_sequence<true>) {
        return get<std::remove_reference_t<A>>(m_data);
    }

    //! @brief Const access to the map corresponding to a type.
    template <typename A>
    std::unordered_map<T, std::remove_reference_t<A>> const& get_map(number_sequence<true>) const {
        return get<std::remove_reference_t<A>>(m_data);
    }

    //! @brief Access to a map corresponding to a missing type.
    template <typename A>
    std::unordered_map<T, std::remove_reference_t<A>>& get_map(number_sequence<false>) const {
        assert(false);
        return common::declare_reference<std::unordered_map<T, std::remove_reference_t<A>>>();
    }

    //! @brief Compares unordered maps, even in case `decltype(U == U)` is not implicitly convertible to bool.
    template <typename U>
    bool map_compare(std::unordered_map<T, U> const& x, std::unordered_map<T, U> const& y) const {
        if (x.size() != y.size()) return false;
        for (auto const& xi : x) {
            if (y.count(xi.first) == 0) return false;
            if (xi.second != y.at(xi.first)) return false;
        }
        return true;
    }

    //! @brief Compares tagged tuples of unordered maps (no elements).
    template <typename U>
    bool maps_compare(U const&, U const&, type_sequence<>) const {
        return true;
    }

    //! @brief Compares tagged tuples of unordered maps (some elements).
    template <typename U, typename S, typename... Ss>
    bool maps_compare(U const& x, U const& y, type_sequence<S, Ss...>) const {
        if (not map_compare(get<S>(x), get<S>(y))) return false;
        return maps_compare(x, y, type_sequence<Ss...>{});
    }

    //! @brief Inserts the data from another multitype map (empty form).
    inline void multi_insert(multitype_map const&, common::type_sequence<>) {}

    //! @brief Inserts the data from another multitype map (active form).
    template <typename S, typename... Ss>
    inline void multi_insert(multitype_map const& m, common::type_sequence<S, Ss...>) {
        get<S>(m_data).insert(get<S>(m.m_data).begin(), get<S>(m.m_data).end());
        multi_insert(m, common::type_sequence<Ss...>{});
    }

    //! @brief Map associating keys to data.
    tagged_tuple<value_types, map_types> m_data;
    //! @brief Set of keys (for void data).
    std::unordered_set<T> m_keys;
};


//! @brief Exchanges contents of multitype maps.
template <typename T, typename... Ts>
void swap(multitype_map<T, Ts...>& x, multitype_map<T, Ts...>& y) {
    x.swap(y);
}


}


}

#endif // FCPP_COMMON_MULTITYPE_MAP_H_
