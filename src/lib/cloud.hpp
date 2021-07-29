// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

/**
 * @file cloud.hpp
 * @brief Bundle including all the cloud headers.
 */

#ifndef FCPP_CLOUD_H_
#define FCPP_CLOUD_H_

#include "lib/component.hpp"
#include "lib/cloud/graph_connector.hpp"
#include "lib/cloud/graph_spawner.hpp"

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

    //! @brief Namespace for all FCPP components.
    namespace component {

        /**
         * @brief Combination of components for batch graph simulations.
         *
         * It can be instantiated as `batch_graph_simulator<options...>::net`.
         */
        DECLARE_COMBINE(batch_graph_simulator, calculus, graph_connector, timer, scheduler, logger, storage, graph_spawner, identifier, randomizer);
    }
}

#endif // FCPP_CLOUD_H_
