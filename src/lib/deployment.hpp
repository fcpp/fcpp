// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file deployment.hpp
 * @brief Bundle including all the deployment component headers.
 *
 * Includes also the general component headers.
 */

#ifndef FCPP_DEPLOYMENT_H_
#define FCPP_DEPLOYMENT_H_

#include "lib/component.hpp"
#include "lib/deployment/hardware_connector.hpp"
#include "lib/deployment/hardware_identifier.hpp"
#include "lib/deployment/hardware_logger.hpp"
#include "lib/deployment/os.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace for all FCPP components.
namespace component {

/**
 * @brief Combination of components for deployments.
 *
 * It can be instantiated as `deployment<options...>::net`.
 */
DECLARE_COMBINE(deployment, calculus, hardware_logger, storage, hardware_connector, timer, scheduler, hardware_identifier, randomizer);

}

}


#endif // FCPP_DEPLOYMENT_H_
