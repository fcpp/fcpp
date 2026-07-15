// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file storage.hpp
 * @brief Implementation of the `storage` component handling persistent data across rounds.
 */

#ifndef FCPP_COMPONENT_STORAGE_H_
#define FCPP_COMPONENT_STORAGE_H_

#include <cassert>
#include <type_traits>

#include "lib/component/base.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


// Namespace for all FCPP components.
namespace component {


// Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of tags and types for storing global persistent data (defaults to the empty sequence).
    template <typename... Ts>
    struct net_store {};

    //! @brief Declaration tag associating to a sequence of tags and types for storing persistent data in nodes (defaults to the empty sequence).
    template <typename... Ts>
    struct node_store {};

    //! @brief Declaration tag associating to a sequence of tags and types for storing persistent data (legacy deprecated name).
    template <typename... Ts>
    using tuple_store = node_store<Ts...>;
}


/**
 * @brief Component modelling persistent data.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::net_store defines a sequence of tags and types for storing global persistent data (defaults to the empty sequence).
 * - \ref tags::node_store defines a sequence of tags and types for storing persistent data in nodes (defaults to the empty sequence).
 */
template <typename... Ts>
struct storage {
    //! @brief Sequence of tags and types for storing global persistent data.
    using net_store_type = common::storage_list<common::option_types<tags::net_store, Ts...>>;

    //! @brief Sequence of tags and types for storing persistent data in nodes.
    using node_store_type = common::storage_list<common::option_types<tags::node_store, Ts...>>;

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
        //! @cond INTERNAL
        DECLARE_COMPONENT(storage);
        //! @endcond

        //! @brief The local part of the component.
        class node : public P::node {
          public: // visible by net objects and the main program
            //! @brief Tuple type of the contents.
            using node_tuple_type = common::tagged_tuple_t<node_store_type>;

          private: // implementation details
            //! @brief Checks whether a type is supported by the storage.
            template <typename A>
            constexpr static bool type_supported = node_tuple_type::tags::template count<std::remove_reference_t<A>> != 0;

          public: // visible by net objects and the main program
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_storage(t) {}

            //! @brief Access to stored data as tagged tuple.
            node_tuple_type& storage_tuple() {
                return m_storage;
            }

            //! @brief Const access to stored data as tagged tuple.
            node_tuple_type const& storage_tuple() const {
                return m_storage;
            }

            //! @cond INTERNAL
            #define MISSING_TYPE_MESSAGE ANSI_START "unsupported tag access (add A to node storage tag list)" ANSI_END
            //! @endcond

            /**
             * @brief Write access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto& storage() {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return common::get_or_wildcard<T>(m_storage);
            }

            /**
             * @brief Const access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto const& storage() const {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return common::get_or_wildcard<T>(m_storage);
            }

            /**
             * @brief Write access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto& storage(T) {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return common::get_or_wildcard<T>(m_storage);
            }

            /**
             * @brief Const access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto const& storage(T) const {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return common::get_or_wildcard<T>(m_storage);
            }

            #undef MISSING_TYPE_MESSAGE

          private: // implementation details
            //! @brief The data storage.
            node_tuple_type m_storage;
        };

        //! @brief The global part of the component.
        class net : public P::net {
          public: // visible by node objects and the main program
            //! @brief Tuple type of the contents.
            using net_tuple_type = common::tagged_tuple_t<net_store_type>;

          private: // implementation details
            //! @brief Checks whether a type is supported by the storage.
            template <typename A>
            constexpr static bool type_supported = net_tuple_type::tags::template count<std::remove_reference_t<A>> != 0;

          public: // visible by node objects and the main program
            //! @brief Constructor from a tagged tuple.
            template <typename S, typename T>
            explicit net(common::tagged_tuple<S,T> const& t) : P::net(t), m_storage(t) {}

            //! @brief Access to stored data as tagged tuple.
            net_tuple_type& storage_tuple() {
                return m_storage;
            }

            //! @brief Const access to stored data as tagged tuple.
            net_tuple_type const& storage_tuple() const {
                return m_storage;
            }

            //! @cond INTERNAL
            #define MISSING_TYPE_MESSAGE ANSI_START "unsupported tag access (add A to net storage tag list)" ANSI_END
            //! @endcond

            /**
             * @brief Write access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto& storage() {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return common::get_or_wildcard<T>(m_storage);
            }

            /**
             * @brief Const access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto const& storage() const {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return common::get_or_wildcard<T>(m_storage);
            }

            /**
             * @brief Write access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto& storage(T) {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return common::get_or_wildcard<T>(m_storage);
            }

            /**
             * @brief Const access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto const& storage(T) const {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return common::get_or_wildcard<T>(m_storage);
            }

            #undef MISSING_TYPE_MESSAGE

          private: // implementation details
            //! @brief The data storage.
            net_tuple_type m_storage;
        };
    };
};


}


}

#endif // FCPP_COMPONENT_STORAGE_H_
