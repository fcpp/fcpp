// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file field.hpp
 * @brief Implementation and helper functions for the `field<T>` class template for neighboring fields.
 */

#ifndef FCPP_DATA_FIELD_H_
#define FCPP_DATA_FIELD_H_

#include <cassert>

#include <algorithm>
#include <vector>

#include "lib/settings.hpp"
#include "lib/common/serialize.hpp"
#include "lib/data/tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @cond INTERNAL
template <typename T> class field;
//! @endcond


/**
 * @name field traits
 *
 * Type operators convenient for field manipulation.
 */
//! @{
//! @brief Corresponds to T only if A is a local type.
template <typename A, typename T = void>
using if_local = common::ifn_template<field, A, T>;
//! @brief Corresponds to T only if A is a field type.
template <typename A, typename T = void>
using if_field = common::if_template<field, A, T>;
//! @brief Type returned upon field collapsing.
template <typename A>
using to_local = common::extract_template<field, A>;
//! @brief Type returned upon field construction.
template <typename A>
using to_field = field<common::extract_template<field, A>>;
//! @brief Computes the result type of applying F pointwise to local versions of A.
template <typename F, typename... A>
using local_result = std::result_of_t<F(to_local<A const&>...)>;
//! @brief Computes the result type of applying F pointwise to local versions of A.
template <typename F, typename... A>
using field_result = field<local_result<F, A...>>;
//! @}


//! @cond INTERNAL
//! @brief Forward declarations for enabling friendships.
namespace details {
    template <typename T, typename>
    class field_iterator;

    template <bool b>
    struct field_base {};

    template <typename A>
    field<A> make_field(std::vector<device_t>&&, std::vector<A>&&);

    template <typename A>
    std::vector<device_t>& get_ids(field<A>&);
    template <typename A>
    std::vector<device_t> get_ids(field<A>&&);
    template <typename A>
    std::vector<device_t> const& get_ids(field<A> const&);

    template <typename A>
    std::vector<A>& get_vals(field<A>&);
    template <typename A>
    std::vector<A> get_vals(field<A>&&);
    template <typename A>
    std::vector<A> const& get_vals(field<A> const&);

    template <typename A>
    if_local<A, to_local<A&&>> other(A&&);
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> other(A&&);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> other(A&&);

    template <typename A>
    if_local<A, to_local<A&&>> self(A&&, device_t);
    template <typename A, typename = common::if_class_template<field, A>>
    to_local<A&&> self(A&&, device_t);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    to_local<A&&> self(A&&, device_t);

    template <typename A, typename = if_local<A>>
    inline A align(A&&, std::vector<device_t> const&);
    template <typename A>
    field<A>& align(field<A>&, std::vector<device_t> const&);
    template <typename A>
    field<A> align(field<A>&&, std::vector<device_t> const&);
    template <typename A>
    field<A> align(field<A> const&, std::vector<device_t> const&);
    template <typename A, typename = if_field<A>, typename = common::if_class_template<tuple, A>>
    decltype(auto) align(A&&, std::vector<device_t> const&);

    template <typename A>
    field<A>& align_inplace(field<A>&, std::vector<device_t>&&);
    template <typename... A>
    tuple<A...>& align_inplace(tuple<A...>&, std::vector<device_t>&&);
}
//! @endcond


/**
 * @brief Class representing a neighboring field of T values.
 */
template <typename T>
class field : public details::field_base<std::is_same<T, bool>::value> {
    static_assert(not common::has_template<field, T>, "cannot instantiate a field of fields");

    //! @cond INTERNAL
    //! @brief Class friendships
    //! @{
    template <typename A>
    friend class field;
    template <bool b>
    friend struct details::field_base;

    //! @brief Function friendships
    //! @{
    template <typename A>
    friend field<A> details::make_field(std::vector<device_t>&&, std::vector<A>&&);

    template <typename A>
    friend std::vector<device_t>& details::get_ids(field<A>&);
    template <typename A>
    friend std::vector<device_t> details::get_ids(field<A>&&);
    template <typename A>
    friend std::vector<device_t> const& details::get_ids(field<A> const&);

    template <typename A>
    friend std::vector<A>& details::get_vals(field<A>&);
    template <typename A>
    friend std::vector<A> details::get_vals(field<A>&&);
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
    field(T const& d) {
        m_vals.push_back(d);
    }

    //! @brief Constant field (moving).
    field(T&& d) {
        m_vals.push_back(std::move(d));
    }

    //! @brief Copy constructor.
    field(field const&) = default;

    //! @brief Move constructor.
    field(field&&) = default;

    //! @brief Implicit conversion copy constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field(field<A> const& f) : m_ids(f.m_ids), m_vals{f.m_vals.begin(), f.m_vals.end()} {}

    //! @brief Implicit conversion move constructor.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field(field<A>&& f) : m_ids(std::move(f.m_ids)), m_vals{make_move_iterator(f.m_vals.begin()), make_move_iterator(f.m_vals.end())} {}

    //! @brief Implicit conversion copy constructor from field-like structures.
    template <typename A, typename = std::enable_if_t<std::is_convertible<to_local<A>,T>::value and (not common::is_class_template<fcpp::field,A>) and not std::is_convertible<A,T>::value>>
    field(A const& f) {
        m_vals.push_back(details::other(f));
        for (details::field_iterator<A const, void> it(f); not it.end(); ++it) {
            m_ids.push_back(it.id());
            m_vals.push_back(it.value());
        }
    }
    //! @}

    //! @name assignment operators
    //! @{

    //! @brief Copy assignment.
    field& operator=(field const&) = default;

    //! @brief Move assignment.
    field& operator=(field&&) = default;

    //! @brief Implicit conversion copy assignment.
    template <typename A, typename = std::enable_if_t<std::is_convertible<A,T>::value>>
    field& operator=(field<A> const& f) {
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

    //! @brief Serialises the content from a given input stream.
    common::isstream& serialize(common::isstream& s) {
        device_t size = 0;
        s.read(size);
        m_ids.resize(size);
        m_vals.resize(size+1);
        for (size_t i = 0; i < m_ids.size(); ++i)
            s >> m_ids[i];
        serialize_vals(s, std::is_same<T, bool>{});
        return s;
    }

    //! @brief Serialises the content to a given output stream.
    template <typename S>
    S& serialize(S& s) const {
        s.write((device_t)m_ids.size());
        for (size_t i = 0; i < m_ids.size(); ++i)
            s << m_ids[i];
        serialize_vals(s, std::is_same<T, bool>{});
        return s;
    }

  private:
    //! @brief Serialises vals from an input stream if `T` is not `bool`.
    void serialize_vals(common::isstream& s, std::false_type) {
        for (size_t i = 0; i < m_vals.size(); ++i) s >> m_vals[i];
    }

    //! @brief Serialises vals to an output stream if `T` is not `bool`.
    template <typename S>
    void serialize_vals(S& s, std::false_type) const {
        for (size_t i = 0; i < m_vals.size(); ++i) s << m_vals[i];
    }

    //! @brief Serialises vals from an input stream if `T` is `bool`.
    void serialize_vals(common::isstream& s, std::true_type) {
        char c;
        for (size_t i = 0; i < m_vals.size(); ++i) {
            if (i%8 == 0) s >> c;
            m_vals[i] = (c>>(i%8))&1;
        }
    }

    //! @brief Serialises vals to an output stream if `T` is `bool`.
    template <typename S>
    void serialize_vals(S& s, std::true_type) const {
        char c = 0;
        for (size_t i = 0; i < m_vals.size(); ++i) {
            c += m_vals[i] << (i%8);
            if (i%8 == 7) {
                s << c;
                c = 0;
            }
        }
        if (m_vals.size() % 8 != 0) s << c;
    }

    //! @brief Ordered IDs of exceptions.
    std::vector<device_t> m_ids;

    //! @brief Corresponding values of exceptions (default value in position 0).
    std::vector<T> m_vals;

    //! @brief Member constructor, for internal use only.
    field(std::vector<device_t>&& ids, std::vector<T>&& vals) : m_ids(std::move(ids)), m_vals(std::move(vals)) {}
};


//! @cond INTERNAL
namespace details {
    //! @brief Additional explicit cast to bool for field of bool.
    template <>
    struct field_base<true> {
        explicit operator bool() const {
            field<bool> const& f = *((field<bool> const*)this);
            for (bool x : f.m_vals) if (not x) return false;
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
    std::vector<device_t> get_ids(field<A>&& f) {
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
    std::vector<A> get_vals(field<A>&& f) {
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
    template <typename A>
    if_local<A, to_local<A&&>> other(A&& x) {
        return std::forward<A>(x);
    }

    //! @brief Full access on fields. WARNING: may lead to unexpected results if the argument is not aligned.
    template <typename A, typename>
    to_local<A&&> other(A&& x) {
        return get_vals(std::forward<A>(x))[0];
    }

    //! @brief Full access on indexed structures.
    template <typename A, size_t... is>
    to_local<A&&> other(A&& x, std::index_sequence<is...>) {
        return {other(get<is>(std::forward<A>(x)))...};
    }

    //! @brief Full access on tuples.
    template <typename A, typename, typename>
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
    template <typename A>
    if_local<A, to_local<A&&>> self(A&& x, device_t) {
        return std::forward<A>(x);
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
    template <typename A, typename>
    to_local<A&&> self(A&& x, device_t i) {
        size_t j = std::lower_bound(get_ids(x).begin(), get_ids(x).end(), i) - get_ids(x).begin();
        if (j == get_ids(x).size() or get_ids(x)[j] != i)
            return maybe_emplace(std::forward<A>(x), i, j);
        return get_vals(std::forward<A>(x))[j+1];
    }

    //! @brief Full access on indexed structures.
    template <typename A, size_t... is>
    to_local<A&&> self(A&& x, device_t i, std::index_sequence<is...>) {
        return {self(get<is>(std::forward<A>(x)), i)...};
    }

    //! @brief Full access on tuples.
    template <typename A, typename, typename>
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
    template <typename A, typename>
    inline A align(A&& x, std::vector<device_t> const&) {
        return x;
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
        common::ignore_args(align(get<is>(x), s)...);
        return x;
    }
    template <typename... A, size_t... is>
    tuple<A...> align(tuple<A...>&& x, std::vector<device_t> const& s, std::index_sequence<is...>) {
        common::ignore_args(align(get<is>(x), s)...);
        return x;
    }
    template <typename... A, size_t... is>
    tuple<A...> align(tuple<A...> const& x, std::vector<device_t> const& s, std::index_sequence<is...>) {
        return {align(get<is>(x), s)...};
    }
    template <typename A, typename, typename>
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
            common::ignore_args(get<is>(m_its).emplace(i, std::move(get<is>(v)))...);
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
     * @name align_inplace
     *
     * Changes the domain of a field-like structure to match a given one.
     */
    //! @{
    //! @brief Field case.
    template <typename A>
    field<A>& align_inplace(field<A>& x, std::vector<device_t>&& s) {
        std::vector<A> vals;
        vals.reserve(s.size()+1);
        vals.push_back(other(x));
        field_iterator<field<A> const> it(x);
        for (device_t i : s) {
            while (it.id() < i) ++it;
            vals.push_back(it.value(i));
        }
        get_ids(x) = std::move(s);
        get_vals(x) = std::move(vals);
        return x;
    }
    //! @brief Indexed structures case.
    template <typename A, size_t i, size_t... is>
    A& align_inplace(A& x, std::vector<device_t>&& s, std::index_sequence<i, is...>) {
        common::ignore_args(align_inplace(get<is>(x), std::vector<device_t>{s})...);
        align_inplace(get<i>(x), std::move(s));
        return x;
    }
    //! @brief Tuple case.
    template <typename... A>
    tuple<A...>& align_inplace(tuple<A...>& x, std::vector<device_t>&& s) {
        return align_inplace(x, std::move(s), std::make_index_sequence<sizeof...(A)>{});
    }
    //! @{

    //! @brief Returns a fully aligned field with the default value modified.
    template <typename A, typename B>
    to_field<A> mod_other(A const& x, B const& y, std::vector<device_t>&& s) {
        std::vector<to_local<A>> vals;
        vals.reserve(s.size()+1);
        vals.push_back(other(y));
        field_iterator<A const> it(x);
        for (device_t i : s) {
            while (it.id() < i) ++it;
            vals.push_back(it.value(i));
        }
        return make_field(std::move(s), std::move(vals));
    }

    /**
     * @name mod_self
     *
     * Returns a field with the self value modified.
     */
    //! @{
    //! @brief General case.
    template <typename A, typename B>
    to_field<A> mod_self(A const& x, B const& y, device_t i) {
        std::vector<device_t> ids;
        std::vector<to_local<A>> vals;
        vals.push_back(other(x));
        field_iterator<A const> it(x);
        for (; it.id() < i; ++it) {
            ids.push_back(it.id());
            vals.push_back(it.value());
        }
        ids.push_back(i);
        vals.push_back(self(y, i));
        if (it.id() == i) ++it;
        for (; not it.end(); ++it) {
            ids.push_back(it.id());
            vals.push_back(it.value());
        }
        return make_field(std::move(ids), std::move(vals));
    }
    //! @brief Optimisation for a movable field argument.
    template <typename A, typename B>
    field<A> mod_self(field<A>&& x, B const& y, device_t i) {
        self(x, i) = self(y, i);
        return std::move(x);
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
    if_local<A, local_result<F,A const&,A const&>>
    fold_hood(F&& op, A const& x, std::vector<device_t> const& dom) {
        assert(dom.size() > 0);
        size_t n = dom.size();
        local_result<F,A const&,A const&> res = x;
        for (--n; n>0; --n) res = op(x, res);
        return res;
    }
    //! @brief Inclusive folding.
    template <typename F, typename A>
    if_field<A, local_result<F,A const&,A const&>>
    fold_hood(F&& op, A const& f, std::vector<device_t> const& dom) {
        assert(dom.size() > 0);
        field_iterator<A const> it(f);
        while (it.id() < dom[0]) ++it;
        local_result<F,A const&,A const&> res = it.value(dom[0]);
        for (size_t k=1; k<dom.size(); ++k) {
            while (it.id() < dom[k]) ++it;
            res = op(it.value(dom[k]), res);
        }
        return res;
    }
    //! @brief Inclusive folding with ids.
    template <typename F, typename A>
    if_field<A, local_result<F,device_t,A const&,A const&>>
    fold_hood(F&& op, A const& f, std::vector<device_t> const& dom) {
        assert(dom.size() > 0);
        field_iterator<A const> it(f);
        while (it.id() < dom[0]) ++it;
        local_result<F,device_t,A const&,A const&> res = it.value(dom[0]);
        for (size_t k=1; k<dom.size(); ++k) {
            while (it.id() < dom[k]) ++it;
            res = op(dom[k], it.value(dom[k]), res);
        }
        return res;
    }
    //! @brief Exclusive folding (optimization for locals).
    template <typename F, typename A, typename B>
    if_local<A, local_result<F,A const&,B const&>>
    fold_hood(F&& op, A const& x, B const& b, std::vector<device_t> const& dom, device_t i) {
        assert(std::binary_search(dom.begin(), dom.end(), i));
        local_result<F,A const&,B const&> res = details::self(b, i);
        for (size_t n = dom.size(); n>1; --n) res = op(x, res);
        return res;
     }
    //! @brief Exclusive folding.
    template <typename F, typename A, typename B>
    if_field<A, local_result<F,A const&,B const&>>
    fold_hood(F&& op, A const& f, B const& b, std::vector<device_t> const& dom, device_t i) {
        assert(std::binary_search(dom.begin(), dom.end(), i));
        local_result<F,A const&,B const&> res = self(b, i);
        field_iterator<A const> it(f);
        for (size_t k=0; k<dom.size(); ++k) if (dom[k] != i) {
            while (it.id() < dom[k]) ++it;
            res = op(it.value(dom[k]), res);
        }
        return res;
    }
    //! @brief Exclusive folding with ids.
    template <typename F, typename A, typename B>
    if_field<A, local_result<F,device_t,A const&,B const&>>
    fold_hood(F&& op, A const& f, B const& b, std::vector<device_t> const& dom, device_t i) {
        assert(std::binary_search(dom.begin(), dom.end(), i));
        local_result<F,device_t,A const&,B const&> res = self(b, i);
        field_iterator<A const> it(f);
        for (size_t k=0; k<dom.size(); ++k) if (dom[k] != i) {
            while (it.id() < dom[k]) ++it;
            res = op(dom[k], it.value(dom[k]), res);
        }
        return res;
    }
    //! @}
}
//! @endcond


/**
 * @name map_hood
 *
 * Applies an operator pointwise on a sequence of fields.
 */
//! @{
//! @brief General case with some field argument.
template <typename F, typename... A, typename = if_field<tuple<A...>>>
field_result<F,A...> map_hood(F&& op, A const&... a) {
    field_result<F,A...> r(op(details::other(a)...));
    for (details::field_iterator<tuple<A...> const> it(a...); not it.end(); ++it) {
        details::get_ids(r).push_back(it.id());
        details::get_vals(r).push_back(it.apply(op));
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
    details::get_ids(r) = details::get_ids(std::move(f));
    details::get_vals(r).resize(details::get_vals(f).size());
    for (size_t i = 0; i < details::get_vals(f).size(); ++i)
            details::get_vals(r)[i] = op(details::get_vals(std::move(f))[i], l...);
    return r;
}
//! @brief Optimisation for a single immutable field argument in first position.
template <typename F, typename T, typename... L, typename = if_local<tuple<L...>>>
field_result<F,field<T>,L...> map_hood(F&& op, field<T> const& f, L&&... l) {
    field_result<F,field<T>,L...> r;
    details::get_ids(r) = details::get_ids(f);
    details::get_vals(r).resize(details::get_vals(f).size());
    for (size_t i = 0; i < details::get_vals(f).size(); ++i)
            details::get_vals(r)[i] = op(details::get_vals(f)[i], l...);
    return r;
}
//! @brief Optimisation for a single movable field argument in second position.
template <typename F, typename A, typename T, typename... L, typename = if_local<tuple<A,L...>>>
field_result<F,A,field<T>,L...> map_hood(F&& op, A&& a, field<T>&& f, L&&... l) {
    field_result<F,A,field<T>,L...> r;
    details::get_ids(r) = details::get_ids(std::move(f));
    details::get_vals(r).resize(details::get_vals(f).size());
    for (size_t i = 0; i < details::get_vals(f).size(); ++i)
            details::get_vals(r)[i] = op(a, details::get_vals(std::move(f))[i], l...);
    return r;
}
//! @brief Optimisation for a single immutable field argument in second position.
template <typename F, typename A, typename T, typename... L, typename = if_local<tuple<A,L...>>>
field_result<F,A,field<T>,L...> map_hood(F&& op, A&& a, field<T> const& f, L&&... l) {
    field_result<F,A,field<T>,L...> r;
    details::get_ids(r) = details::get_ids(f);
    details::get_vals(r).resize(details::get_vals(f).size());
    for (size_t i = 0; i < details::get_vals(f).size(); ++i)
            details::get_vals(r)[i] = op(a, details::get_vals(f)[i], l...);
    return r;
}
//! @brief Optimisation for a single movable field argument in third position.
template <typename F, typename A, typename B, typename T, typename... L, typename = if_local<tuple<A,B,L...>>>
field_result<F,A,B,field<T>,L...> map_hood(F&& op, A&& a, B&& b, field<T>&& f, L&&... l) {
    field_result<F,A,B,field<T>,L...> r;
    details::get_ids(r) = details::get_ids(std::move(f));
    details::get_vals(r).resize(details::get_vals(f).size());
    for (size_t i = 0; i < details::get_vals(f).size(); ++i)
            details::get_vals(r)[i] = op(a, b, details::get_vals(std::move(f))[i], l...);
    return r;
}
//! @brief Optimisation for a single immutable field argument in third position.
template <typename F, typename A, typename B, typename T, typename... L, typename = if_local<tuple<A,B,L...>>>
field_result<F,A,B,field<T>,L...> map_hood(F&& op, A&& a, B&& b, field<T> const& f, L&&... l) {
    field_result<F,A,B,field<T>,L...> r;
    details::get_ids(r) = details::get_ids(f);
    details::get_vals(r).resize(details::get_vals(f).size());
    for (size_t i = 0; i < details::get_vals(f).size(); ++i)
            details::get_vals(r)[i] = op(a, b, details::get_vals(f)[i], l...);
    return r;
}
//! @brief Optimisation for two field arguments in starting position.
template <typename F, typename T, typename U, typename... L, typename = if_local<tuple<L...>>>
field_result<F,field<T>,field<U>,L...> map_hood(F&& op, field<T> const& f, field<U> const& g, L&&... l) {
    field_result<F,field<T>,field<U>,L...> r;
    details::get_ids(r).reserve(details::get_ids(f).size() + details::get_ids(g).size());
    details::get_vals(r).reserve(details::get_ids(f).size() + details::get_ids(g).size() + 1);
    details::get_vals(r).push_back(op(details::get_vals(f)[0], details::get_vals(g)[0], l...));
    size_t i = 0, j = 0;
    while (i < details::get_ids(f).size() or j < details::get_ids(g).size()) {
        if (i == details::get_ids(f).size()) {
            details::get_ids(r).push_back(details::get_ids(g)[j]);
            details::get_vals(r).push_back(op(details::get_vals(f)[0], details::get_vals(g)[++j], l...));
        } else if (j == details::get_ids(g).size() or details::get_ids(f)[i] < details::get_ids(g)[j]) {
            details::get_ids(r).push_back(details::get_ids(f)[i]);
            details::get_vals(r).push_back(op(details::get_vals(f)[++i], details::get_vals(g)[0], l...));
        } else if (details::get_ids(f)[i] > details::get_ids(g)[j]) {
            details::get_ids(r).push_back(details::get_ids(g)[j]);
            details::get_vals(r).push_back(op(details::get_vals(f)[0], details::get_vals(g)[++j], l...));
        } else {
            details::get_ids(r).push_back(details::get_ids(f)[i]);
            details::get_vals(r).push_back(op(details::get_vals(f)[++i], details::get_vals(g)[++j], l...));
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
    for (details::field_iterator<tuple<A,L const...>> it(a,l...); not it.end(); ++it)
        get<0>(it).emplace(it.id(), it.apply(op));
    details::other(a) = op(details::other(a), details::other(l)...);
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
    for (typename std::vector<A>::reference x : details::get_vals(a)) x = op(x, l...);
    return a;
}
//! @}


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
field<A> operator op(field<A> const& x) {                                           \
    return map_hood([] (A const& a) {return op a;}, x);                             \
}                                                                                   \
template <typename A>                                                               \
field<A> operator op(field<A>&& x) {                                                \
    mod_hood([] (A const& a) {return op std::move(a);}, x);                         \
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
_BOP_TYPE(field<A>,op,B) operator op(field<A> const& x, B const& y) {                                   \
    return map_hood([](A const& a, to_local<B> const& b) { return a op b; }, x, y);                     \
}                                                                                                       \
template <typename A, typename B>                                                                       \
std::enable_if_t<std::is_same<_BOP_TYPE(field<A>,op,B), field<A>>::value, field<A>>                     \
operator op(field<A>&& x, B const& y) {                                                                 \
    return mod_hood([](A const& a, to_local<B> const& b) { return std::move(a) op b; }, x, y);          \
}                                                                                                       \
template <typename A, typename B>                                                                       \
std::enable_if_t<not std::is_same<_BOP_TYPE(field<A>,op,B), field<A>>::value, _BOP_TYPE(field<A>,op,B)> \
operator op(field<A>&& x, B const& y) {                                                                 \
    return map_hood([](A const& a, to_local<B> const& b) { return a op b; }, x, y);                     \
}                                                                                                       \
template <typename A, typename B>                                                                       \
common::ifn_class_template<field, A, _BOP_TYPE(A,op,field<B>)>                                          \
operator op(A const& x, field<B> const& y) {                                                            \
    return map_hood([](to_local<A> const& a, B const& b) { return a op b; }, x, y);                     \
}                                                                                                       \

/**
 * @brief Overloads composite assignment operators for fields.
 *
 * Used to overload every operator available for the base type.
 * Macro not available outside of the scope of this file.
 */
#define _DEF_IOP(op)                                                                                    \
template <typename A, typename B>                                                                       \
field<A>& operator op##=(field<A>& x, B const& y) {                                                     \
    return mod_hood([](A const& a, to_local<B> const& b) { return std::move(a) op b; }, x, y);          \
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

//! @cond INTERNAL
template <typename A, typename B>
_BOP_TYPE(field<A>,<<,B) operator<<(field<A> const& x, B const& y) {
    return map_hood([](A const& a, to_local<B> const& b) { return a << b; }, x, y);
}
template <typename A, typename B>
_BOP_TYPE(field<A>,<<,B) operator<<(field<A>&& x, B const& y) {
    return mod_hood([](A const& a, to_local<B> const& b) { return std::move(a) << b; }, x, y);
}
template <typename A, typename B>
std::enable_if_t<
    not common::is_class_template<field, A> and not fcpp::common::is_ostream<A>::value,
    _BOP_TYPE(A,<<,field<B>)
>
operator<<(A const& x, field<B> const& y) {
    return map_hood([](to_local<A> const& a, B const& b) { return a << b; }, x, y);
}
//! @endcond

#undef _BOP_TYPE
#undef _DEF_UOP
#undef _DEF_BOP
#undef _DEF_IOP


}

#endif // FCPP_DATA_FIELD_H_
