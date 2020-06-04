// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file field.hpp
 * @brief Implementation and helper functions for the `field<T>` class template for neighboring fields.
 */

#ifndef FCPP_DATA_FIELD_H_
#define FCPP_DATA_FIELD_H_

#include <cassert>

#include <algorithm>
#include <ostream>
#include <vector>

#include "lib/settings.hpp"
#include "lib/common/traits.hpp"
#include "lib/data/tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @cond INTERNAL
template<typename T> class field;
//! @endcond


/**
 * @name field traits
 *
 * Type operators convenient for field manipulation.
 */
//! @{
//! @brief Corresponds to T only if A is a local type.
template <typename A, typename T = void>
using if_local = std::enable_if_t<not common::has_template<field, A>, T>;
//! @brief Corresponds to T only if A is a field type.
template <typename A, typename T = void>
using if_field = std::enable_if_t<common::has_template<field, A>, T>;
//! @brief Type returned upon field collapsing.
template <typename A>
using to_local = common::extract_template<field, A>;
//! @brief Type returned upon field construction.
template <typename A>
using to_field = common::add_template<field, A>;
//! @brief Computes the result type of applying F pointwise to local versions of A.
template <typename F, typename... A>
using local_result = std::result_of_t<F(to_local<const A&>...)>;
//! @brief Computes the result type of applying F pointwise to local versions of A.
template <typename F, typename... A>
using field_result = field<local_result<F, A...>>;
//! @}


//! @cond INTERNAL
//! @brief Forward declarations for enabling friendships.
namespace details {
    template <bool b, typename T>
    struct field_base {};

    template <typename A>
    field<A> make_field(std::vector<device_t>&&, std::vector<A>&&);

    template <typename A>
    std::vector<device_t>& get_ids(field<A>&);
    template <typename A>
    std::vector<device_t>&& get_ids(field<A>&&);
    template <typename A>
    std::vector<device_t> const& get_ids(field<A> const&);

    template <typename A>
    std::vector<A>& get_vals(field<A>&);
    template <typename A>
    std::vector<A>&& get_vals(field<A>&&);
    template <typename A>
    std::vector<A> const& get_vals(field<A> const&);

    template <typename A, typename = if_local<A>>
    A const&& other(A&&);
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> other(A&&);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> other(A&&);

    template <typename A, typename = if_local<A>>
    A const&& self(A&&, device_t);
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> self(A&&, device_t);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> self(A&&, device_t);

    template <typename A, typename = if_local<A>>
    A&& align(A&&, std::vector<device_t> const&);
    template <typename A>
    field<A>& align(field<A>&, std::vector<device_t> const&);
    template <typename A>
    field<A> align(field<A>&&, std::vector<device_t> const&);
    template <typename A>
    field<A> align(field<A> const&, std::vector<device_t> const&);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    decltype(auto) align(A&&, std::vector<device_t> const&);
}
//! @endcond


/**
 * @brief Class representing a neighboring field of T values.
 */
template <typename T>
class field : public details::field_base<std::is_convertible<T, bool>::value, T> {
    static_assert(not common::has_template<field, T>, "cannot instantiate a field of fields");

    //! @cond INTERNAL
    //! @brief Class friendships
    //! @{
    template <typename A>
    friend class field;
    template <bool b, typename A>
    friend class details::field_base;

    //! @brief Function friendships
    //! @{
    template <typename A>
    friend field<A> details::make_field(std::vector<device_t>&&, std::vector<A>&&);

    template <typename A>
    friend std::vector<device_t>& details::get_ids(field<A>&);
    template <typename A>
    friend std::vector<device_t>&& details::get_ids(field<A>&&);
    template <typename A>
    friend std::vector<device_t> const& details::get_ids(field<A> const&);

    template <typename A>
    friend std::vector<A>& details::get_vals(field<A>&);
    template <typename A>
    friend std::vector<A>&& details::get_vals(field<A>&&);
    template <typename A>
    friend std::vector<A> const& details::get_vals(field<A> const&);
    //! @}
    //! @endcond

  public:
    //! @brief The type of the content.
    using value_type = T;

    //! @name constructors
    //! @{
    //! @brief Default constructor (dangerous: creates a field in an invalid state).
    field() = default;

    //! @brief Constant field (copying).
    field(const T& d) {
        m_vals.push_back(d);
    }

    //! @brief Constant field (moving).
    field(T&& d) {
        m_vals.push_back(std::move(d));
    }

    //! @brief Copy constructor.
    field(const field&) = default;

    //! @brief Move constructor.
    field(field&&) = default;

    //! @brief Implicit conversion copy constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field(const field<A>& f) : m_ids(f.m_ids), m_vals{f.m_vals.begin(), f.m_vals.end()} {}

    //! @brief Implicit conversion move constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field(field<A>&& f) : m_ids(std::move(f.m_ids)), m_vals{make_move_iterator(f.m_vals.begin()), make_move_iterator(f.m_vals.end())} {}
    //! @}

    //! @name assignment operators
    //! @{
    //! @brief Copy assignment.
    field& operator=(const field&) = default;

    //! @brief Move assignment.
    field& operator=(field&&) = default;

    //! @brief Implicit conversion copy assignment.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field& operator=(const field<A>& f) {
        m_ids = f.m_ids;
        m_vals.clear();
        m_vals.insert(m_vals.end(), f.m_vals.begin(), f.m_vals.end());
        return *this;
    }

    //! @brief Implicit conversion move assignment.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field& operator=(field<A>&& f) {
        m_ids = std::move(f.m_ids);
        m_vals.clear();
        m_vals.insert(m_vals.end(), make_move_iterator(f.m_vals.begin()), make_move_iterator(f.m_vals.end()));
        return *this;
    }
    //! @}

    //! @brief Exchanges the content of the `field` objects.
    void swap(field& f) {
        swap(m_ids,  f.m_ids);
        swap(m_vals, f.m_vals);
    }

  private:
    //! @brief Ordered IDs of exceptions.
    std::vector<device_t> m_ids;

    //! @brief Corresponding values of exceptions (default value in position 0).
    std::vector<T> m_vals;

    //! @brief Member constructor, for internal use only.
    field(std::vector<device_t>&& ids, std::vector<T>&& vals) : m_ids(std::move(ids)), m_vals(std::move(vals)) {}
};


//! @cond INTERNAL
namespace details {
    //! @brief Additional cast to bool for field of type convertible to bool.
    template <typename T>
    struct field_base<true, T> {
        operator bool() const {
            const field<T>& f = *((const field<T>*)this);
            for (const auto& x : f.m_vals) if (not ((bool)x)) return false;
            return true;
        }
    };

    //! @brief Builds a field from member values.
    template <typename A>
    field<A> make_field(std::vector<device_t>&& ids, std::vector<A>&& vals) {
        return {std::move(ids), std::move(vals)};
    }

    //! @brief Accesses the private field `m_ids` of a field.
    //! @{
    template <typename A>
    std::vector<device_t>& get_ids(field<A>& f) {
        return f.m_ids;
    }
    template <typename A>
    std::vector<device_t>&& get_ids(field<A>&& f) {
        return std::move(f.m_ids);
    }
    template <typename A>
    std::vector<device_t> const& get_ids(field<A> const& f) {
        return f.m_ids;
    }
    //! @}

    //! @brief Accesses the private field `m_vals` of a field.
    //! @{
    template <typename A>
    std::vector<A>& get_vals(field<A>& f) {
        return f.m_vals;
    }
    template <typename A>
    std::vector<A>&& get_vals(field<A>&& f) {
        return std::move(f.m_vals);
    }
    template <typename A>
    std::vector<A> const& get_vals(field<A> const& f) {
        return f.m_vals;
    }
    //! @}

    /**
     * @name other
     *
     * Accessor method selecting the default value of a given field.
     */
    //! @{
    //! @brief Const access for non-field values (treated as constant fields).
    template <typename A, typename = if_local<A>>
    A const&& other(A&& x) {
        return static_cast<A const&&>(x);
    }

    //! @brief Full access on fields. WARNING: may lead to unexpected results if the argument is not aligned.
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> other(A&& x) {
        return static_cast<to_local<A&&>>(get_vals(std::forward<A>(x))[0]);
    }

    //! @brief Full access on indexed structures.
    template <typename A, size_t... is>
    to_local<A&&> other(A&& x, std::index_sequence<is...>) {
        return {other(get<is>(std::forward<A>(x)))...};
    }

    //! @brief Full access on tuples.
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> other(A&& x) {
        return other(std::forward<A>(x), std::make_index_sequence<common::template_args<A>::size>{});
    }
    //! @}

    /**
     * @name self
     *
     * Accesses the value from a field corresponing to a certain device.
     */
    //! @{
    //! @brief Const access for non-field values (treated as constant fields).
    template <typename A, typename = if_local<A>>
    A const&& self(A&& x, device_t) {
        return static_cast<A const&&>(x);
    }

    template <typename A>
    to_local<field<A>&> maybe_emplace(field<A>& f, device_t i, size_t pos) {
        get_ids(f).insert(get_ids(f).begin() + pos, i);
        get_vals(f).insert(get_vals(f).begin() + pos+1, get_vals(f)[0]);
        return get_vals(f)[pos+1];
    }
    template <typename A>
    to_local<field<A> const&> maybe_emplace(field<A> const& f, device_t, size_t) {
        return get_vals(f)[0];
    }
    template <typename A>
    to_local<field<A>&&> maybe_emplace(field<A>&& f, device_t, size_t) {
        return get_vals(std::move(f))[0];
    }
    //! @brief Full access on fields.
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> self(A&& x, device_t i) {
        auto it = std::lower_bound(get_ids(x).begin(), get_ids(x).end(), i);
        if (it == get_ids(x).end() or *it != i)
            return static_cast<to_local<A&&>>(maybe_emplace(std::forward<A>(x), i, it - get_ids(x).begin()));
        return static_cast<to_local<A&&>>(get_vals(x)[it - get_ids(x).begin() + 1]);
    }

    //! @brief Full access on indexed structures.
    template <typename A, size_t... is>
    to_local<A&&> self(A&& x, device_t i, std::index_sequence<is...>) {
        return {self(get<is>(std::forward<A>(x)), i)...};
    }

    //! @brief Full access on tuples.
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> self(A&& x, device_t i) {
        return self(std::forward<A>(x), i, std::make_index_sequence<common::template_args<A>::size>{});
    }
    //! @}

    /**
     * @name align
     *
     * Computes the restriction of a field to a given domain.
     * The resulting field has exactly the given domain.
     */
    //! @{
    //! @brief align of locals.
    template <typename A, typename = if_local<A>>
    A&& align(A&& x, std::vector<device_t> const&) {
        return std::forward<A>(x);
    }

    //! @brief align of fields.
    template <typename A>
    field<A>& align(field<A>& x, std::vector<device_t> const& s) {
        size_t rx = 0, wx = 0, ks = 0;
        while (ks < s.size() and rx < get_ids(x).size()) {
            if      (s[ks] < get_ids(x)[rx]) ++ks;
            else if (s[ks] > get_ids(x)[rx]) ++rx;
            else {
                if (rx > wx) {
                    get_ids(x)[wx] = get_ids(x)[rx];
                    get_vals(x)[wx+1] = std::move(get_vals(x)[rx+1]);
                }
                ++ks, ++rx, ++wx;
            }
        }
        if (wx < get_ids(x).size()) {
            get_ids(x).resize(wx);
            get_vals(x).resize(wx+1);
        }
        return x;
    }
    template <typename A>
    field<A> align(field<A>&& x, std::vector<device_t> const& s) {
        align(x, s);
        return x;
    }
    template <typename A>
    field<A> align(field<A> const& x, std::vector<device_t> const& s) {
        std::vector<device_t> ids;
        std::vector<A> vals;
        ids.reserve(get_ids(x).size());
        vals.reserve(get_vals(x).size());
        vals.push_back(get_vals(x)[0]);
        size_t rx = 0, ks = 0;
        while (ks < s.size() and rx < get_ids(x).size()) {
            if      (s[ks] < get_ids(x)[rx]) ++ks;
            else if (s[ks] > get_ids(x)[rx]) ++rx;
            else {
                ids.push_back(get_ids(x)[rx]);
                vals.push_back(get_vals(x)[rx+1]);
                ++ks, ++rx;
            }
        }
        return make_field(std::move(ids), std::move(vals));
    }

    //! @brief align of tuples.
    template <typename... A, size_t... is>
    tuple<A...>& align(tuple<A...>& x, std::vector<device_t> const& s, std::index_sequence<is...>) {
        ignore_args(align(get<is>(x), s)...);
        return x;
    }
    template <typename... A, size_t... is>
    tuple<A...> align(tuple<A...>&& x, std::vector<device_t> const& s, std::index_sequence<is...>) {
        ignore_args(align(get<is>(x), s)...);
        return x;
    }
    template <typename... A, size_t... is>
    tuple<A...> align(tuple<A...> const& x, std::vector<device_t> const& s, std::index_sequence<is...>) {
        return {align(get<is>(x), s)...};
    }
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    decltype(auto) align(A&& x, std::vector<device_t> const& s) {
        return align(std::forward<A>(x), s, std::make_index_sequence<common::template_args<A>::size>{});
    }
    //! @}

    /**
     * @name field iterator
     *
     * Iterates through the values of a field-like object.
     */
    //! @{
    //! @brief Local case.
    template <typename T, typename = void>
    class field_iterator {
      public:
        //! @brief Constructor.
        field_iterator(T& ref) : m_ref(ref) {}

        //! @brief Deleted constructor on temporary values.
        field_iterator(T const&&) = delete;

        //! @brief Checks if the iterator reached the end.
        inline bool end() const {
            return true;
        }

        //! @brief Accesses the device id.
        inline device_t id() const {
            return std::numeric_limits<device_t>::max();
        }

        //! @brief Accesses the value.
        inline to_local<T const&> value(device_t = 0) const {
            return m_ref;
        }

        //! @brief Increments the iterator.
        inline field_iterator& operator++() {
            return *this;
        }

      private:
        //! @brief Reference to the base object.
        T& m_ref;
    };
    //! @brief Const field case.
    template <typename T>
    class field_iterator<field<T> const, void> {
      public:
        //! @brief Constructor.
        field_iterator(field<T> const& ref) : m_ref(ref) {}

        //! @brief Deleted constructor on temporary values.
        field_iterator(field<T> const&&) = delete;

        //! @brief Checks if the iterator reached the end.
        inline bool end() const {
            return m_i == get_ids(m_ref).size();
        }

        //! @brief Accesses the device id.
        inline device_t id() const {
            if (end()) return std::numeric_limits<device_t>::max();
            return get_ids(m_ref)[m_i];
        }

        //! @brief Accesses the value.
        inline to_local<field<T> const&> value() const {
            if (end()) return get_vals(m_ref)[0];
            return get_vals(m_ref)[m_i+1];
        }

        //! @brief Accesses the value (given a device id).
        inline to_local<field<T> const&> value(device_t i) const {
            if (id() == i) return get_vals(m_ref)[m_i+1];
            return get_vals(m_ref)[0];
        }

        //! @brief Increments the iterator.
        inline field_iterator& operator++() {
            ++m_i;
            return *this;
        }

      private:
        //! @brief Reference to the base object.
        field<T> const& m_ref;
        //! @brief Index.
        size_t m_i = 0;
    };
    //! @brief Field case.
    template <typename T>
    class field_iterator<field<T>, void> {
      public:
        //! @brief Constructor.
        field_iterator(field<T>& ref) : m_ref(ref) {}

        //! @brief Deleted constructor on temporary values.
        field_iterator(field<T> const&&) = delete;

        //! @brief Checks if the iterator reached the end.
        inline bool end() const {
            return m_i == get_ids(m_ref).size();
        }

        //! @brief Accesses the device id.
        inline device_t id() const {
            if (end()) return std::numeric_limits<device_t>::max();
            return get_ids(m_ref)[m_i];
        }

        //! @brief Accesses the value.
        inline to_local<field<T> const&> value() const {
            if (end()) return get_vals(m_ref)[0];
            return get_vals(m_ref)[m_i+1];
        }

        //! @brief Accesses the value (given a device id).
        inline to_local<field<T> const&> value(device_t i) const {
            if (id() == i) return get_vals(m_ref)[m_i+1];
            return get_vals(m_ref)[0];
        }

        //! @brief Inserts a value into the field before the current element.
        field_iterator& emplace(device_t i, T&& v) {
            if (id() == i) {
                get_vals(m_ref)[m_i+1] = std::move(v);
            } else {
                get_ids(m_ref).insert(get_ids(m_ref).begin() + m_i, i);
                get_vals(m_ref).insert(get_vals(m_ref).begin() + m_i+1, std::move(v));
            }
            return *this;
        }

        //! @brief Increments the iterator.
        inline field_iterator& operator++() {
            ++m_i;
            return *this;
        }

      private:
        //! @brief Reference to the base object.
        field<T>& m_ref;
        //! @brief Index.
        size_t m_i = 0;
    };
    //! @brief Const tuple case.
    template <typename... Ts>
    class field_iterator<tuple<Ts...> const, if_field<tuple<Ts...>>> {
          template <size_t i, typename T>
          friend auto& get(field_iterator<T>&);
          template <size_t i, typename T>
          friend auto const& get(field_iterator<T> const&);
      public:
        //! @brief Constructor.
        field_iterator(tuple<Ts...> const& ref) : field_iterator(ref, std::make_index_sequence<sizeof...(Ts)>{}) {}

        //! @brief Deleted constructor on temporary values.
        field_iterator(tuple<Ts...> const&&) = delete;

        //! @brief Unpacked constructor.
        field_iterator(Ts const&... refs) : m_its(field_iterator<Ts const>{refs}...) {
            init(std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Checks if the iterator reached the end.
        inline bool end() const {
            return m_id == std::numeric_limits<device_t>::max();
        }

        //! @brief Accesses the device id.
        inline device_t id() const {
            return m_id;
        }

        //! @brief Accesses the value.
        inline to_local<tuple<Ts...> const&> value() const {
            return value(m_id, std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Accesses the value (given a device id).
        inline to_local<tuple<Ts...> const&> value(device_t i) const {
            return value(i, std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Applies a function to the values of the iterated pair.
        template <typename F>
        inline local_result<F,Ts...> apply(F&& op) const {
            return apply(std::forward<F>(op), std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Increments the iterator.
        inline field_iterator& operator++() {
            increment(std::make_index_sequence<sizeof...(Ts)>{});
            return *this;
        }

      private:
        //! @brief Constructor (with index sequence).
        template <size_t... is>
        field_iterator(tuple<Ts...> const& ref, std::index_sequence<is...>) : m_its(field_iterator<Ts const>{get<is>(ref)}...) {
            init(std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Initialises the current iterated id.
        template <size_t... is>
        inline void init(std::index_sequence<is...>) {
            device_t ids[] = {get<is>(m_its).id()...};
            m_id = *std::min_element(ids, ids + sizeof...(Ts));
        }

        //! @brief Accesses the value (with index sequence).
        template <size_t... is>
        inline to_local<tuple<Ts...> const&> value(device_t i, std::index_sequence<is...>) const {
            return {get<is>(m_its).value(i)...};
        }

        //! @brief Applies a function to the values of the iterated pair (with index sequence).
        template <typename F, size_t... is>
        inline local_result<F,Ts...> apply(F&& op, std::index_sequence<is...>) const {
            return op(get<is>(m_its).value(m_id)...);
        }

        //! @brief Increments the iterator (with index sequence).
        template <size_t... is>
        inline void increment(std::index_sequence<is...>) {
            device_t ids[] = {(get<is>(m_its).id() == m_id ? ++get<is>(m_its) : get<is>(m_its)).id()...};
            m_id = *std::min_element(ids, ids + sizeof...(Ts));
        }

        //! @brief A tuple of iterators to the components.
        tuple<field_iterator<Ts const>...> m_its;
        //! @brief The current iterated id.
        device_t m_id;
    };
    //! @brief Tuple case.
    template <typename... Ts>
    class field_iterator<tuple<Ts...>, if_field<tuple<Ts...>>> {
          template <size_t i, typename T>
          friend auto& get(field_iterator<T>&);
          template <size_t i, typename T>
          friend auto const& get(field_iterator<T> const&);
      public:
        //! @brief Constructor.
        field_iterator(tuple<Ts...>& ref) : field_iterator(ref, std::make_index_sequence<sizeof...(Ts)>{}) {}

        //! @brief Deleted constructor on temporary values.
        field_iterator(tuple<Ts...> const&&) = delete;

        //! @brief Unpacked constructor.
        field_iterator(Ts&... refs) : m_its(field_iterator<Ts>{refs}...) {
            init(std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Checks if the iterator reached the end.
        inline bool end() const {
            return m_id == std::numeric_limits<device_t>::max();
        }

        //! @brief Accesses the device id.
        inline device_t id() const {
            return m_id;
        }

        //! @brief Accesses the value.
        inline to_local<tuple<Ts...> const&> value() const {
            return value(m_id, std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Accesses the value (given a device id).
        inline to_local<tuple<Ts...> const&> value(device_t i) const {
            return value(i, std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Applies a function to the values of the iterated pair.
        template <typename F>
        inline local_result<F,Ts...> apply(F&& op) const {
            return apply(std::forward<F>(op), std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Inserts a value into the field before the current element.
        template <typename... Us>
        field_iterator& emplace(device_t i, tuple<Us...>&& v) {
            return emplace(i, std::move(v), std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Increments the iterator.
        inline field_iterator& operator++() {
            increment(std::make_index_sequence<sizeof...(Ts)>{});
            return *this;
        }

      private:
        //! @brief Constructor (with index sequence).
        template <size_t... is>
        field_iterator(tuple<Ts...>& ref, std::index_sequence<is...>) : m_its(field_iterator<Ts>{get<is>(ref)}...) {
            init(std::make_index_sequence<sizeof...(Ts)>{});
        }

        //! @brief Initialises the current iterated id.
        template <size_t... is>
        inline void init(std::index_sequence<is...>) {
            device_t ids[] = {get<is>(m_its).id()...};
            m_id = *std::min_element(ids, ids + sizeof...(Ts));
        }

        //! @brief Accesses the value (with index sequence).
        template <size_t... is>
        inline to_local<tuple<Ts...> const&> value(device_t i, std::index_sequence<is...>) const {
            return {get<is>(m_its).value(i)...};
        }

        //! @brief Applies a function to the values of the iterated pair (with index sequence).
        template <typename F, size_t... is>
        inline local_result<F,Ts...> apply(F&& op, std::index_sequence<is...>) const {
            return op(get<is>(m_its).value(m_id)...);
        }

        //! @brief Inserts a value into the field before the current element (with index sequence).
        template <typename... Us, size_t... is>
        field_iterator& emplace(device_t i, tuple<Us...>&& v, std::index_sequence<is...>) {
            ignore_args(get<is>(m_its).emplace(i, std::move(get<is>(v)))...);
            m_id = i;
            return *this;
        }

        //! @brief Increments the iterator (with index sequence).
        template <size_t... is>
        inline void increment(std::index_sequence<is...>) {
            device_t ids[] = {(get<is>(m_its).id() == m_id ? ++get<is>(m_its) : get<is>(m_its)).id()...};
            m_id = *std::min_element(ids, ids + sizeof...(Ts));
        }

        //! @brief A tuple of iterators to the components.
        tuple<field_iterator<Ts>...> m_its;
        //! @brief The current iterated id.
        device_t m_id;
    };
    //! @}

    /**
     * @name get
     *
     * Accesses the i-th sub-iterator of a field iterator on a tuple.
     */
    //! @{
    //! @brief Full access.
    template <size_t i, typename T>
    inline auto& get(field_iterator<T>& it) {
        return get<i>(it.m_its);
    }
    //! @brief Const access.
    template <size_t i, typename T>
    inline auto const& get(field_iterator<T> const& it) {
        return get<i>(it.m_its);
    }
    //! @}

    /**
     * @name map_hood
     *
     * Applies an operator pointwise on a sequence of fields.
     */
    //! @{
    //! @brief General case with some field argument.
    template <typename F, typename... A, typename = if_field<tuple<A...>>>
    field_result<F,A...> map_hood(F&& op, A const&... a) {
        field_result<F,A...> r(op(other(a)...));
        for (field_iterator<tuple<A...> const> it(a...); not it.end(); ++it) {
            get_ids(r).push_back(it.id());
            get_vals(r).push_back(it.apply(op));
        }
        return r;
    }
    //! @brief Optimisation for all local arguments.
    template <typename F, typename... L, typename = if_local<tuple<L...>>>
    field_result<F,L...> map_hood(F&& op, L&&... l) {
        return op(std::forward<L>(l)...);
    }
    //! @brief Optimisation for a single movable field argument in first position.
    template <typename F, typename T, typename... L, typename = if_local<tuple<L...>>>
    field_result<F,field<T>,L...> map_hood(F&& op, field<T>&& f, L&&... l) {
        field_result<F,field<T>,L...> r;
        get_ids(r) = get_ids(std::move(f));
        get_vals(r).resize(get_vals(f).size());
        for (size_t i = 0; i < get_vals(f).size(); ++i)
                get_vals(r)[i] = op(get_vals(std::move(f))[i], l...);
        return r;
    }
    //! @brief Optimisation for a single immutable field argument in first position.
    template <typename F, typename T, typename... L, typename = if_local<tuple<L...>>>
    field_result<F,field<T>,L...> map_hood(F&& op, field<T> const& f, L&&... l) {
        field_result<F,field<T>,L...> r;
        get_ids(r) = get_ids(f);
        get_vals(r).resize(get_vals(f).size());
        for (size_t i = 0; i < get_vals(f).size(); ++i)
                get_vals(r)[i] = op(get_vals(f)[i], l...);
        return r;
    }
    //! @brief Optimisation for a single movable field argument in second position.
    template <typename F, typename A, typename T, typename... L, typename = if_local<tuple<A,L...>>>
    field_result<F,A,field<T>,L...> map_hood(F&& op, A&& a, field<T>&& f, L&&... l) {
        field_result<F,A,field<T>,L...> r;
        get_ids(r) = get_ids(std::move(f));
        get_vals(r).resize(get_vals(f).size());
        for (size_t i = 0; i < get_vals(f).size(); ++i)
                get_vals(r)[i] = op(a, get_vals(std::move(f))[i], l...);
        return r;
    }
    //! @brief Optimisation for a single immutable field argument in second position.
    template <typename F, typename A, typename T, typename... L, typename = if_local<tuple<A,L...>>>
    field_result<F,A,field<T>,L...> map_hood(F&& op, A&& a, field<T> const& f, L&&... l) {
        field_result<F,A,field<T>,L...> r;
        get_ids(r) = get_ids(f);
        get_vals(r).resize(get_vals(f).size());
        for (size_t i = 0; i < get_vals(f).size(); ++i)
                get_vals(r)[i] = op(a, get_vals(f)[i], l...);
        return r;
    }
    //! @brief Optimisation for a single movable field argument in third position.
    template <typename F, typename A, typename B, typename T, typename... L, typename = if_local<tuple<A,B,L...>>>
    field_result<F,A,B,field<T>,L...> map_hood(F&& op, A&& a, B&& b, field<T>&& f, L&&... l) {
        field_result<F,A,B,field<T>,L...> r;
        get_ids(r) = get_ids(std::move(f));
        get_vals(r).resize(get_vals(f).size());
        for (size_t i = 0; i < get_vals(f).size(); ++i)
                get_vals(r)[i] = op(a, b, get_vals(std::move(f))[i], l...);
        return r;
    }
    //! @brief Optimisation for a single immutable field argument in third position.
    template <typename F, typename A, typename B, typename T, typename... L, typename = if_local<tuple<A,B,L...>>>
    field_result<F,A,B,field<T>,L...> map_hood(F&& op, A&& a, B&& b, field<T> const& f, L&&... l) {
        field_result<F,A,B,field<T>,L...> r;
        get_ids(r) = get_ids(f);
        get_vals(r).resize(get_vals(f).size());
        for (size_t i = 0; i < get_vals(f).size(); ++i)
                get_vals(r)[i] = op(a, b, get_vals(f)[i], l...);
        return r;
    }
    //! @brief Optimisation for two field arguments in starting position.
    template <typename F, typename T, typename U, typename... L, typename = if_local<tuple<L...>>>
    field_result<F,field<T>,field<U>,L...> map_hood(F&& op, field<T> const& f, field<U> const& g, L&&... l) {
        field_result<F,field<T>,field<U>,L...> r;
        get_ids(r).reserve(get_ids(f).size() + get_ids(g).size());
        get_vals(r).reserve(get_ids(f).size() + get_ids(g).size() + 1);
        get_vals(r).push_back(op(get_vals(f)[0], get_vals(g)[0], l...));
        size_t i = 0, j = 0;
        while (i < get_ids(f).size() or j < get_ids(g).size()) {
            if (i == get_ids(f).size()) {
                get_ids(r).push_back(get_ids(g)[j]);
                get_vals(r).push_back(op(get_vals(f)[0], get_vals(g)[++j], l...));
            } else if (j == get_ids(g).size() or get_ids(f)[i] < get_ids(g)[j]) {
                get_ids(r).push_back(get_ids(f)[i]);
                get_vals(r).push_back(op(get_vals(f)[++i], get_vals(g)[0], l...));
            } else if (get_ids(f)[i] > get_ids(g)[j]) {
                get_ids(r).push_back(get_ids(g)[j]);
                get_vals(r).push_back(op(get_vals(f)[0], get_vals(g)[++j], l...));
            } else {
                get_ids(r).push_back(get_ids(f)[i]);
                get_vals(r).push_back(op(get_vals(f)[++i], get_vals(g)[++j], l...));
            }
        }
        return r;
    }
    //! @}

    /**
     * @name mod_hood
     *
     * Modifies a field in-place, by applying an operator pointwise (with a sequence of parameters).
     */
    //! @{
    //! @brief General case with some field argument.
    template <typename F, typename A, typename... L>
    if_field<tuple<A,L...>, A&> mod_hood(F&& op, A& a, L const&... l) {
        for (field_iterator<tuple<A,L const...>> it(a,l...); not it.end(); ++it)
            get<0>(it).emplace(it.id(), it.apply(op));
        other(a) = op(other(a), other(l)...);
        return a;
    }
    //! @brief Optimisation for all local arguments.
    template <typename F, typename A, typename... L>
    if_local<tuple<A,L...>, A&> mod_hood(F&& op, A& a, L&&... l) {
        return a = op(a, std::forward<L>(l)...);
    }
    //! @brief Optimisation for a single field argument in first position.
    template <typename F, typename A, typename... L>
    if_local<tuple<L...>, field<A>&> mod_hood(F&& op, field<A>& a, L const&... l) {
        for (typename std::vector<A>::reference x : get_vals(a)) x = op(x, l...);
        return a;
    }
    //! @}

    /**
     * @name fold_hood
     *
     * Reduces the values in a part of a field (determined by domain) to a single value through a binary operation.
     */
    //! @{
    //! @brief Inclusive folding (optimization for locals).
    template <typename F, typename A>
    if_local<A, local_result<F,A,A>>
    fold_hood(F&& op, const A& x, std::vector<device_t> const& dom) {
        assert(dom.size() > 0);
        size_t n = dom.size();
        local_result<F,A,A> res = x;
        for (--n; n>0; --n) res = op(x, res);
        return res;
    }
    //! @brief Inclusive folding.
    template <typename F, typename A>
    if_field<A, local_result<F,A,A>>
    fold_hood(F&& op, A const& f, std::vector<device_t> const& dom) {
        assert(dom.size() > 0);
        field_iterator<A const> it(f);
        while (it.id() < dom[0]) ++it;
        local_result<F,A,A> res = it.value(dom[0]);
        for (size_t k=1; k<dom.size(); ++k) {
            while (it.id() < dom[k]) ++it;
            res = op(it.value(dom[k]), res);
        }
        return res;
    }
    //! @brief Exclusive folding (optimization for locals).
    template <typename F, typename A, typename B>
    if_local<A, local_result<F,A,B>>
    fold_hood(F&& op, const A& x, const B& b, std::vector<device_t> const& dom, device_t i) {
        assert(std::binary_search(dom.begin(), dom.end(), i));
        local_result<F,A,B> res = details::self(b, i);
        for (size_t n = dom.size(); n>1; --n) res = op(x, res);
        return res;
     }
    //! @brief Exclusive folding.
    template <typename F, typename A, typename B>
    if_field<A, local_result<F,A,B>>
    fold_hood(F&& op, A const& f, B const& b, std::vector<device_t> const& dom, device_t i) {
        assert(std::binary_search(dom.begin(), dom.end(), i));
        local_result<F,A,B> res = self(b, i);
        field_iterator<A const> it(f);
        for (size_t k=0; k<dom.size(); ++k) if (dom[k] != i) {
            while (it.id() < dom[k]) ++it;
            res = op(it.value(dom[k]), res);
        }
        return res;
    }
    //! @}
}
//! @endcond


//! @brief Prints a field in dictionary-like format.
template <typename A>
std::ostream& operator<<(std::ostream& o, const field<A>& x) {
    o << "{";
    for (size_t i = 0; i < details::get_ids(x).size(); ++i) {
        o << get_ids(x)[i] << ":" << get_vals(x)[i+1] << ", ";
    }
    return o << "*:" << details::get_vals(x)[0] << "}";
}


/**
 * @name mux
 *
 * Multiplexer operator, choosing between its arguments based on the value of the first
 * (always evaluating both arguments).
 */
//! @{
//! @brief local guard
template <typename A>
const A& mux(bool b, const A& x, const A& y) {
    return b ? x : y;
}
//! @brief local guard, moving arguments
template <typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
A mux(bool b, A&& x, A&& y) {
    return b ? std::move(x) : std::move(y);
}
//! @brief field guard
template <typename A>
to_field<A> mux(field<bool> b, const A& x, const A& y) {
    return map_hood([] (bool b, to_local<const A&> x, to_local<const A&> y) -> common::del_template<field, A> {
        return b ? x : y;
    }, b, x, y);
}
//! @brief field guard, moving arguments
template <typename A, typename = std::enable_if_t<not std::is_reference<A>::value>>
to_field<A> mux(field<bool> b, A&& x, A&& y) {
    return map_hood([] (bool b, to_local<const A&> x, to_local<const A&> y) -> common::del_template<field, A> {
        return b ? std::move(x) : std::move(y);
    }, b, x, y);
}
//! @}


/**
 * @name max
 *
 * Maximum between two values.
 */
//! @{
//! @brief max between locals.
template <typename A, typename = if_local<A>>
const A& max(const A& x, const A& y) {
    return std::max(x, y);
}

//! @brief max between fields.
template <typename A, typename = if_field<A>>
to_field<A> max(const A& x, const A& y) {
    return map_hood([] (to_local<const A&> x, to_local<const A&> y) -> common::del_template<field, A> {
        return std::max(x, y);
    }, x, y);
}
//! @}


/**
 * @name min
 *
 * Minimum between two values.
 */
//! @{
//! @brief min between locals.
template <typename A, typename = if_local<A>>
const A& min(const A& x, const A& y) {
    return std::min(x, y);
}

//! @brief min between fields.
template <typename A, typename = if_field<A>>
to_field<A> min(const A& x, const A& y) {
    return map_hood([] (to_local<const A&> x, to_local<const A&> y) -> common::del_template<field, A> {
        return std::min(x, y);
    }, x, y);
}
//! @}


//! @brief Extracts a component from a field of tuple-like structures.
template <size_t n, typename A>
auto get(const field<A>& f) {
    return map_hood([] (const A& x) {
        return get<n>(x);
    }, f);
}


//! @cond INTERNAL
#define _BOP_TYPE(A,op,B)                                                           \
field<decltype(std::declval<to_local<A>>() op std::declval<to_local<B>>())>
//! @endcond

/**
 * @brief Overloads unary operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_UOP(op)                                                                \
template <typename A>                                                               \
field<A> operator op(const field<A>& x) {                                           \
    return details::map_hood([] (const A& a) {return op a;}, x);                    \
}                                                                                   \
template <typename A>                                                               \
field<A> operator op(field<A>&& x) {                                                \
    details::mod_hood([] (const A& a) {return op std::move(a);}, x);                \
    return x;                                                                       \
}

/**
 * @brief Overloads binary operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_BOP(op)                                                                                    \
template <typename A, typename B>                                                                       \
_BOP_TYPE(field<A>,op,B) operator op(const field<A>& x, const B& y) {                                   \
    return details::map_hood([](const A& a, const to_local<B>& b) { return a op b; }, x, y);            \
}                                                                                                       \
template <typename A, typename B>                                                                       \
_BOP_TYPE(field<A>,op,B) operator op(field<A>&& x, const B& y) {                                        \
    return details::mod_hood([](const A& a, const to_local<B>& b) { return std::move(a) op b; }, x, y); \
}                                                                                                       \
template <typename A, typename B>                                                                       \
common::ifn_class_template<field, A, _BOP_TYPE(A,op,field<B>)>                                          \
operator op(const A& x, const field<B>& y) {                                                            \
    return details::map_hood([](const to_local<A>& a, const B& b) { return a op b; }, x, y);            \
}                                                                                                       \

/**
 * @brief Overloads composite assignment operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_IOP(op)                                                                                    \
template <typename A, typename B>                                                                       \
field<A>& operator op##=(field<A>& x, const B& y) {                                                     \
    return details::mod_hood([](const A& a, const to_local<B>& b) { return std::move(a) op b; }, x, y); \
}


_DEF_UOP(+)
_DEF_UOP(-)
_DEF_UOP(~)
_DEF_UOP(!)

_DEF_BOP(+)
_DEF_BOP(-)
_DEF_BOP(*)
_DEF_BOP(/)
_DEF_BOP(%)
_DEF_BOP(^)
_DEF_BOP(&)
_DEF_BOP(|)
_DEF_BOP(<)
_DEF_BOP(>)
_DEF_BOP(<=)
_DEF_BOP(>=)
_DEF_BOP(==)
_DEF_BOP(!=)
_DEF_BOP(&&)
_DEF_BOP(||)
_DEF_BOP(<<)
_DEF_BOP(>>)

_DEF_IOP(+)
_DEF_IOP(-)
_DEF_IOP(*)
_DEF_IOP(/)
_DEF_IOP(%)
_DEF_IOP(^)
_DEF_IOP(&)
_DEF_IOP(|)
_DEF_IOP(>>)
_DEF_IOP(<<)

#undef _BOP_TYPE
#undef _DEF_UOP
#undef _DEF_BOP
#undef _DEF_IOP


}

#endif // FCPP_DATA_FIELD_H_
