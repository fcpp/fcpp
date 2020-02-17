// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file randomizer.hpp
 * @brief Implementation of the `randomizer` component handling a random number generator.
 */

#ifndef FCPP_COMPONENT_RANDOMIZER_H_
#define FCPP_COMPONENT_RANDOMIZER_H_

#include <cstdlib>
#include <random>
#include <type_traits>

#include "lib/common/tagged_tuple.hpp"


//! @brief Namespace of tags to be used for `tagged_tuple` objects.
namespace tags {
    //! @brief Tag for setting the random number generator seed (defaults to 0).
    struct seed {};
}


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Lightweight interface for C random generators to C++ distributions.
struct crand {
    using result_type = int;
    
    explicit crand(result_type val = 0) {
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


/**
 * @brief Component handling a random number generator.
 * Initialises the generator with the value corresponding to `tags::seed` in the tagged tuple given.
 * Must be unique in a composition of components.
 *
 * @param G The generator type (defaults to `std::mt19937_64`, use `fcpp::crand` if no generator needed).
 */
template <typename G = std::mt19937_64>
struct randomizer {
    /**
     * @brief The actual component.
     * Component functionalities are added to those of the parent by inheritance at multiple levels: the whole component class inherits tag for static checks of correct composition, while `node` and `net` sub-classes inherit actual behaviour.
     * Further parametrisation with F enables <a href="https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">CRTP</a> for static emulation of virtual calls.
     *
     * @param F The final composition of all components.
     * @param P The parent component to inherit from.
     */
    template <typename F, typename P>
    struct component : public P {
        //! @brief Marks that a randomizer component is present.
        struct randomizer_tag {};
        
        //! @brief Checks if T has a `randomizer_tag`.
        template <typename T, typename = int>
        struct has_tag : std::false_type {};
        template <typename T>
        struct has_tag<T, std::conditional_t<true,int,typename T::randomizer_tag>> : std::true_type {};
        
        //! @brief Asserts that P has no `randomizer_tag` to avoid component duplication.
        static_assert(not has_tag<P>::value, "cannot combine two randomizer components");

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            using P::node::node;

          protected: // visible by node objects only
            /**
             * @brief Write access to stored data.
             *
             * @param T The tag corresponding to the data to be accessed.
             */
            //! @brief Gives access to the random number generator.
            inline G& generator() const {
                return P::node::net.generator();
            }
            
            //! @brief Generates a `double` value between zero and `b`.
            inline double next_double(double b = 1.0) const {
                return next_double(0.0,b);
            }

            //! @brief Generates a `double` value between `a` and `b`.
            double next_double(double a, double b) const {
                std::uniform_real_distribution<double> dist(a,b);
                return dist(generator());
            }
            
            //! @brief Generates an `int` value between 0 and `b`.
            inline int next_int(int b = std::numeric_limits<int>::max()) const {
                return next_int(0,b);
            }

            //! @brief Generates an `int` value between `a` and `b`.
            int next_int(int a, int b) const {
                std::uniform_int_distribution<int> dist(a,b);
                return dist(generator());
            }
        };
        
        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const tagged_tuple<S,T>& t) : P::net(t), m_generator(get_or<tags::seed>(t, 0)) {}
            
            //! @brief Gives access to the random number generator.
            G& generator() {
                return m_generator;
            }
            
          protected: // visible by net objects only
            //! @brief Generates a `double` value between zero and `b`.
            inline double next_double(double b = 1.0) {
                return next_double(0.0,b);
            }

            //! @brief Generates a `double` value between `a` and `b`.
            double next_double(double a, double b) {
                std::uniform_real_distribution<double> dist(a,b);
                return dist(m_generator);
            }
            
            //! @brief Generates an `int` value between 0 and `b`.
            inline int next_int(int b = std::numeric_limits<int>::max()) {
                return next_int(0,b);
            }

            //! @brief Generates an `int` value between `a` and `b`.
            int next_int(int a, int b) {
                std::uniform_int_distribution<int> dist(a,b);
                return dist(m_generator);
            }
            
          private: // implementation details
            //! @brief The random number generator.
            G m_generator;
        };
    };
};


}

#endif // FCPP_COMPONENT_RANDOMIZER_H_
