// Copyright © 2026 Giorgio Audrito. All Rights Reserved.

#include <cassert>

#include "lib/common/mpi.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing objects of common use.
namespace common {


mpi_manager::mpi_manager(int n_tags, bool multithread) :
#ifdef FCPP_MPI
    initialized(get_initialized(multithread)), rank(get_rank()), n_procs(get_n_procs()), n_tags(n_tags),
    m_sends(), m_sends_it(m_sends.end()), m_recvs(n_procs+1, std::vector<promise>(n_tags)) {}
#else
    initialized(false), rank(0), n_procs(1), n_tags(n_tags) {}
#endif

mpi_manager::~mpi_manager() {
    #ifdef FCPP_MPI
        for (auto& p : m_sends) MPI_Wait(&p.request, MPI_STATUS_IGNORE);
        for (auto& r : m_recvs) for (auto& p : r) if (p.request != MPI_REQUEST_NULL) {
            MPI_Cancel(&p.request);
            MPI_Wait(&p.request, MPI_STATUS_IGNORE);
        }
        if (initialized) MPI_Finalize();
    #endif
}


void mpi_manager::barrier() {
    #ifdef FCPP_MPI
        MPI_Barrier(MPI_COMM_WORLD);
    #endif
}


void mpi_manager::send(int tag, mpi_message const& msg) {
    assert(msg.rank != rank);
    #ifdef FCPP_MPI
        MPI_Send(msg.data.data(), msg.data.size(), MPI_CHAR, msg.rank, tag, MPI_COMM_WORLD);
    #endif
}

void mpi_manager::isend(int tag, mpi_message msg) {
    assert(msg.rank != rank);
    #ifdef FCPP_MPI
        m_sends.emplace_back(std::move(msg.data), MPI_REQUEST_NULL);
        MPI_Isend(m_sends.back().data.data(), m_sends.back().data.size(), MPI_CHAR, msg.rank, tag, MPI_COMM_WORLD, &m_sends.back().request);
        bool skipped = false;
        while (not m_sends.empty()) {
            if (m_sends_it == m_sends.end()) m_sends_it = m_sends.begin();
            int completed;
            MPI_Test(&m_sends_it->request, &completed, MPI_STATUS_IGNORE);
            if (completed) {
                m_sends_it = m_sends.erase(m_sends_it);
            } else {
                if (skipped) break;
                skipped = true;
                ++m_sends_it;
            }
        }
    #endif
}


mpi_message mpi_manager::recv(int tag, int rank, int buf_size) {
    assert(rank != this->rank);
    #ifdef FCPP_MPI
        mpi_message m;
        m.data.resize(buf_size);
        MPI_Status status;
        int size;
        MPI_Recv(m.data.data(), m.data.size(), MPI_CHAR, rank, tag, MPI_COMM_WORLD, &status);
        MPI_Get_count(&status, MPI_CHAR, &size);
        m.data.resize(size);
        m.rank = status.MPI_SOURCE;
        return m;
    #else
        return {};
    #endif
}

option<mpi_message> mpi_manager::irecv(int tag, int rank, int buf_size) {
    #ifdef FCPP_MPI
        option<mpi_message> m;
        int irank = rank >= 0 ? rank : n_procs;
        if (m_recvs[irank][tag].request == MPI_REQUEST_NULL) {
            m_recvs[irank][tag].data.resize(buf_size);
            MPI_Irecv(m_recvs[irank][tag].data.data(), buf_size, MPI_CHAR, rank, tag, MPI_COMM_WORLD, &m_recvs[irank][tag].request);
        }
        int count;
        MPI_Status status;
        MPI_Test(&m_recvs[irank][tag].request, &count, &status);
        if (count) {
            MPI_Get_count(&status, MPI_CHAR, &count);
            auto it = m_recvs[irank][tag].data.begin();
            m.emplace(status.MPI_SOURCE, std::vector<char>(it, it+count));
            MPI_Irecv(m_recvs[irank][tag].data.data(), buf_size, MPI_CHAR, rank, tag, MPI_COMM_WORLD, &m_recvs[irank][tag].request);
        }
        return m;
    #else
        return {};
    #endif
}


} // namespace common


} // namespace fcpp
