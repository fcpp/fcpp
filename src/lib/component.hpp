// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file component.hpp
 * @brief Bundle including all the component headers.
 */

#ifndef FCPP_COMPONENT_H_
#define FCPP_COMPONENT_H_

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/component/identifier.hpp"
#include "lib/component/logger.hpp"
#include "lib/component/randomizer.hpp"
#include "lib/component/scheduler.hpp"
#include "lib/component/storage.hpp"
#include "lib/component/timer.hpp"

// Namespace containing all the objects in the FCPP library.
namespace fcpp {

/**
 * @brief Namespace for all FCPP components.
 *
 * For a comprehensive list of methods provided by all components to the `node` and `net` object, see also [this page](extras/docs/node_net.md).
 *
 * For a list of the available component options, see \ref fcpp::component::tags "this other page".
 */
namespace component {

/**
 * @brief Namespace of tags to be used for initialising components.
 *
 * Component tags are divided into:
 * - <b>declaration tags</b>: can be passed to \ref DECLARE_OPTIONS to affect behaviour of components at compile-time.
 * - <b>declaration flags</b>: declaration tags which can assume a boolean value.
 * - <b>net initialisation tags</b>: can be passed to the constructor of the `net` object to affect its behaviour at run-time.
 * - <b>node initialisation tags</b>: can be passed to the constructor of the `node` object (indirectly through the \ref tags::init "init" declaration tag) to affect individual node initialisation.
 *
 * Whenever a tags is omitted, its default value is used instead.
 * Tag can be passed in any order, and (for declaration tags) also while nesting \ref DECLARE_OPTIONS calls.
 */
namespace tags {}

}

}

#endif // FCPP_COMPONENT_H_
