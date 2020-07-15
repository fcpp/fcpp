// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file distribution.hpp
 * @brief Collection of random distributions. Similar to distributions in `<random>`, but with distribution parameters as template arguments, which are also made uniform (mean and deviation) whenever possible.
 */

#ifndef FCPP_OPTION_DISTRIBUTION_H_
#define FCPP_OPTION_DISTRIBUTION_H_

#include <cassert>
#include <cmath>
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
    using result_type = int;
    
    crand() {}
    
    crand(result_type val) {
        srand(val);
    }
    
    static constexpr result_type min() {
        return 0;
    }
    
    static constexpr result_type max() {
        return RAND_MAX;
    }
    
    void seed(result_type val = 0) {
        srand(val);
    }
    
    result_type operator()() {
        return rand();
    }
    
    void discard (unsigned long long z) {
        for (unsigned long long i=0; i<z; ++i) rand();
    }

    bool operator==(const crand&) {
        return true;
    }

    bool operator!=(const crand&) {
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
        T t = log((dev * dev) / (mean * mean) + 1);
        T kmin = 0, kmax = 1;
        while (lgamma(1 + 2 * kmax) - 2 * lgamma(1 + kmax) < t) {
            kmin = kmax;
            kmax *= 2;
        }
        double k = (kmin + kmax) / 2;
        while (kmin < k && k < kmax) {
            if (lgamma(1 + 2 * k) - 2 * lgamma(1 + k) < t) {
                kmin = k;
            } else {
                kmax = k;
            }
            k = (kmin + kmax) / 2;
        }
        T shape = 1 / k;
        T scale = mean / exp(lgamma(1 + k));
        return std::weibull_distribution<T>(shape, scale);
    }
}
//! @endcond


/**
 * Function uniformily creating instances of real distributions in `<random>` based on mean and deviation.
 *
 * @param D    A distribution template in `<random>`.
 * @param mean The required mean of the distribution.
 * @param dev  The standard deviation of the distribution.
 */
template <template<typename> class D, typename T>
D<T> make(T mean, T dev) {
    return details::make(common::type_sequence<D<T>>(), mean, dev);
}


/**
 * @brief Macro defining a constant distribution for non-numeric types.
 * To be used as `CONSTANT_DISTRIBUTION(hello_distr, string, "hello");`.
 *
 * @param name The name of the distribution
 * @param T The result type.
 * @param val The constant value.
 */
#define CONSTANT_DISTRIBUTION(name, R, val)         \
struct name {                                       \
    using type = R;                                 \
    template <typename G>                           \
    name(G&&) {}                                     \
    template <typename G, typename S, typename T>   \
    name(G&&, const common::tagged_tuple<S,T>&) {}     \
    template <typename G>                           \
    type operator()(G&&) {                           \
        return val;                                 \
    }                                               \
}


/**
 * @brief Constant distribution for numeric types.
 * @param R The result type.
 * @param num The (integral) numerator of the value.
 * @param den The (optional, integral) denominator of the value.
 */
template <typename R, intmax_t num, intmax_t den = 1>
struct constant {
    using type = R;
    
    template <typename G>
    constant(G&&) {}
    
    template <typename G, typename S, typename T>
    constant(G&&, const common::tagged_tuple<S,T>&) {}
    
    template <typename G>
    type operator()(G&&) {
        return (type)num / (type)den;
    }
};


//! @cond INTERNAL
namespace details {
    template <typename R, typename G>
    typename R::type call_distr(G&& g) {
        R dist{g};
        return dist(g);
    }

    template <typename R, typename G, typename S, typename T>
    typename R::type call_distr(G&& g, const common::tagged_tuple<S,T>& t) {
        R dist{g, t};
        return dist(g);
    }
}
//! @endcond


//! @brief Uniform real distribution.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @param mean     The mean of the distribution (as distribution).
 * @param dev      The standard deviation of the distribution (as distribution).
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class uniform {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");
    
  public:
    using type = typename mean::type;
    
    template <typename G>
    uniform(G&& g) : m_d(make<std::uniform_real_distribution>(details::call_distr<mean>(g), details::call_distr<dev>(g))) {}
    
    template <typename G, typename S, typename T>
    uniform(G&& g, const common::tagged_tuple<S,T>& t) : m_d(make<std::uniform_real_distribution>(common::get_or<mean_tag>(t,details::call_distr<mean>(g, t)), common::get_or<dev_tag>(t,details::call_distr<dev>(g, t)))) {}
    
    template <typename G>
    type operator()(G&& g) {
        return m_d(g);
    }

  private:
    std::uniform_real_distribution<type> m_d;
};
/**
 * @brief With mean and deviation as numeric template parameters.
 * @param T        The type returned by the distribution.
 * @param mean     The (integral) mean of the distribution.
 * @param dev      The (integral) standard deviation of the distribution.
 * @param scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using uniform_t = uniform<constant<T, mean, scale>, constant<T, dev, scale>, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @param T        The type returned by the distribution.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, typename mean_tag, typename dev_tag>
using uniform_i = uniform<constant<T, 0>, constant<T, 0>, mean_tag, dev_tag>;
//! @}


//! @brief Uniform real distribution set up through extremes instead of mean and deviation.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @param min     The minimum of the distribution (as distribution).
 * @param max     The maximum of the distribution (as distribution).
 * @param min_tag The tag corresponding to the minimum in initialisation values.
 * @param max_tag The tag corresponding to the maximum in initialisation values.
 */
template <typename min, typename max, typename min_tag = void, typename max_tag = void>
class interval {
    static_assert(std::is_same<typename min::type, typename max::type>::value, "min and max of different type");
    
  public:
    using type = typename min::type;
    
    template <typename G>
    interval(G&& g) : m_d{details::call_distr<min>(g), details::call_distr<max>(g)} {}
    
    template <typename G, typename S, typename T>
    interval(G&& g, const common::tagged_tuple<S,T>& t) : m_d{common::get_or<min_tag>(t,details::call_distr<min>(g, t)), common::get_or<max_tag>(t,details::call_distr<max>(g, t))} {}
    
    template <typename G>
    type operator()(G&& g) {
        return m_d(g);
    }

  private:
    std::uniform_real_distribution<type> m_d;
};
/**
 * @brief With mean and deviation as numeric template parameters.
 * @param T       The type returned by the distribution.
 * @param min     The (integral) minimum of the distribution.
 * @param max     The (integral) maximum of the distribution.
 * @param scale   An (optional) scale factor by which `min` and `max` are divided.
 * @param min_tag The tag corresponding to the minimum in initialisation values.
 * @param max_tag The tag corresponding to the maximum in initialisation values.
 */
template <typename T, intmax_t min, intmax_t max, intmax_t scale = 1, typename min_tag = void, typename max_tag = void>
using interval_t = interval<constant<T, min, scale>, constant<T, max, scale>, min_tag, max_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @param T       The type returned by the distribution.
 * @param min_tag The tag corresponding to the minimum in initialisation values.
 * @param max_tag The tag corresponding to the maximum in initialisation values.
 */
template <typename T, typename min_tag, typename max_tag>
using interval_i = interval<constant<T, 0>, constant<T, 0>, min_tag, max_tag>;
//! @}


//! @brief Normal real distribution.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @param mean     The mean of the distribution (as distribution).
 * @param dev      The standard deviation of the distribution (as distribution).
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class normal {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");
    
  public:
    using type = typename mean::type;
    
  public:
    template <typename G>
    normal(G&& g) : m_d(details::call_distr<mean>(g), details::call_distr<dev>(g)) {}
    
    template <typename G, typename S, typename T>
    normal(G&& g, const common::tagged_tuple<S,T>& t) : m_d(common::get_or<mean_tag>(t,details::call_distr<mean>(g, t)), common::get_or<dev_tag>(t,details::call_distr<dev>(g, t))) {}
    
    template <typename G>
    type operator()(G&& g) {
        return m_d(g);
    }

  private:
    std::normal_distribution<type> m_d;
};
/**
 * @brief With mean and deviation as numeric template parameters.
 * @param T        The type returned by the distribution.
 * @param mean     The (integral) mean of the distribution.
 * @param dev      The (integral) standard deviation of the distribution.
 * @param scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using normal_t = normal<constant<T, mean, scale>, constant<T, dev, scale>, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @param T        The type returned by the distribution.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, typename mean_tag, typename dev_tag>
using normal_i = normal<constant<T, 0>, constant<T, 0>, mean_tag, dev_tag>;
//! @}


//! @brief Exponential real distribution.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @param mean     The mean of the distribution (as distribution).
 * @param dev      The standard deviation of the distribution (as distribution).
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class exponential {
    static_assert(std::is_same<mean, dev>::value, "deviation must be equal to mean in exponential distributions");
    
  public:
    using type = typename mean::type;

    template <typename G>
    exponential(G&& g) : m_d(1/details::call_distr<mean>(g)) {}
    
    template <typename G, typename S, typename T>
    exponential(G&& g, const common::tagged_tuple<S,T>& t) : m_d(1/common::get_or<mean_tag>(t, details::call_distr<mean>(g, t))) {}
    
    template <typename G>
    type operator()(G&& g) {
        return m_d(g);
    }

  private:
    std::exponential_distribution<type> m_d;
};
/**
 * @brief With mean and deviation as numeric template parameters.
 * @param T        The type returned by the distribution.
 * @param mean     The (integral) mean of the distribution.
 * @param dev      The (integral) standard deviation of the distribution.
 * @param scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using exponential_t = exponential<constant<T, mean, scale>, constant<T, dev, scale>, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @param T        The type returned by the distribution.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, typename mean_tag, typename dev_tag>
using exponential_i = exponential<constant<T, 0>, constant<T, 0>, mean_tag, dev_tag>;
//! @}


//! @brief Weibull real distribution.
//! @{
/**
 * @brief With mean and deviation as distributions.
 * @param mean     The mean of the distribution (as distribution).
 * @param dev      The standard deviation of the distribution (as distribution).
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class weibull {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");

  public:
    using type = typename mean::type;
    
    template <typename G>
    weibull(G&& g) : m_d(make<std::weibull_distribution>(details::call_distr<mean>(g), details::call_distr<dev>(g))) {}
    
    template <typename G, typename S, typename T>
    weibull(G&& g, const common::tagged_tuple<S,T>& t) : m_d(make<std::weibull_distribution>(common::get_or<mean_tag>(t,details::call_distr<mean>(g, t)), common::get_or<dev_tag>(t,details::call_distr<dev>(g, t)))) {}
    
    template <typename G>
    type operator()(G&& g) {
        return m_d(g);
    }

  private:
    std::weibull_distribution<type> m_d;
};
/**
 * @brief With mean and deviation as numeric template parameters.
 * @param T        The type returned by the distribution.
 * @param mean     The (integral) mean of the distribution.
 * @param dev      The (integral) standard deviation of the distribution.
 * @param scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using weibull_t = weibull<constant<T, mean, scale>, constant<T, dev, scale>, mean_tag, dev_tag>;
/**
 * @brief With mean and deviation as initialisation values.
 * @param T        The type returned by the distribution.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, typename mean_tag, typename dev_tag>
using weibull_i = weibull<constant<T, 0>, constant<T, 0>, mean_tag, dev_tag>;
//! @}


/**
 * @brief Modifies a real distribution to be positive.
 *
 * Assumes that the probability of generating positive numbers is high.
 *
 * @param D A real distribution.
 */
template <typename D>
struct positive : public D {
    using type = typename D::type;
    
    using D::D;
    
    template <typename G>
    type operator()(G&& g) {
        type t = D::operator()(g);
        return (t >= 0) ? t : operator()(g);
    }
};


/**
 * @brief Combines multiple distributions into a single distribution generating vectors.
 *
 * All `Ds::type` must be convertible to `double`.
 *
 * @param Ds The distributions.
 */
template <typename... Ds>
class vec {
  public:
    using type = fcpp::vec<sizeof...(Ds)>;
    
    template <typename G>
    vec(G&& g) : m_distributions{Ds{g}...} {}
    
    template <typename G, typename S, typename T>
    vec(G&& g, const common::tagged_tuple<S,T>& t) : m_distributions{Ds{g,t}...} {}
    
    template <typename G>
    type operator()(G&& g) {
        return call_impl(g, std::make_index_sequence<sizeof...(Ds)>{});
    }
    
  private:
    template <typename G, size_t... i>
    type call_impl(G&& g, std::index_sequence<i...>) {
        return {std::get<i>(m_distributions)(g)...};
    }
    
    std::tuple<Ds...> m_distributions;
};


}


}

#endif // FCPP_OPTION_DISTRIBUTION_H_
