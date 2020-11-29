// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

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
#include "lib/simulation/simulated_connector.hpp"
#include "lib/simulation/simulated_positioner.hpp"
#include "lib/simulation/spawner.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace for all FCPP components.
namespace component {

/**
 * @brief Combination of components for batch simulations.
 *
 * It can be instantiated as `batch_simulator<options...>::net`.
 */
DECLARE_COMBINE(batch_simulator, calculus, simulated_connector, simulated_positioner, timer, scheduler, logger, storage, spawner, identifier, randomizer);

}

}


#endif // FCPP_SIMULATION_H_
