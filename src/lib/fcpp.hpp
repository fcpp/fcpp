// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file fcpp.hpp
 * @brief Bundle including all FCPP headers.
 *
 * Headers are grouped into folders (and namespaces), which are further grouped in conceptual layers.
 *
 * The first <i>data structures</i> layer comprises general classes, useful for the second and third layers.
 * It includes the following folders:
 * - <b>common</b> (namespace \ref fcpp::common): general purpose classes, which may be useful outside FCPP.
 * - <b>data</b> (main namespace \ref fcpp): data structures used by aggregate functions.
 * - <b>internal</b> (namespace \ref fcpp::internal): internal classes with no purpose outside FCPP.
 * - <b>option</b> (file-based namespaces): classes to be used as options for components.
 *
 * The second <i>components</i> layer comprises the components to be combined for generating the
 * <b>node</b> and <b>net</b> abstractions. It includes the following folders:
 * - <b>component</b> (namespace \ref fcpp::component): general purpose components.
 * - <b>deployment</b> (namespace \ref fcpp::component): components for deployed systems.
 * - <b>simulation</b> (namespace \ref fcpp::component): components for simulated systems.
 *
 * The third <i>aggregate functions</i> layer comprises libraries of field calculus functions,
 * possibly using the <b>node</b> abstraction and implementing composable distributed behaviour.
 * It includes the only folder:
 * - <b>coordination</b> (namespace \ref fcpp::coordination): libraries of aggregate functions.
 */

#ifndef FCPP_FCPP_H_
#define FCPP_FCPP_H_

#include "lib/beautify.hpp"
#include "lib/cloud.hpp"
#include "lib/common.hpp"
#include "lib/component.hpp"
#include "lib/coordination.hpp"
#include "lib/data.hpp"
#include "lib/deployment.hpp"
#include "lib/internal.hpp"
#include "lib/option.hpp"
#include "lib/settings.hpp"
#include "lib/simulation.hpp"

#endif // FCPP_FCPP_H_
