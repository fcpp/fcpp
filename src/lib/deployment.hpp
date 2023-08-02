// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

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
#include "lib/deployment/persister.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

// Namespace for all FCPP components.
namespace component {

/**
 * @brief Combination of components for deployments.
 *
 * It can be instantiated and run as:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * using net_t = deployment<decl_opts...>::net;
 * net_t network{common::make_tagged_tuple<init_opts...>(init_vals...)};
 * network.run();
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
DECLARE_COMBINE(deployment, hardware_logger, persister, storage, hardware_connector, timer, scheduler, hardware_identifier, randomizer, calculus);

}

}


#endif // FCPP_DEPLOYMENT_H_
