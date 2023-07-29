// Copyright © 2021 Giorgio Audrito and Gianluca Torta. All Rights Reserved.

/**
 * @file cloud.hpp
 * @brief Bundle including all the cloud headers.
 */

#ifndef FCPP_CLOUD_H_
#define FCPP_CLOUD_H_

#include "lib/component.hpp"
#include "lib/cloud/graph_connector.hpp"
#include "lib/cloud/graph_spawner.hpp"
#include "lib/simulation/displayer.hpp"
#include "lib/simulation/simulated_positioner.hpp"

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

// Namespace for all FCPP components.
namespace component {

/**
 * @brief Combination of components for batch graph simulations.
 *
 * It can be instantiated and run as:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * using net_t = batch_graph_simulator<decl_opts...>::net;
 * net_t network{common::make_tagged_tuple<init_opts...>(init_vals...)};
 * network.run();
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
         */
DECLARE_COMBINE(batch_graph_simulator, graph_connector, timer, scheduler, logger, storage, graph_spawner, identifier, randomizer, calculus);

/**
 * @brief Combination of components for batch graph simulations with a graphical interface.
 *
 * It can be instantiated and run as:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * using net_t = interactive_graph_simulator<decl_opts...>::net;
 * net_t network{common::make_tagged_tuple<init_opts...>(init_vals...)};
 * network.run();
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
DECLARE_COMBINE(interactive_graph_simulator, displayer, simulated_positioner, graph_connector, timer, scheduler, logger, storage, graph_spawner, identifier, randomizer, calculus);

}

}

#endif // FCPP_CLOUD_H_
