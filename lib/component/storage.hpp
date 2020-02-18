// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file storage.hpp
 * @brief Implementation of the `storage` component handling persistent data across rounds.
 */

#ifndef FCPP_COMPONENT_STORAGE_H_
#define FCPP_COMPONENT_STORAGE_H_

#include <type_traits>

#include "lib/common/tagged_tuple.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

/**
 * @brief Component modelling persistent data.
 * Must be unique in a composition of components.
 *
 * @param Ss The tags and types of the `tagged_tuple` storing the data.
 */
template <typename... Ss>
struct storage {
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
        //! @brief Marks that a storage component is present.
        struct storage_tag {};
        
        //! @brief Checks if T has a `storage_tag`.
        template <typename T, typename = int>
        struct has_tag : std::false_type {};
        template <typename T>
        struct has_tag<T, std::conditional_t<true,int,typename T::storage_tag>> : std::true_type {};
        
        //! @brief Asserts that P has no `storage_tag`.
        static_assert(not has_tag<P>::value, "cannot combine multiple storage components");

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
            node(typename F::net& n, const tagged_tuple<S,T>& t) : P::node(n,t), m_storage(t) {}
            
          protected: // visible by node objects only
            /**
             * @brief Write access to stored data.
             *
             * @param T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            typename tagged_tuple_t<Ss...>::template tag_type<T>& storage() {
                return fcpp::get<T>(m_storage);
            }
            
            /**
             * @brief Const access to stored data.
             *
             * @param T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            const typename tagged_tuple_t<Ss...>::template tag_type<T>& storage() const {
                return fcpp::get<T>(m_storage);
            }
            
            //! @brief Const access to stored data as tagged tuple.
            const tagged_tuple_t<Ss...>& storage_tuple() const {
                return m_storage;
            }
            
          private: // implementation details
            //! @brief The data storage.
            tagged_tuple_t<Ss...> m_storage;
        };
        
        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}

#endif // FCPP_COMPONENT_STORAGE_H_
