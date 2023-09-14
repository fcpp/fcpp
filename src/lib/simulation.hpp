// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

/**
 * @file simulation.hpp
 * @brief Bundle including all the simulation component headers.
 *
 * Includes also the general component headers.
 */

#ifndef FCPP_SIMULATION_H_
#define FCPP_SIMULATION_H_

#include "lib/component.hpp"
#include "lib/simulation/batch.hpp"
#include "lib/simulation/displayer.hpp"
#include "lib/simulation/simulated_connector.hpp"
#include "lib/simulation/simulated_positioner.hpp"
#include "lib/simulation/simulated_map.hpp"
#include "lib/simulation/spawner.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

// Namespace for all FCPP components.
namespace component {

/**
 * @brief Combination of components for batch simulations.
 *
 * It can be instantiated and run as:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * using net_t = batch_simulator<decl_opts...>::net;
 * net_t network{common::make_tagged_tuple<init_opts...>(init_vals...)};
 * network.run();
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

DECLARE_COMBINE(batch_simulator, simulated_connector, simulated_map, simulated_positioner, timer, scheduler, logger, storage, spawner, identifier, randomizer, calculus);

/**
 * @brief Combination of components for interactive simulations.
 *
 * It can be instantiated and run as:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * using net_t = interactive_simulator<decl_opts...>::net;
 * net_t network{common::make_tagged_tuple<init_opts...>(init_vals...)};
 * network.run();
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
DECLARE_COMBINE(interactive_simulator, displayer, simulated_connector, simulated_map, simulated_positioner, timer, scheduler, logger, storage, spawner, identifier, randomizer, calculus);

}

}


#endif // FCPP_SIMULATION_H_
