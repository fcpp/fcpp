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

#include "lib/common/distribution.hpp"
#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a random number generator type.
    template <typename T>
    struct generator {};

    //! @brief Initialisation tag associating to a random number generator seed.
    struct seed {};
}


/**
 * @brief Component handling random number generation.
 *
 * Must be unique in a composition of components.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::generator defines a random number generator type (defaults to `std::mt19937_64`).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::seed associates to a random number generator seed (defaults to `P::node::uid`).
 *
 * <b>Net initialisation tags:</b>
 * - \ref tags::seed associates to a random number generator seed (defaults to zero).
 */
template <class... Ts>
struct randomizer {
    //! @brief Random number generator type.
    using generator_type = common::option_type<tags::generator, std::mt19937_64, Ts...>;

    /**
     * @brief The actual component.
     * 
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
        
        //! @brief Asserts that P has no `randomizer_tag`.
        static_assert(not has_tag<P>::value, "cannot combine multiple randomizer components");

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, const common::tagged_tuple<S,T>& t) : P::node(n,t), m_generator(common::get_or<tags::seed>(t, P::node::uid)) {}

            /**
             * @brief Write access to stored data.
             *
             * @param T The tag corresponding to the data to be accessed.
             */
            //! @brief Gives access to the random number generator.
            inline generator_type& generator() {
                return m_generator;
            }
            
            //! @brief Generates an `int` value between 0 and `b`.
            inline int next_int(int b = std::numeric_limits<int>::max()) {
                return next_int(0,b);
            }

            //! @brief Generates an `int` value between `a` and `b`.
            int next_int(int a, int b) {
                std::uniform_int_distribution<int> dist(a,b);
                return dist(generator());
            }
            
            //! @brief Generates a `double` value between zero and `b`.
            inline double next_double(double b = 1.0) {
                return next_double(0.0,b);
            }

            //! @brief Generates a `double` value between `a` and `b`.
            double next_double(double a, double b) {
                std::uniform_real_distribution<double> dist(a,b);
                return dist(generator());
            }
            
            //! @brief Applies a random relative `r` and absolute `a` deviation to a value `v` with distribution `D` from header `<random>`.
            template <template<typename> class D>
            double random_error(double v, double r, double a = 0) {
                D<double> dist = random::make_distribution<D>(v, r*v+a);
                return dist(m_generator);
            }
            
          private: // implementation details
            //! @brief The random number generator.
            generator_type m_generator;
        };
        
        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            net(const common::tagged_tuple<S,T>& t) : P::net(t), m_generator(common::get_or<tags::seed>(t, 0)) {}
            
          protected: // visible by net objects only
            //! @brief Gives access to the random number generator.
            inline generator_type& generator() {
                return m_generator;
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
            
            //! @brief Generates a `double` value between zero and `b`.
            inline double next_double(double b = 1.0) {
                return next_double(0.0,b);
            }

            //! @brief Generates a `double` value between `a` and `b`.
            double next_double(double a, double b) {
                std::uniform_real_distribution<double> dist(a,b);
                return dist(m_generator);
            }
            
            //! @brief Applies a random relative `r` and absolute `a` deviation to a value `v` with distribution `D` from header `<random>`.
            template <template<typename> class D>
            double random_error(double v, double r, double a = 0) {
                D<double> dist = random::make_distribution<D>(v, r*v+a);
                return dist(m_generator);
            }
            
          private: // implementation details
            //! @brief The random number generator.
            generator_type m_generator;
        };
    };
};


}


}

#endif // FCPP_COMPONENT_RANDOMIZER_H_
