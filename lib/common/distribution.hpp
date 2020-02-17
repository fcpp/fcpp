// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file distribution.hpp
 * @brief Collection of random distributions. Similar to distributions in `<random>`, but with distribution parameters as template arguments, which are also made uniform (mean and deviation) whenever possible.
 */

#ifndef FCPP_COMMON_DISTRIBUTION_H_
#define FCPP_COMMON_DISTRIBUTION_H_

#include <cmath>
#include <random>
#include <type_traits>

#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


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
    name(G&) {}                                     \
    template <typename G, typename S, typename T>   \
    name(G&, const fcpp::tagged_tuple<S,T>&) {}     \
    template <typename G>                           \
    type operator()(G&) {                           \
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
struct constant_distribution {
    using type = R;
    
    template <typename G>
    constant_distribution(G&) {}
    
    template <typename G, typename S, typename T>
    constant_distribution(G&, const tagged_tuple<S,T>&) {}
    
    template <typename G>
    type operator()(G&) {
        return (type)num / (type)den;
    }
};


//! @cond INTERNAL
namespace details {
    template <typename R, typename G>
    typename R::type call_distr(G& g) {
        R dist{g};
        return dist(g);
    }

    template <typename R, typename G, typename S, typename T>
    typename R::type call_distr(G& g, const tagged_tuple<S,T>& t) {
        R dist{g, t};
        return dist(g);
    }
}
//! @endcond


//! @brief Uniform real distribution.
//{@
/**
 * @param mean     The mean of the distribution (as distribution).
 * @param dev      The standard deviation of the distribution (as distribution).
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class uniform_distribution {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");
    
  public:
    using type = typename mean::type;
    
    template <typename G>
    uniform_distribution(G& g) : uniform_distribution(details::call_distr<mean>(g), details::call_distr<dev>(g)) {}
    
    template <typename G, typename S, typename T>
    uniform_distribution(G& g, const tagged_tuple<S,T>& t) : uniform_distribution(get_or<mean_tag>(t,details::call_distr<mean>(g, t)), get_or<dev_tag>(t,details::call_distr<dev>(g, t))) {}
    
    template <typename G>
    type operator()(G& g) {
        return m_d(g);
    }

  private:
    std::uniform_real_distribution<type> m_d;
    
    uniform_distribution(type m, type s) : m_d(m - 1.7320508075688772*s, m + 1.7320508075688772*s) {}
};
/**
 * @param T        The type returned by the distribution.
 * @param mean     The (integral) mean of the distribution.
 * @param dev      The (integral) standard deviation of the distribution.
 * @param scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using uniform_d = uniform_distribution<constant_distribution<T, mean, scale>, constant_distribution<T, dev, scale>, mean_tag, dev_tag>;
//@}


//! @brief Normal real distribution.
//{@
/**
 * @param mean     The mean of the distribution (as distribution).
 * @param dev      The standard deviation of the distribution (as distribution).
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class normal_distribution {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");
    
  public:
    using type = typename mean::type;
    
  public:
    template <typename G>
    normal_distribution(G& g) : m_d(details::call_distr<mean>(g), details::call_distr<dev>(g)) {}
    
    template <typename G, typename S, typename T>
    normal_distribution(G& g, const tagged_tuple<S,T>& t) : m_d(get_or<mean_tag>(t,details::call_distr<mean>(g, t)), get_or<dev_tag>(t,details::call_distr<dev>(g, t))) {}
    
    template <typename G>
    type operator()(G& g) {
        return m_d(g);
    }

  private:
    std::normal_distribution<type> m_d;
};
/**
 * @param T        The type returned by the distribution.
 * @param mean     The (integral) mean of the distribution.
 * @param dev      The (integral) standard deviation of the distribution.
 * @param scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using normal_d = normal_distribution<constant_distribution<T, mean, scale>, constant_distribution<T, dev, scale>, mean_tag, dev_tag>;
//@}


//! @brief Exponential real distribution.
//{@
/**
 * @param mean     The mean of the distribution (as distribution).
 * @param dev      The standard deviation of the distribution (as distribution).
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class exponential_distribution {
    static_assert(std::is_same<mean, dev>::value, "deviation must be equal to mean in exponential distributions");
    
  public:
    using type = typename mean::type;

    template <typename G>
    exponential_distribution(G& g) : m_d(1/details::call_distr<mean>(g)) {}
    
    template <typename G, typename S, typename T>
    exponential_distribution(G& g, const tagged_tuple<S,T>& t) : m_d(1/get_or<mean_tag>(t, details::call_distr<mean>(g, t))) {}
    
    template <typename G>
    type operator()(G& g) {
        return m_d(g);
    }

  private:
    std::exponential_distribution<type> m_d;
    bool start = true;
};
/**
 * @param T        The type returned by the distribution.
 * @param mean     The (integral) mean of the distribution.
 * @param dev      The (integral) standard deviation of the distribution.
 * @param scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using exponential_d = exponential_distribution<constant_distribution<T, mean, scale>, constant_distribution<T, dev, scale>, mean_tag, dev_tag>;
//@}


//! @brief Weibull real distribution.
//{@
/**
 * @param mean     The mean of the distribution (as distribution).
 * @param dev      The standard deviation of the distribution (as distribution).
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename mean, typename dev, typename mean_tag = void, typename dev_tag = void>
class weibull_distribution {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");

  public:
    using type = typename mean::type;

    template <typename G>
    weibull_distribution(G& g) : weibull_distribution(details::call_distr<mean>(g), details::call_distr<dev>(g)) {}
    
    template <typename G, typename S, typename T>
    weibull_distribution(G& g, const tagged_tuple<S,T>& t) : weibull_distribution(get_or<mean_tag>(t,details::call_distr<mean>(g,t)), get_or<dev_tag>(t,details::call_distr<dev>(g,t))) {}
    
    template <typename G>
    type operator()(G& g) {
        return m_d(g);
    }

  private:
    weibull_distribution(type m, type s) {
        type t = log((s * s) / (m * m) + 1);
        type kmin = 0, kmax = 1;
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
        type shape = 1 / k;
        type scale = m / exp(lgamma(1 + k));
        m_d = std::weibull_distribution<type>(shape, scale);
    }
    std::weibull_distribution<type> m_d;
};
/**
 * @param T        The type returned by the distribution.
 * @param mean     The (integral) mean of the distribution.
 * @param dev      The (integral) standard deviation of the distribution.
 * @param scale    An (optional) scale factor by which `mean` and `dev` are divided.
 * @param mean_tag The tag corresponding to the mean in initialisation values.
 * @param dev_tag  The tag corresponding to the deviation in initialisation values.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1, typename mean_tag = void, typename dev_tag = void>
using weibull_d = weibull_distribution<constant_distribution<T, mean, scale>, constant_distribution<T, dev, scale>, mean_tag, dev_tag>;
//@}


/**
 * @brief Modifies a real distribution to be positive.
 * Assumes that the probability of generating positive numbers is high.
 *
 * @param D A real distribution.
 */
template <typename D>
class make_positive : public D {
  public:
    using type = typename D::type;
    
    using D::D;
    
    template <typename G>
    type operator()(G& g) {
        type t = D::operator()(g);
        return (t >= 0) ? t : operator()(g);
    }
};


}

#endif // FCPP_COMMON_DISTRIBUTION_H_
