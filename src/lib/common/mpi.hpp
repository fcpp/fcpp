// Copyright © 2026 Giorgio Audrito. All Rights Reserved.

/**
 * @file mpi.hpp
 * @brief Implementation of function abstractions over MPI communication.
 */

#ifndef FCPP_COMMON_MPI_H_
#define FCPP_COMMON_MPI_H_

#ifdef FCPP_MPI
    #include <mpi.h>

    #include <list>
    #include <mutex>
    #include <thread>
#else
    //! @brief Identifier for any MPI source (used in MPI receive calls).
    #define MPI_ANY_SOURCE -1
#endif

#include <assert.h>

#include <vector>

#include "lib/settings.hpp"
#include "lib/common/option.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing objects of common use.
namespace common {


//! @brief Struct modelling a received MPI message.
struct mpi_message {
    //! @brief Default constructor.
    mpi_message() = default;
    //! @brief Initializing constructor.
    mpi_message(int rank, std::vector<char> data) : rank(rank), data(data) {}

    //! @brief The rank of the message.
    int rank;
    //! @brief The payload of the message.
    std::vector<char> data;
};


/**
 * @brief Struct managing MPI communication.
 *
 * Does not allow inter-rank communication.
 * Exceeding the maximum buffer size for receival results in undefined behaviour.
 */
struct mpi_manager {
    //! @brief Initialize the MPI manager possibly enabling multithreading support.
    mpi_manager(int n_tags, bool multithread = true);
    //! @brief Finalize the MPI manager.
    ~mpi_manager();

    //! @brief Disabled copy constructor.
    mpi_manager(mpi_manager const&) = delete;
    //! @brief Disabled copy assignment.
    mpi_manager& operator=(mpi_manager const&) = delete;

    //! @brief Disabled move constructor.
    mpi_manager(mpi_manager&&) = delete;
    //! @brief Disabled move assignment.
    mpi_manager& operator=(mpi_manager&&) = delete;

    //! @brief Forces MPI processes to wait for each other.
    inline void barrier() {
        #ifdef FCPP_MPI
            MPI_Barrier(MPI_COMM_WORLD);
        #endif
    }

    //! @brief Sends a message to another rank (blocking).
    inline void send(int tag, mpi_message const& msg) {
        assert(0 <= tag and tag < n_tags);
        assert(msg.rank == MPI_ANY_SOURCE or (0 <= msg.rank and msg.rank < n_procs and msg.rank != rank));
        #ifdef FCPP_MPI
            assert(multithread or m_allowed_thread == std::this_thread::get_id());
            MPI_Send(msg.data.data(), msg.data.size(), MPI_CHAR, msg.rank, tag, MPI_COMM_WORLD);
        #endif
    }
    //! @brief Sends a message to another rank (non-blocking).
    void isend(int tag, mpi_message msg);

    //! @brief Receives a message from another rank (blocking).
    mpi_message recv(int tag, int rank = MPI_ANY_SOURCE);
    //! @brief Receives messages from another rank (non-blocking). Concurrent irecvs with the same tag and rank are undefined behaviour.
    option<mpi_message> irecv(int tag, int rank = MPI_ANY_SOURCE);

    //! @brief Whether this manager supports multithreaded use.
    bool const multithread;
    //! @brief Whether this manager initialized MPI.
    bool const initialized;
    //! @brief The rank of the current MPI process.
    int const rank;
    //! @brief The total number of MPI processes.
    int const n_procs;
    //! @brief The number of tags (starting from 0) available for MPI communication.
    int const n_tags;

#ifdef FCPP_MPI
private:
    //! @brief Wrapper to MPI_Initialized and MPI_Init (_thread).
    static bool get_initialized(bool& multithread, std::thread::id& allowed_thread);

    //! @brief Wrapper to MPI_Comm_rank.
    inline static int get_rank() {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        return rank;
    }

    //! @brief Wrapper to MPI_Comm_size.
    inline static int get_n_procs() {
        int n_procs;
        MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
        return n_procs;
    }

    //! @brief Data holder for async operations.
    struct promise {
        //! @brief Constructor.
        promise(std::vector<char> data = {}, MPI_Request request = MPI_REQUEST_NULL) : data(data), request(request) {}

        //! @brief The payload.
        std::vector<char> data;
        //! @brief The request.
        MPI_Request request;
    };

    //! @brief The only thread allowed to perform MPI operations (if not multithreaded).
    std::thread::id m_allowed_thread;
    //! @brief Mutex for multithreaded isend operations.
    std::mutex m_isend_mutex;
    //! @brief The list of send promises.
    std::list<promise> m_sends;
    //! @brief Circular iterator on the list of send promises for cleanup.
    std::list<promise>::iterator m_sends_it;
    //! @brief Matrix of n_procs+1 x n_tags receive promises.
    std::vector<std::vector<promise>> m_recvs;
#endif
};


} // namespace common


} // namespace fcpp

#endif // FCPP_COMMON_MPI_H_
