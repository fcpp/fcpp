// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file immutable_map.hpp
 * @brief Implementation of the `immutable_map<K, T, A>` class template providing an immutable map interface based on sorted vectors.
 */

#ifndef FCPP_COMMON_IMMUTABLE_MAP_H_
#define FCPP_COMMON_IMMUTABLE_MAP_H_

#include <cassert>
#include <algorithm>
#include <vector>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


/**
 * @brief Class providing an immutable map interface based on sorted vectors.
 *
 * @param K Key type.
 * @param T Mapped type.
 * @param A Allocator type.
 */
template <typename K, typename T, typename A = std::allocator<std::pair<K const,T>>>
class immutable_map {
    //! @brief The underlying value type used by the stored vector.
    using val_type = std::pair<K, T>;
    //! @brief The underlying vector used to store key-value pairs.
    using vec_type = std::vector<std::pair<K, T>>;
    //! @brief The underlying vector used to stor key-value pairs, with const keys.
    using const_vec_type = std::vector<std::pair<K const, T>>;
  public:
    //! @brief The key type.
    using key_type = K;
    //! @brief The mapped type.
    using mapped_type = T;
    //! @brief The value type.
    using value_type = std::pair<key_type const, mapped_type>;
    //! @brief The allocator type.
    using allocator_type = A;
    //! @brief Reference type.
    using reference = value_type&;
    //! @brief Const reference type.
    using const_reference = value_type const&;
    //! @brief Pointer type.
    using pointer = typename std::allocator_traits<A>::pointer;
    //! @brief Const pointer type.
    using const_pointer = typename std::allocator_traits<A>::const_pointer;
    //! @brief The type for sizes.
    using size_type = typename const_vec_type::size_type;
    //! @brief The type for pointer differences.
    using difference_type = typename const_vec_type::difference_type;
    //! @brief The iterator type.
    using iterator = typename const_vec_type::iterator;
    //! @brief The const iterator type.
    using const_iterator = typename const_vec_type::const_iterator;

    //! @name constructors
    //! @{

    //! @brief Default constructor.
    immutable_map() = default;

    //! @brief Copy constructor.
    immutable_map(immutable_map const&) = default;

    //! @brief Move constructor.
    immutable_map(immutable_map&&) = default;

    //! @brief Range constructor.
    template <typename I>
    immutable_map(I first, I last) : m_data(first, last) {}

    //! @brief Initializer list constructor.
    immutable_map(std::initializer_list<val_type> il) : m_data(il) {}
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Copy assignment.
    immutable_map& operator=(immutable_map const&) = default;

    //! @brief Move assignment.
    immutable_map& operator=(immutable_map&&) = default;

    //! @brief Initializer list assignment.
    immutable_map& operator=(std::initializer_list<val_type> il) {
        m_data = il;
#ifndef NDEBUG
        m_freezed = false;
#endif
        return *this;
    }
    //! @}

    //! @brief Test whether the container is empty.
    inline bool empty() const noexcept {
        return m_data.empty();
    }

    //! @brief Returns the number of elements in the container.
    inline size_type size() const noexcept {
        return m_data.size();
    }

    //! @brief Returns the maximum number of elements that the container can hold due to system constraints.
    inline size_type max_size() const noexcept {
        return m_data.max_size();
    }

    //! @brief Returns an iterator pointing to the first element in the container.
    inline iterator begin() noexcept {
        return const_data().begin();
    }

    //! @brief Returns an iterator pointing to the first element in the container (const overload).
    inline const_iterator begin() const noexcept {
        return const_data().begin();
    }

    //! @brief Returns a const iterator pointing to the first element in the container.
    inline const_iterator cbegin() const noexcept {
        return const_data().begin();
    }

    //! @brief Returns an iterator pointing to the past-the-end element in the container.
    inline iterator end() noexcept {
        return const_data().end();
    }

    //! @brief Returns an iterator pointing to the past-the-end element in the container (const overload).
    inline const_iterator end() const noexcept {
        return const_data().end();
    }

    //! @brief Returns a const iterator pointing to the past-the-end element in the container.
    inline const_iterator cend() const noexcept {
        return const_data().end();
    }

    //! @brief Accesses an element of the container throwing if not found.
    mapped_type& at(key_type const& k) {
#ifndef NDEBUG
        assert(m_freezed);
#endif
        return lower_bound(m_data, k)->second;
    }

    //! @brief Accesses an element of the container throwing if not found (const overload).
    mapped_type const& at(key_type const& k) const {
#ifndef NDEBUG
        assert(m_freezed);
#endif
        return lower_bound(m_data, k)->second;
    }

    //! @brief Searches the container for an element with a given key, returning end if not found.
    iterator find(key_type const& k) {
#ifndef NDEBUG
        assert(m_freezed);
#endif
        auto it = lower_bound(const_data(), k);
        return it == const_data().end() ? it : it->first == k ? it : const_data().end();
    }

    //! @brief Searches the container for an element with a given key, returning end if not found (const overload).
    const_iterator find(key_type const& k) const {
#ifndef NDEBUG
        assert(m_freezed);
#endif
        auto it = lower_bound(const_data(), k);
        return it == const_data().end() ? it : it->first == k ? it : const_data().end();
    }

    //! @brief Counts the elements with a specific key.
    size_type count(key_type const& k) const {
#ifndef NDEBUG
        assert(m_freezed);
#endif
        auto it = lower_bound(m_data, k);
        return it == m_data.end() ? 0 : it->first == k;
    }

    //! @brief Constructs and inserts an element.
    template <class... Args>
    void emplace(Args&&... args) {
#ifndef NDEBUG
        assert(not m_freezed);
#endif
        m_data.emplace_back(std::forward<Args>(args)...);
    }

    //! @brief Inserts new elements in the map (const value overload).
    void insert(value_type const& val) {
#ifndef NDEBUG
        assert(not m_freezed);
#endif
        m_data.push_back(val);
    }

    //! @brief Inserts new elements in the map (rvalue overload).
    void insert(value_type&& val) {
#ifndef NDEBUG
        assert(not m_freezed);
#endif
        m_data.push_back(std::move(val));
    }

    //! @brief Inserts new elements in the map (range overload).
    template <class I>
    void insert(I first, I last) {
#ifndef NDEBUG
        assert(not m_freezed);
#endif
        m_data.insert(m_data.end(), first, last);
    }

    //! @brief Clear content
    inline void clear() noexcept {
        m_data.clear();
#ifndef NDEBUG
        m_freezed = false;
#endif
    }

    //! @brief Swaps content with another map.
    inline void swap(immutable_map& o) {
        using std::swap;
        swap(m_data,  o.m_data);
    }

    //! @brief Equality operator.
    inline bool operator==(immutable_map const& o) const {
        return m_data == o.m_data;
    }

    //! @brief Inequality operator.
    inline bool operator!=(immutable_map const& o) const {
        return m_data != o.m_data;
    }

    //! @brief Freezes the content of the immutable map, allowing to access it.
    inline void freeze() {
        std::sort(m_data.begin(), m_data.end(), [](val_type const& x, val_type const& y){
            return x.first < y.first;
        });
#ifndef NDEBUG
        assert(not m_freezed);
        m_freezed = true;
#endif
    }

    //! @brief Unfreezes the content of the immutable map, allowing to update it.
    inline void unfreeze() {
#ifndef NDEBUG
        assert(m_freezed);
        m_freezed = false;
#endif
    }

  private:
    //! @brief Searches for key `k` in vector of pairs `v`.
    template <typename V>
    inline auto lower_bound(V& v, key_type const& k) const {
        return std::lower_bound(v.begin(), v.end(), k, [](val_type const& x, key_type const& y){
            return x.first < y;
        });
    }
    //! @brief Accesses the vector with const keys.
    inline const_vec_type& const_data() {
        return reinterpret_cast<const_vec_type&>(m_data);
    }

    //! @brief Accesses the vector with const keys (const overload).
    inline const_vec_type const& const_data() const {
        return reinterpret_cast<const_vec_type const&>(m_data);
    }

    //! @brief The vector of keys and mapped values.
    vec_type m_data;

#ifndef NDEBUG
    //! @brief Whether the map is freezed.
    bool m_freezed = false;
#endif
};


//! @brief Exchanges contents of immutable maps.
template <typename K, typename T, typename A>
void swap(immutable_map<K, T, A>& x, immutable_map<K, T, A>& y) {
    x.swap(y);
}


}


}

#endif // FCPP_COMMON_IMMUTABLE_MAP_H_
