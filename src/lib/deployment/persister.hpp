// Copyright © 2022 Giorgio Audrito. All Rights Reserved.

/**
 * @file persister.hpp
 * @brief Implementation of the `persister` component handling persistence of data across device reboots.
 */

#ifndef FCPP_DEPLOYMENT_PERSISTER_H_
#define FCPP_DEPLOYMENT_PERSISTER_H_

#include <fstream>
#include <string>
#include <type_traits>
#include <vector>

#include "lib/common/serialize.hpp"
#include "lib/component/base.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace for all FCPP components.
namespace component {


//! @brief Namespace of tags to be used for initialising components.
namespace tags {
    //! @brief Declaration tag associating to the input/output stream type to be used.
    template <typename T>
    struct stream_type {};

    //! @brief Node initialisation tag associating to a path for persistence.
    struct persistence_path {};
}


/**
 * @brief Component modelling persistent data.
 *
 * <b>Declaration tags:</b>
 * - \ref tags::stream_type defines the input/output stream type to be used (defaults to `std::fstream`).
 *
 * <b>Node initialisation tags:</b>
 * - \ref tags::persistence_path associates to a path for persistence (defaults to no persistence).
 */
template <typename... Ts>
struct persister {
    //! @brief The input/output stream type to be used.
    using stream_type = common::option_type<tags::stream_type, std::fstream, Ts ...>;

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
        DECLARE_COMPONENT(persister);
        REQUIRE_COMPONENT(persister,storage);
        //! @endcond

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
            node(typename F::net& n, common::tagged_tuple<S,T> const& t) : P::node(n,t), m_path(common::get_or<tags::persistence_path>(t, "")) {
                if (m_path.size()) {
                    stream_type in(m_path, std::ios_base::in);
                    std::vector<char> v;
                    char c;
                    while (in >> c) v.push_back(c);
                    if (v.size()) {
                        common::isstream is(std::move(v));
                        is >> P::node::storage_tuple();
                    }
                }
            }

            //! @brief Performs computations at round end with current time `t`.
            void round_end(times_t t) {
                P::node::round_end(t);
                if (m_path.size()) {
                    stream_type out(m_path, std::ios_base::out);
                    common::osstream os;
                    os << P::node::storage_tuple();
                    for (char c : os.data())
                        out << c;
                }
            }

          private: // implementation details
            //! @brief The data storage.
            std::string m_path;
        };

        //! @brief The global part of the component.
        using net = typename P::net;
    };
};


}


}

#endif // FCPP_DEPLOYMENT_PERSISTER_H_
