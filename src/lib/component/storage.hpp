// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file storage.hpp
 * @brief Implementation of the `storage` component handling persistent data across rounds.
 */

#ifndef FCPP_COMPONENT_STORAGE_H_
#define FCPP_COMPONENT_STORAGE_H_

#include <type_traits>

#include "lib/component/base.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to a sequence of tags and types for storing persistent data.
    template <typename... Ts>
    struct tuple_store {};
}


/**
 * @brief Component modelling persistent data.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::tuple_store defines a sequence of tags and types for storing persistent data (defaults to the empty sequence).
 */
template <typename... Ts>
struct storage {
    //! @brief Sequence of tags and types for storing persistent data.
    using tuple_store_type = common::option_types<tags::tuple_store, Ts...>;

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
            using tuple_type = common::tagged_tuple_t<tuple_store_type>;

          private: // implementation details
            //! @brief Checks whether a type is supported by the storage.
            template <typename A>
            constexpr static bool type_supported = tuple_type::tags::template count<std::remove_reference_t<A>> != 0;

          public: // visible by net objects and the main program
            /**
             * @brief Main constructor.
             *
             * @param n The corresponding net object.
             * @param t A `tagged_tuple` gathering initialisation values.
             */
            template <typename S, typename T>
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_storage(t) {}

            //! @brief Const access to stored data as tagged tuple.
            tuple_type const& storage_tuple() const {
                return m_storage;
            }

            //! @cond INTERNAL
            #define MISSING_TYPE_MESSAGE "\033[1m\033[4munsupported tag access (add A to storage tag list)\033[0m"
            //! @endcond

            /**
             * @brief Write access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto& storage() {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return get_impl<T>(common::bool_pack<type_supported<T>>{});
            }

            /**
             * @brief Const access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto const& storage() const {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return get_impl<T>(common::bool_pack<type_supported<T>>{});
            }

            /**
             * @brief Write access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto& storage(T) {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return get_impl<T>(common::bool_pack<type_supported<T>>{});
            }

            /**
             * @brief Const access to stored data.
             *
             * @tparam T The tag corresponding to the data to be accessed.
             */
            template <typename T>
            auto const& storage(T) const {
                static_assert(type_supported<T>, MISSING_TYPE_MESSAGE);
                return get_impl<T>(common::bool_pack<type_supported<T>>{});
            }

            #undef MISSING_TYPE_MESSAGE

          private: // implementation details
            //! @brief Access to the data corresponding to an existing tag.
            template <typename T>
            inline auto& get_impl(common::bool_pack<true>) {
                return common::get<T>(m_storage);
            }

            //! @brief Const access to the data corresponding to an existing tag.
            template <typename T>
            inline auto const& get_impl(common::bool_pack<true>) const {
                return common::get<T>(m_storage);
            }

            //! @brief Access to the data corresponding to a non-existent tag.
            template <typename T>
            inline auto& get_impl(common::bool_pack<false>) {
                return m_storage;
            }

            //! @brief Const access to the data corresponding to a non-existent tag.
            template <typename T>
            inline auto const& get_impl(common::bool_pack<false>) const {
                return m_storage;
            }

            //! @brief The data storage.
            tuple_type m_storage;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_COMPONENT_STORAGE_H_
