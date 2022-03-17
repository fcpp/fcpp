// Copyright © 2022 Giorgio Audrito. All Rights Reserved.

/**
 * @file functor.hpp
 * @brief Composable functor classes on tagged tuples or distributions.
 */

#ifndef FCPP_OPTION_FUNCTOR_H_
#define FCPP_OPTION_FUNCTOR_H_

#include <cmath>

#include <type_traits>

#include "lib/settings.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


/**
 * @brief Namespace containing tagged tuple functors.
 */
namespace functor {


//! @brief Functor casting a value.
template <typename A, typename R = real_t>
struct cast {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    cast(G&& g, T const& t) : m_a(g, t) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& t) {
        return (type)m_a(g, t);
    }

  private:
    //! @brief The value.
    A m_a;
};


//! @brief General tuple accessor.
template <typename A, typename R = real_t>
struct get {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    get(G&&, T const&) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&&, T const& row) {
        return common::get<A>(row);
    }
};


//! @cond INTERNAL
namespace details {
    //! @brief Polymorphic tuple accessor.
    template <typename A>
    struct accessor {
        //! @brief Constructor.
        template <typename G, typename T>
        accessor(G&&, T const&) {}

        //! @brief Functor computation.
        template <typename G, typename T>
        auto operator()(G&&, T const& row) {
            return common::get<A>(row);
        }
    };

    //! @brief Checks whether a class has a non-void type member.
    template<typename C>
    struct has_tag_member {
      private:
        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<typename T::tag, void>::type;

        template <typename>
        static constexpr std::true_type check(...);

        typedef decltype(check<C>(0)) type;

      public:
        static constexpr bool value = not type::value;
    };

    //! @brief Checks whether a class has a non-void type member.
    template<typename C>
    struct has_type_member {
      private:
        template <typename T>
        static constexpr auto check(T*) -> typename std::is_same<typename T::type, void>::type;

        template <typename>
        static constexpr std::true_type check(...);

        typedef decltype(check<C>(0)) type;

      public:
        static constexpr bool value = not type::value;
    };

    //! @brief Polymorphic tuple accessor or functor/distribution.
    template <typename A>
    using element = std::conditional_t<has_type_member<A>::value and not has_tag_member<A>::value, A, accessor<A>>;
}
//! @endcond


//! @brief Functor adding elements.
template <typename A, typename B, typename R = real_t>
struct add {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    add(G&& g, T const& t) : m_a(g, t), m_b(g, t) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        return m_a(g, row) + m_b(g, row);
    }

  private:
    //! @brief The first element.
    details::element<A> m_a;
    //! @brief The second element.
    details::element<B> m_b;
};


//! @brief Functor subtracting elements.
template <typename A, typename B, typename R = real_t>
struct sub {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    sub(G&& g, T const& t) : m_a(g, t), m_b(g, t) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        return m_a(g, row) - m_b(g, row);
    }

  private:
    //! @brief The first element.
    details::element<A> m_a;
    //! @brief The second element.
    details::element<B> m_b;
};


//! @brief Functor multiplying elements.
template <typename A, typename B, typename R = real_t>
struct mul {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    mul(G&& g, T const& t) : m_a(g, t), m_b(g, t) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        return m_a(g, row) * m_b(g, row);
    }

  private:
    //! @brief The first element.
    details::element<A> m_a;
    //! @brief The second element.
    details::element<B> m_b;
};


//! @brief Functor dividing elements.
template <typename A, typename B, typename R = real_t>
struct div {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    div(G&& g, T const& t) : m_a(g, t), m_b(g, t) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        return m_a(g, row) / m_b(g, row);
    }

  private:
    //! @brief The first element.
    details::element<A> m_a;
    //! @brief The second element.
    details::element<B> m_b;
};


//! @brief Functor elevating elements as powers.
template <typename A, typename B, typename R = real_t>
struct pow {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    pow(G&& g, T const& t) : m_a(g, t), m_b(g, t) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        return std::pow(m_a(g, row), m_b(g, row));
    }

  private:
    //! @brief The first element.
    details::element<A> m_a;
    //! @brief The second element.
    details::element<B> m_b;
};


//! @brief Functor accumulating elements over time.
template <typename A, typename R = real_t>
struct exp {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    exp(G&& g, T const& t) : m_a(g, t) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        return std::exp(m_a(g, row));
    }

  private:
    //! @brief The element.
    details::element<A> m_a;
};


//! @brief Functor accumulating elements over time.
template <typename A, typename R = real_t>
struct log {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    log(G&& g, T const& t) : m_a(g, t) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        return std::log(m_a(g, row));
    }

  private:
    //! @brief The element.
    details::element<A> m_a;
};


//! @brief Functor accumulating elements over time.
template <typename A, typename R = real_t>
struct acc {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    acc(G&& g, T const& t) : m_a(g, t), m_value(0) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        return m_value += m_a(g, row);
    }

  private:
    //! @brief The element.
    details::element<A> m_a;
    //! @brief The accumulate.
    type m_value;
};


//! @brief Functor differentiating elements over time.
template <typename A, typename R = real_t>
struct diff {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    diff(G&& g, T const& t) : m_a(g, t), m_last(0) {}

    //! @brief Functor computation.
    template <typename G, typename T>
    type operator()(G&& g, T const& row) {
        type t = m_last;
        m_last = m_a(g, row);
        return m_last - t;
    }

  private:
    //! @brief The element.
    details::element<A> m_a;
    //! @brief The last value.
    type m_last;
};


}


}

#endif // FCPP_OPTION_FUNCTOR_H_
