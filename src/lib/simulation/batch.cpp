// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

#include "lib/simulation/batch.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing tools for batch execution of FCPP simulations.
namespace batch {


#ifdef FCPP_MPI

void mpi_init(int& rank, int& n_procs) {
    int provided;
    // TODO: we should avoid MPI_THREAD_MULTIPLE and use MPI_THREAD_SERIALIZED instead
    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
    assert(provided == MPI_THREAD_MULTIPLE);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
}

void mpi_barrier() {
    MPI_Barrier(MPI_COMM_WORLD);
}

void mpi_finalize() {
    MPI_Finalize();
}

#else

void mpi_init(int& rank, int& n_procs) {
    rank = 0;
    n_procs = 1;
}

void mpi_barrier() {}

void mpi_finalize() {}

#endif


} // batch


} // fcpp
