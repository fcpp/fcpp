// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file distribution.hpp
 * @brief Collection of random distributions. Similar to distributions in `<random>`, but with distribution parameters as template arguments, which are also made uniform (mean and deviation) whenever possible.
 */

#ifndef FCPP_OPTION_DISTRIBUTION_H_
#define FCPP_OPTION_DISTRIBUTION_H_

#include <cassert>
#include <cmath>
#include <limits>
#include <random>
#include <type_traits>
#include <utility>

#include "lib/common/tagged_tuple.hpp"
#include "lib/data/vec.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Lightweight interface for C random generators to C++ distributions.
struct crand {
    //! @brief The data produced by the generator.
    using result_type = uint16_t;

    //! @brief Default constructor.
    crand() {}

    //! @brief Constructor with seed.
    crand(result_type val) {
        srand(val);
    }

    //! @brief The minimum generated value.
    static constexpr result_type min() {
        return 0;
    }

    //! @brief The maximum generated value.
    static constexpr result_type max() {
        return 65535;
    }

    //! @brief Feeds a given seed.
    inline void seed(result_type val = 0) {
        srand(val);
    }

    //! @brief Generates a new element.
    inline result_type operator()() {
        return rand() & 65535;
    }

    //! @brief Discards bits of random data.
    inline void discard(unsigned long long) {}

    //! @brief Equality operator.
    inline bool operator==(crand const&) {
        return true;
    }

    //! @brief Inequality operator.
    inline bool operator!=(crand const&) {
        return false;
    }
};



//! @brief Namespace containing classes for random data generation.
namespace distribution {


//! @cond INTERNAL
namespace details {
    template <typename T>
    std::uniform_real_distribution<T> make(common::type_sequence<std::uniform_real_distribution<T>>, T mean, T dev) {
        return std::uniform_real_distribution<T>(mean - 1.7320508075688772*dev, mean + 1.7320508075688772*dev);
    }

    template <typename T>
    std::normal_distribution<T> make(common::type_sequence<std::normal_distribution<T>>, T mean, T dev) {
        return std::normal_distribution<T>(mean, dev);
    }

    template <typename T>
    std::exponential_distribution<T> make(common::type_sequence<std::exponential_distribution<T>>, T mean, T dev) {
        assert(mean == dev);
        return std::exponential_distribution<T>(1/mean);
    }

    template <typename T>
    std::weibull_distribution<T> make(common::type_sequence<std::weibull_distribution<T>>, T mean, T dev) {
        long double k = 0;
        if (dev > 0) {
            long double t = logl((dev * (long double)dev) / (mean * mean) + 1);
            long double kmin = 0, kmax = 1;
            while (lgammal(1 + 2 * kmax) - 2 * lgammal(1 + kmax) < t) {
                kmin = kmax;
                kmax *= 2;
            }
            k = (kmin + kmax) / 2;
            while (kmin < k && k < kmax) {
                if (lgammal(1 + 2 * k) - 2 * lgammal(1 + k) < t)
                    kmin = k;
                else
                    kmax = k;
                k = (kmin + kmax) / 2;
            }
        }
        long double shape = 1 / k;
        long double scale = mean / tgammal(1 + k);
        return std::weibull_distribution<T>(shape, scale);
    }
}
//! @endcond


/**
 * Function uniformily creating instances of real distributions in `<random>` based on mean and deviation.
 *
 * @tparam D   A distribution template in `<random>`.
 * @param mean The required mean of the distribution.
 * @param dev  The standard deviation of the distribution.
 */
template <template<class> class D, typename T>
D<T> make(T mean, T dev) {
    return details::make(common::type_sequence<D<T>>(), mean, dev);
}


//! @cond INTERNAL
namespace details {
    template <typename R, typename G, typename S, typename T>
    typename R::type call_distr(G&& g, common::tagged_tuple<S,T> const& t) {
        R dist{g, t};
        return dist(g, t);
    }

    constexpr intmax_t maybe_divide(intmax_t num, common::number_sequence<1>) {
        return num;
    }

    constexpr long double maybe_divide(intmax_t num, common::number_sequence<0>) {
        return num * std::numeric_limits<long double>::infinity();
    }

    template <intmax_t den, typename = std::enable_if_t<(den > 1)>>
    constexpr long double maybe_divide(intmax_t num, common::number_sequence<den>) {
        return num / (long double)den;
    }
}
//! @endcond


/**
 * @brief Macro defining a constant distribution for non-numeric types.
 * To be used as `CONSTANT_DISTRIBUTION(hello_distr, string, "hello");`.
 *
 * @param name The name of the distribution
 * @param R The result type.
 * @param val The constant value.
 */
#define CONSTANT_DISTRIBUTION(name, R, val) \
struct name {                               \
    using type = R;                         \
    template <typename G, typename T>       \
    name(G&&, T&&) {}                       \
    template <typename G, typename T>       \
    type operator()(G&&, T&&) {             \
        return val;                         \
    }                                       \
}


//! @name Constant distribution.
//! @{
/**
 * @brief With value as distribution.
 * @tparam D The value (as distribution).
 */
template <typename D>
struct constant {
    //! @brief Result type.
    using type = typename D::type;

    //! @brief Constructor.
    template <typename G, typename S, typename T>
    constant(G&& g, common::tagged_tuple<S,T> const& t) : val(details::call_distr<D>(g, t)) {}

    //! @brief Generator function.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        return val;
    }

  private:
    //! @brief The constant value.
    type val;
};
/**
 * @brief With value as numeric template parameter.
 * @tparam R The result type.
 * @tparam num The (integral) numerator of the value.
 * @tparam den The (integral) denominator of the value.
 * @tparam val_tag The tag corresponding to the value in initialisation values.
 */
template <typename R, intmax_t num, intmax_t den = 1, typename val_tag = void>
struct constant_n {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename S, typename T>
    constant_n(G&&, common::tagged_tuple<S,T> const& t) {
        constexpr auto def = details::maybe_divide(num, common::number_sequence<den>{});
        val = type(common::get_or<val_tag>(t, def));
    }

    //! @brief Generator function.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        return val;
    }

  private:
    //! @brief The constant value.
    type val;
};
//! @cond INTERNAL
//! @brief With value as numeric template parameter (optimisation for no given tag).
template <typename R, intmax_t num, intmax_t den>
struct constant_n<R, num, den, void> {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename T>
    constant_n(G&&, T&&) {}

    //! @brief Generator function.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        constexpr auto def = details::maybe_divide(num, common::number_sequence<den>{});
        return type(def);
    }
};
//! @endcond
/**
 * @brief With value at initialisation.
 * @tparam R The result type.
 * @tparam val_tag The tag corresponding to the value in initialisation values.
 */
template <typename R, typename val_tag>
struct constant_i {
    //! @brief Result type.
    using type = R;

    //! @brief Constructor.
    template <typename G, typename S, typename T>
    constant_i(G&&, common::tagged_tuple<S,T> const& t) : val(common::get_or<val_tag>(t, type())) {}

    //! @brief Generator function.
    template <typename G, typename T>
    type operator()(G&&, T&&) {
        return val;
    }

  private:
    //! @brief The constant value.
    type val;
};
//! @}


//! @name Variable distribution, varying the distribution parameters at every call.
//! @{
/**
 * @brief With value as distribution.
 * @tparam D The value (as distribution).
 */
template <typename D>
struct variable {
    //! @brief Result type.
    using type = typename D::type;

    //! @brief Constructor.
    template <typename G, typename T>
    variable(G&&, T&&) {}

    //! @brief Generator function.
    template <typename G, typename S, typename T>
    type operator()(G&& g, common::tagged_tuple<S,T> const& t) {
        return details::call_distr<D>(g, t);
    }
};
/**
 * @brief With value at initialisation.
 * @tparam R The result type.
 * @tparam val_tag The tag corresponding to the value in initialisation values.
 */
template <typename R, typename val_tag>
using variable_i = variable<constant_i<R, val_tag>>;
//! @}


/**
 * @brief Standard real distribution.
 * @tparam std_dist The standard distribution in the `<random>` header.
 * @tparam mean     The mean of the distribution (as distribution).
 * @tparam dev      The standard deviation of the distribution (as distribution).
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <template<class> class std_dist, typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class standard {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");

  public:
    //! @brief Result type.
    using type = typename mean::type;

    //! @brief Constructor.
    template <typename G, typename S, typename T>
    standard(G&& g, common::tagged_tuple<S,T> const& t) : m_d(make<std_dist>((type)common::get_or<mean_tag>(t,details::call_distr<mean>(g, t)), (type)common::get_or<dev_tag>(t,details::call_distr<dev>(g, t)))) {}

    //! @brief Generator function.
    template <typename G, typename T>
    type operator()(G&& g, T&&) {
        return m_d(g);
    }

  private:
    //! @brief The distribution.
    std_dist<type> m_d;
};


//! @name Uniform real distribution.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @tparam mean     The mean of the distribution (as distribution).
 * @tparam dev      The standard deviation of the distribution (as distribution).
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
using uniform = standard<std::uniform_real_distribution, mean, dev, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as numeric template parameters.
 * @tparam T        The type returned by the distribution.
 * @tparam mean     The (integral) mean of the distribution.
 * @tparam dev      The (integral) standard deviation of the distribution.
 * @tparam scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using uniform_n = uniform<constant_n<T, mean, scale>, constant_n<T, dev, scale>, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @tparam T        The type returned by the distribution.
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, typename mean_tag, typename dev_tag>
using uniform_i = uniform<constant_n<T, 0>, constant_n<T, 0>, mean_tag, dev_tag>;
//! @}


//! @name Uniform real distribution set up through extremes instead of mean and deviation.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @tparam min     The minimum of the distribution (as distribution).
 * @tparam max     The maximum of the distribution (as distribution).
 * @tparam min_tag The tag corresponding to the minimum in initialisation values.
 * @tparam max_tag The tag corresponding to the maximum in initialisation values.
 */
template <typename min, typename max, typename min_tag = void, typename max_tag = void>
class interval {
    static_assert(std::is_same<typename min::type, typename max::type>::value, "min and max of different type");

  public:
    //! @brief Result type.
    using type = typename min::type;

    //! @brief Constructor.
    template <typename G, typename S, typename T>
    interval(G&& g, common::tagged_tuple<S,T> const& t) : m_d{common::get_or<min_tag>(t,details::call_distr<min>(g, t)), common::get_or<max_tag>(t,details::call_distr<max>(g, t))} {}

    //! @brief Generator function.
    template <typename G, typename T>
    type operator()(G&& g, T&&) {
        return m_d(g);
    }

  private:
    //! @brief The distribution.
    std::uniform_real_distribution<type> m_d;
};
/**
 * @brief With mean and deviation as numeric template parameters.
 * @tparam T       The type returned by the distribution.
 * @tparam min     The (integral) minimum of the distribution.
 * @tparam max     The (integral) maximum of the distribution.
 * @tparam scale   An (optional) scale factor by which `min` and `max` are divided.
 * @tparam min_tag The tag corresponding to the minimum in initialisation values.
 * @tparam max_tag The tag corresponding to the maximum in initialisation values.
 */
template <typename T, intmax_t min, intmax_t max, intmax_t scale = 1, typename min_tag = void, typename max_tag = void>
using interval_n = interval<constant_n<T, min, scale>, constant_n<T, max, scale>, min_tag, max_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @tparam T       The type returned by the distribution.
 * @tparam min_tag The tag corresponding to the minimum in initialisation values.
 * @tparam max_tag The tag corresponding to the maximum in initialisation values.
 */
template <typename T, typename min_tag, typename max_tag>
using interval_i = interval<constant_n<T, 0>, constant_n<T, 0>, min_tag, max_tag>;
//! @}


//! @name Normal real distribution.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @tparam mean     The mean of the distribution (as distribution).
 * @tparam dev      The standard deviation of the distribution (as distribution).
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
using normal = standard<std::normal_distribution, mean, dev, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as numeric template parameters.
 * @tparam T        The type returned by the distribution.
 * @tparam mean     The (integral) mean of the distribution.
 * @tparam dev      The (integral) standard deviation of the distribution.
 * @tparam scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using normal_n = normal<constant_n<T, mean, scale>, constant_n<T, dev, scale>, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @tparam T        The type returned by the distribution.
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, typename mean_tag, typename dev_tag>
using normal_i = normal<constant_n<T, 0>, constant_n<T, 0>, mean_tag, dev_tag>;
//! @}


//! @name Exponential real distribution.
//! @{
/**
 * @brief With mean as distribution.
 * @tparam mean     The mean of the distribution (as distribution).
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 */
template <typename mean, typename mean_tag = void>
class exponential {
  public:
    //! @brief Result type.
    using type = typename mean::type;

    //! @brief Constructor.
    template <typename G, typename S, typename T>
    exponential(G&& g, common::tagged_tuple<S,T> const& t) : m_d(1/common::get_or<mean_tag>(t, details::call_distr<mean>(g, t))) {}

    //! @brief Generator function.
    template <typename G, typename T>
    type operator()(G&& g, T&&) {
        return m_d(g);
    }

  private:
    //! @brief The distribution.
    std::exponential_distribution<type> m_d;
};
/**
 * @brief With mean as numeric template parameter.
 * @tparam T        The type returned by the distribution.
 * @tparam mean     The (integral) mean of the distribution.
 * @tparam scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t scale = 1, typename mean_tag = void>
using exponential_n = exponential<constant_n<T, mean, scale>, mean_tag>;
/**
 * @brief With mean as initialisation value.
 * @tparam T        The type returned by the distribution.
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 */
template <typename T, typename mean_tag>
using exponential_i = exponential<constant_n<T, 0>, mean_tag>;
//! @}


//! @name Weibull real distribution.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @tparam mean     The mean of the distribution (as distribution).
 * @tparam dev      The standard deviation of the distribution (as distribution).
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
using weibull = standard<std::weibull_distribution, mean, dev, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as numeric template parameters.
 * @tparam T        The type returned by the distribution.
 * @tparam mean     The (integral) mean of the distribution.
 * @tparam dev      The (integral) standard deviation of the distribution.
 * @tparam scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using weibull_n = weibull<constant_n<T, mean, scale>, constant_n<T, dev, scale>, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @tparam T        The type returned by the distribution.
 * @tparam mean_tag The tag corresponding to the mean in initialisation values.
 * @tparam dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, typename mean_tag, typename dev_tag>
using weibull_i = weibull<constant_n<T, 0>, constant_n<T, 0>, mean_tag, dev_tag>;
//! @}


/**
 * @brief Modifies a real distribution to be positive.
 *
 * Assumes that the probability of generating positive numbers is high.
 *
 * @tparam D A real distribution.
 */
template <typename D>
struct positive : public D {
    //! @brief Result type.
    using type = typename D::type;

    //! @brief Constructor.
    using D::D;

    //! @brief Generator function.
    template <typename G, typename S, typename T>
    type operator()(G&& g, common::tagged_tuple<S,T> const& t) {
        type x = D::operator()(g, t);
        return (x >= 0) ? x : operator()(g, t);
    }
};


//! @name Generates points given coordinates.
//! @{
/**
 * @brief With coordinates as distributions.
 *
 * All `Ds::type` must be convertible to `real_t`.
 *
 * @tparam Ds The coordinates (as distributions).
 */
template <typename... Ds>
class point {
  public:
    //! @brief Result type.
    using type = vec<sizeof...(Ds)>;

    //! @brief Constructor.
    template <typename G, typename S, typename T>
    point(G&& g, common::tagged_tuple<S,T> const& t) : m_distributions{Ds{g,t}...} {}

    //! @brief Generator function.
    template <typename G, typename S, typename T>
    type operator()(G&& g, common::tagged_tuple<S,T> const& t) {
        return call_impl(g, t, std::make_index_sequence<sizeof...(Ds)>{});
    }

  private:
    //! @brief Helper generator function.
    template <typename G, typename S, typename T, size_t... i>
    type call_impl(G&& g, common::tagged_tuple<S,T> const& t, std::index_sequence<i...>) {
        return {std::get<i>(m_distributions)(g, t)...};
    }

    //! @brief The distributions.
    std::tuple<Ds...> m_distributions;
};
/**
 * @brief With coordinates as numeric template parameters.
 * @tparam scale A scale factor by which the coordinates are divided.
 * @tparam x The (integral) coordinates.
 */
template <intmax_t scale, intmax_t... x>
using point_n = point<constant_n<real_t, x, scale>...>;
/**
 * @brief With coordinates as initialisation values.
 * @tparam x_tag The tags corresponding to coordinates in initialisation values.
 */
template <typename... x_tag>
using point_i = point<constant_i<real_t, x_tag>...>;
//! @}


//! @name Generates points in a rectangle given its extremes.
//! @{
//! @cond INTERNAL
namespace details {
    template <typename L, typename U>
    struct rect;
    template <typename... Ls, typename... Us>
    struct rect<common::type_sequence<Ls...>, common::type_sequence<Us...>> {
        using type = point<interval<Ls,Us>...>;
    };
}
//! @endcond
/**
 * @brief With extremes as distributions.
 *
 * All `Ds::type` must be convertible to `real_t`.
 *
 * @tparam Ds The extremes (as distributions).
 */
template <typename... Ds>
using rect  = typename details::rect<
    common::type_slice<0,              sizeof...(Ds)/2,1,Ds...>,
    common::type_slice<sizeof...(Ds)/2,sizeof...(Ds),  1,Ds...>
>::type;
/**
 * @brief With extremes as numeric template parameters.
 * @tparam scale A scale factor by which the extremes are divided.
 * @tparam x The (integral) extremes.
 */
template <intmax_t scale, intmax_t... x>
using rect_n = rect<constant_n<real_t, x, scale>...>;
/**
 * @brief With extremes as initialisation values.
 * @tparam x_tag The tags corresponding to extremes in initialisation values.
 */
template <typename... x_tag>
using rect_i = rect<constant_i<real_t, x_tag>...>;
//! @}


}


}

#endif // FCPP_OPTION_DISTRIBUTION_H_
