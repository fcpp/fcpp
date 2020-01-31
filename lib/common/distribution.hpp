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
#define CONSTANT_DISTRIBUTION(name, T, val)     \
struct name {                                   \
    using type = T;                             \
    template <typename G>                       \
    name(G&) {}                                 \
    template <typename G>                       \
    type operator()(G&) {                       \
        return val;                             \
    }                                           \
}


/**
 * @brief Constant distribution for numeric types.
 * @param T The result type.
 * @param num The (integral) numerator of the value.
 * @param den The (optional, integral) denominator of the value.
 */
template <typename T, intmax_t num, intmax_t den = 1>
struct constant_distribution {
    using type = T;
    
    template <typename G>
    constant_distribution(G&) {}
    
    template <typename G>
    T operator()(G&) {
        return (T)num / (T)den;
    }
};


//! @cond INTERNAL
namespace details {
    template <typename T, typename G>
    typename T::type call_distr(G& g) {
        T dist(g);
        return dist(g);
    }
}
//! @endcond


//! @brief Uniform real distribution.
//{@
/**
 * @param mean The mean of the distribution (as distribution).
 * @param dev The standard deviation of the distribution (as distribution).
 */
template <typename mean, typename dev>
class uniform_distribution {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");
    
  public:
    using type = typename mean::type;
    
  private:
    std::uniform_real_distribution<type> d;
    
    uniform_distribution(type m, type s) : d(m - 1.7320508075688772*s, m + 1.7320508075688772*s) {}
    
  public:
    template <typename G>
    uniform_distribution(G& g) : uniform_distribution(details::call_distr<mean>(g), details::call_distr<dev>(g)) {}
    
    template <typename G>
    type operator()(G& g) {
        return d(g);
    }
};
/**
 * @param T The type returned by the distribution.
 * @param mean The (integral) mean of the distribution.
 * @param dev The (integral) standard deviation of the distribution.
 * @param scale An (optional) scale factor by which `mean` and `dev` are divided.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1>
using uniform_d = uniform_distribution<constant_distribution<T, mean, scale>, constant_distribution<T, dev, scale>>;
//@}


//! @brief Normal real distribution.
//{@
/**
 * @param mean The mean of the distribution (as distribution).
 * @param dev The standard deviation of the distribution (as distribution).
 */
template <typename mean, typename dev>
class normal_distribution {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");
    
  public:
    using type = typename mean::type;

  private:
    std::normal_distribution<type> d;
    
  public:
    template <typename G>
    normal_distribution(G& g) : d(details::call_distr<mean>(g), details::call_distr<dev>(g)) {}
    
    template <typename G>
    type operator()(G& g) {
        return d(g);
    }
};
/**
 * @param T The type returned by the distribution.
 * @param mean The (integral) mean of the distribution.
 * @param dev The (integral) standard deviation of the distribution.
 * @param scale An (optional) scale factor by which `mean` and `dev` are divided.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1>
using normal_d = normal_distribution<constant_distribution<T, mean, scale>, constant_distribution<T, dev, scale>>;
//@}


//! @brief Exponential real distribution.
//{@
/**
 * @param mean The mean of the distribution (as distribution).
 * @param dev The standard deviation of the distribution (as distribution).
 */
template <typename mean, typename dev>
class exponential_distribution {
    static_assert(std::is_same<mean, dev>::value, "deviation must be equal to mean in exponential distributions");
    
  public:
    using type = typename mean::type;

  private:
    std::exponential_distribution<type> d;
    bool start = true;

  public:
    template <typename G>
    exponential_distribution(G& g) : d(1/details::call_distr<mean>(g)) {}
    
    template <typename G>
    type operator()(G& g) {
        return d(g);
    }
};
/**
 * @param T The type returned by the distribution.
 * @param mean The (integral) mean of the distribution.
 * @param dev The (integral, optional) standard deviation of the distribution.
 * @param scale An (optional) scale factor by which `mean` and `dev` are divided.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1>
using exponential_d = exponential_distribution<constant_distribution<T, mean, scale>, constant_distribution<T, dev, scale>>;
//@}


//! @brief Weibull real distribution.
//{@
/**
 * @param mean The mean of the distribution (as distribution).
 * @param dev The standard deviation of the distribution (as distribution).
 */
template <typename mean, typename dev>
class weibull_distribution {
    static_assert(std::is_same<typename mean::type, typename dev::type>::value, "mean and deviation of different type");

  public:
    using type = typename mean::type;

  private:
    std::weibull_distribution<type> d;

  public:
    template <typename G>
    weibull_distribution(G& g) {
        type m = details::call_distr<mean>(g);
        type s = details::call_distr<dev>(g);
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
        d = std::weibull_distribution<type>(shape, scale);
    }

    template <typename G>
    type operator()(G& g) {
        return d(g);
    }
};
/**
 * @param T The type returned by the distribution.
 * @param mean The (integral) mean of the distribution.
 * @param dev The (integral) standard deviation of the distribution.
 * @param scale An (optional) scale factor by which `mean` and `dev` are divided.
 */
template <typename T, intmax_t mean, intmax_t dev, intmax_t scale = 1>
using weibull_d = weibull_distribution<constant_distribution<T, mean, scale>, constant_distribution<T, dev, scale>>;
//@}
    

/**
 * @brief Modifies a real distribution to be positive.
 * Assumes that the probability of generating positive numbers is high.
 *
 * @param T A real distribution.
 */
template <typename T>
class make_positive : public T {
  public:
    using type = typename T::type;
    
    template <typename G>
    make_positive(G& g) : T(g) {}
    
    template <typename G>
    type operator()(G& g) {
        type t = T::operator()(g);
        return (t >= 0) ? t : operator()(g);
    }
};


}

#endif // FCPP_COMMON_DISTRIBUTION_H_
