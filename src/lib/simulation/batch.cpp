// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "lib/simulation/batch.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing tools for batch execution of FCPP simulations.
namespace batch {


#ifdef FCPP_MPI

bool mpi_init(int& rank, int& n_procs) {
    int initialized;
    MPI_Initialized(&initialized);
    if (not initialized) {
        int provided;
        MPI_Init_thread(NULL, NULL, MPI_THREAD_SERIALIZED, &provided);
        assert(provided == MPI_THREAD_SERIALIZED);
    }
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
    return not initialized;
}

void mpi_barrier() {
    MPI_Barrier(MPI_COMM_WORLD);
}

void mpi_finalize() {
    MPI_Finalize();
}

#else

bool mpi_init(int& rank, int& n_procs) {
    rank = 0;
    n_procs = 1;
    return true;
}

void mpi_barrier() {}

void mpi_finalize() {}

#endif


} // batch


} // fcpp
