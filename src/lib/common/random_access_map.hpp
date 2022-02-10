// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file random_access_map.hpp
 * @brief Implementation of the `random_access_map` class template allowing random access iteration through an `unordered_map`.
 */

#ifndef FCPP_COMMON_RANDOM_ACCESS_MAP_H_
#define FCPP_COMMON_RANDOM_ACCESS_MAP_H_

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <unordered_map>
#include <vector>


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing objects of common use.
 */
namespace common {


//! @cond INTERNAL
namespace details {
    //! @brief The iterator type.
    /**
     * @brief Wraps an iterator I over a collection of iterators.
     *
     * @param I The underlying iterator type.
     * @param V The value type.
     * @param D The difference type.
     * @param P The pointer type.
     * @param R The reference type.
     */
    template <typename I, typename V, typename D, typename P, typename R>
    class iterator : public std::iterator<std::random_access_iterator_tag, V, D, P, R> {
      public:
        //! @name constructors
        //! @{
        //! @brief Default constructor.
        iterator() = default;

        //! @brief Copy constructor.
        iterator(iterator const&) = default;

        //! @brief Move constructor.
        iterator(iterator&&) = default;

        //! @brief Copy constructor from underlying iterator.
        iterator(I const& i) : m_it(i) {}

        //! @brief Move constructor from underlying iterator.
        iterator(I&& i) : m_it(i) {}
        //! @}

        //! @name assignment operators
        //! @{
        //! @brief Copy assignment.
        iterator& operator=(iterator const&) = default;

        //! @brief Move assignment.
        iterator& operator=(iterator&&) = default;

        //! @brief Swap function.
        friend void swap(iterator& i, iterator& j) {
            using std::swap;
            swap(i.m_it, j.m_it);
        }
        //! @}

        //! @brief Dereferencing operators.
        //! @{
        R operator*() const {
            return *(m_it[0]);
        }
        P operator->() const {
            return &(*(m_it[0]));
        }
        R operator[](size_t n) const {
            return *(m_it[n]);
        }
        operator I() const {
            return m_it;
        }
        //! @}

        //! @brief Arithmetic operators.
        //! @{
        iterator& operator++() {
            ++m_it;
            return *this;
        }
        iterator& operator--() {
            --m_it;
            return *this;
        }
        iterator operator++(int) {
            iterator it(*this);
            ++m_it;
            return it;
        }
        iterator operator--(int) {
            iterator it(*this);
            --m_it;
            return it;
        }
        iterator& operator+=(size_t n) {
            m_it+=n;
            return *this;
        }
        iterator& operator-=(size_t n) {
            m_it-=n;
            return *this;
        }
        iterator operator+(size_t n) const {
            return m_it+n;
        }
        iterator operator-(size_t n) const {
            return m_it-n;
        }
        D operator-(iterator const& o) const {
            return m_it - o.m_it;
        }
        friend iterator operator+(size_t n, iterator const& i) {
            return i+n;
        }
        //! @}

        //! @brief Relational operators.
        //! @{
        bool operator==(iterator const& o) const {
            return m_it == o.m_it;
        }
        bool operator!=(iterator const& o) const {
            return m_it != o.m_it;
        }
        bool operator<=(iterator const& o) const {
            return m_it <= o.m_it;
        }
        bool operator>=(iterator const& o) const {
            return m_it >= o.m_it;
        }
        bool operator<(iterator const& o) const {
            return m_it < o.m_it;
        }
        bool operator>(iterator const& o) const {
            return m_it > o.m_it;
        }
        //! @}

      private:
        //! @brief The actual iterator.
        I m_it;
    };
}
//! @endcond


/**
 * @brief Class providing an unordered map interface together with random access iterators.
 *
 * @param K Key type.
 * @param T Mapped type.
 * @param H Hasher type.
 * @param P Equality predicate type.
 * @param A Allocator type.
 */
template <typename K, typename T, typename H = std::hash<K>, typename P = std::equal_to<K>, typename A = std::allocator<std::pair<K const,T>>>
class random_access_map {
  private:
    //! @brief The internal unordered map type.
    using map_t = std::unordered_map<K,T,H,P,A>;
    //! @brief The map type from key to indices in the iterator vector.
    using idx_t = std::unordered_map<K,size_t,H,P>;
    //! @brief The internal vector type for random access.
    using vec_t = std::vector<typename map_t::iterator>;

  public:
    //! @brief The key type.
    using key_type = K;
    //! @brief The mapped type.
    using mapped_type = T;
    //! @brief The value type.
    using value_type = std::pair<key_type const, mapped_type>;
    //! @brief The hasher type.
    using hasher = H;
    //! @brief The comparison predicate type.
    using key_equal = P;
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
    using size_type = typename map_t::size_type;
    //! @brief The type for pointer differences.
    using difference_type = typename map_t::difference_type;
    //! @brief The iterator type.
    using iterator = details::iterator<typename vec_t::iterator, value_type, difference_type, pointer, reference>;
    //! @brief The const iterator type.
    using const_iterator = details::iterator<typename vec_t::const_iterator, value_type, difference_type, const_pointer, const_reference>;

  public:
    //! @name constructors
    //! @{

    //! @brief Default constructor.
    random_access_map() = default;

    //! @brief Copy constructor.
    random_access_map(random_access_map const&) = default;

    //! @brief Move constructor.
    random_access_map(random_access_map&&) = default;

    //! @brief Range constructor.
    template <typename I>
    random_access_map(I first, I last) : m_map(first, last) {
        for (auto it = m_map.begin(); it != m_map.end(); ++it) insert_impl(it);
    }

    //! @brief Initializer list constructor.
    random_access_map(std::initializer_list<value_type> il) : m_map(il) {
        for (auto it = m_map.begin(); it != m_map.end(); ++it) insert_impl(it);
    }
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Copy assignment.
    random_access_map& operator=(random_access_map const&) = default;

    //! @brief Move assignment.
    random_access_map& operator=(random_access_map&&) = default;

    //! @brief Initializer list assignment.
    random_access_map& operator=(std::initializer_list<value_type> il) {
        m_idx.clear();
        m_iter.clear();
        m_map = il;
        for (auto it = m_map.begin(); it != m_map.end(); ++it) insert_impl(it);
        return *this;
    }
    //! @}

    //! @brief Test whether the container is empty.
    bool empty() const noexcept {
        return m_map.empty();
    }

    //! @brief Returns the number of elements in the container.
    size_type size() const noexcept {
        return m_map.size();
    }

    //! @brief Returns the maximum number of elements that the container can hold due to system constraints.
    size_type max_size() const noexcept {
        return std::min(m_map.max_size(), m_idx.max_size(), m_iter.max_size());
    }

    //! @brief Returns an iterator pointing to the first element in the container.
    iterator begin() noexcept {
        return m_iter.begin();
    }

    //! @brief Returns an iterator pointing to the first element in the container (const overload).
    const_iterator begin() const noexcept {
        return m_iter.begin();
    }

    //! @brief Returns a const iterator pointing to the first element in the container.
    const_iterator cbegin() const noexcept {
        return m_iter.begin();
    }

    //! @brief Returns an iterator pointing to the past-the-end element in the container.
    iterator end() noexcept {
        return m_iter.end();
    }

    //! @brief Returns an iterator pointing to the past-the-end element in the container (const overload).
    const_iterator end() const noexcept {
        return m_iter.end();
    }

    //! @brief Returns a const iterator pointing to the past-the-end element in the container.
    const_iterator cend() const noexcept {
        return m_iter.end();
    }

    //! @brief Accesses an element of the container creating one if not found (const key overload).
    mapped_type& operator[](key_type const& k) {
        if (m_map.count(k) == 0) {
            m_map[k] = mapped_type();
            insert_impl(m_map.find(k));
        }
        return m_map[k];
    }

    //! @brief Accesses an element of the container creating one if not found (value key overload).
    mapped_type& operator[](key_type&& k) {
        if (m_map.count(k) == 0) {
            m_map[k] = mapped_type();
            insert_impl(m_map.find(k));
        }
        return m_map[std::move(k)];
    }

    //! @brief Accesses an element of the container throwing if not found.
    mapped_type& at(key_type const& k) {
        return m_map.at(k);
    }

    //! @brief Accesses an element of the container throwing if not found (const overload).
    mapped_type const& at(key_type const& k) const {
        return m_map.at(k);
    }

    //! @brief Searches the container for an element with a given key, returning end if not found.
    iterator find(key_type const& k) {
        return convert(m_map.find(k));
    }

    //! @brief Searches the container for an element with a given key, returning end if not found (const overload).
    const_iterator find(key_type const& k) const {
        return convert(m_map.find(k));
    }

    //! @brief Counts the elements with a specific key.
    size_type count(key_type const& k) const {
        return m_map.count(k);
    }

    //! @brief Constructs and inserts an element.
    template <class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        auto itb = m_map.emplace(args...);
        if (itb.second) insert_impl(itb.first);
        return {convert(itb.first), itb.second};
    }

    //! @brief Inserts new elements in the map (const value overload).
    std::pair<iterator,bool> insert(value_type const& val) {
        auto itb = m_map.insert(val);
        if (itb.second) insert_impl(itb.first);
        return {convert(itb.first), itb.second};
    }

    //! @brief Inserts new elements in the map (rvalue overload).
    std::pair<iterator,bool> insert(value_type&& val) {
        auto itb = m_map.insert(std::move(val));
        if (itb.second) insert_impl(itb.first);
        return {convert(itb.first), itb.second};
    }

    //! @brief Inserts new elements in the map (range overload).
    template <class I>
    void insert(I first, I last) {
        for (I it = first; it != last; ++it) insert(*it);
    }

    //! @brief Erases elements from the map (iterator overload).
    iterator erase(iterator position) {
        auto it = convert(position);
        if (it != m_map.end()) erase_impl(it);
        return convert(m_map.erase(it));
    }

    //! @brief Erases elements from the map (const iterator overload).
    iterator erase(const_iterator position) {
        auto it = convert(position);
        if (it != m_map.end()) erase_impl(it);
        return convert(m_map.erase(it));
    }

    //! @brief Erases elements from the map (key overload).
    size_type erase(key_type const& k) {
        auto it = m_map.find(k);
        if (it != m_map.end()) erase_impl(it);
        return m_map.erase(k);
    }

    //! @brief Erases elements from the map (range overload).
    iterator erase(iterator first, iterator last) {
        typename map_t::iterator f = convert(first);
        typename map_t::iterator l = convert(last);
        for (auto it = f; it != l; ++it) erase_impl(it);
        return convert(m_map.erase(first, last));
    }

    //! @brief Erases elements from the map (const range overload).
    iterator erase(const_iterator first, const_iterator last) {
        typename map_t::const_iterator f = convert(first);
        typename map_t::const_iterator l = convert(last);
        for (auto it = f; it != l; ++it) erase_impl(it);
        return convert(m_map.erase(first, last));
    }

    //! @brief Clear content
    void clear() noexcept {
        m_iter.clear();
        m_idx.clear();
        m_map.clear();
    }

    //! @brief Swaps content with another map.
    void swap(random_access_map& o) {
        using std::swap;
        swap(m_map,  o.m_map);
        swap(m_idx,  o.m_idx);
        swap(m_iter, o.m_iter);
    }

    //! @brief Equality operator.
    bool operator==(random_access_map const& o) const {
        return m_map == o.m_map;
    }

    //! @brief Inequality operator.
    bool operator!=(random_access_map const& o) const {
        return m_map != o.m_map;
    }

  private:
    //! @brief Convert a `map_t` iterator into an actual iterator.
    iterator convert(typename map_t::iterator it) {
        if (it == m_map.end()) return m_iter.end();
        return m_iter.begin() + m_idx[it->first];
    }
    const_iterator convert(typename map_t::const_iterator it) const {
        if (it == m_map.end()) return m_iter.end();
        return m_iter.begin() + m_idx[it->first];
    }

    //! @brief Convert an actual iterator into a `map_t` iterator.
    typename map_t::iterator convert(iterator it) {
        if (it == end()) return m_map.end();
        return *((typename vec_t::iterator)it);
    }
    typename map_t::const_iterator convert(const_iterator it) const {
        if (it == end()) return m_map.end();
        return *((typename vec_t::iterator)it);
    }

    //! @brief Erases from `m_idx` and `m_iter` something that is about to be erased from `m_map`.
    void erase_impl(typename map_t::iterator it) {
        m_idx[m_iter.back()->first] = m_idx[it->first];
        m_iter[m_idx[it->first]] = std::move(m_iter.back());
        m_iter.pop_back();
    }

    //! @brief Inserts in `m_idx` and `m_iter` something that has just been inserted into `m_map`.
    void insert_impl(typename map_t::iterator it) {
        m_idx[it->first] = m_iter.size();
        m_iter.push_back(it);
    }

    //! @brief The unordered map from keys to mapped types and vector indices.
    map_t m_map;
    //! @brief The map from keys to indices in the iterators vector.
    idx_t m_idx;
    //! @brief A vector of iterators into the unordered map.
    vec_t m_iter;
};


}


}

#endif // FCPP_COMMON_RANDOM_ACCESS_MAP_H_
