// Copyright © 2026 Giorgio Audrito. All Rights Reserved.

#include "lib/common/mpi.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing objects of common use.
namespace common {


mpi_manager::mpi_manager(int n_tags, bool multithread) :
#ifdef FCPP_MPI
    initialized(get_initialized(multithread)), multithread(multithread), rank(get_rank()), n_procs(get_n_procs()), n_tags(n_tags),
    m_allowed_thread(std::this_thread::get_id()), m_sends(), m_sends_it(m_sends.end()), m_recvs(n_procs+1, std::vector<promise>(n_tags)) {
        m_recvs[rank].clear();
    }
#else
    initialized(false), multithread(true), rank(0), n_procs(1), n_tags(n_tags) {}
#endif

mpi_manager::~mpi_manager() {
    #ifdef FCPP_MPI
        for (auto& s : m_sends) if (s.request != MPI_REQUEST_NULL) {
            MPI_Cancel(&s.request);
            MPI_Wait(&s.request, MPI_STATUS_IGNORE);
        }
        for (auto& r : m_recvs) for (auto& p : r) if (p.request != MPI_REQUEST_NULL) {
            MPI_Cancel(&p.request);
            MPI_Wait(&p.request, MPI_STATUS_IGNORE);
        }
        if (initialized) MPI_Finalize();
    #endif
}


void mpi_manager::isend(int tag, mpi_message msg) {
    assert(0 <= tag and tag < n_tags);
    assert(msg.rank == MPI_ANY_SOURCE or (0 <= msg.rank and msg.rank < n_procs and msg.rank != rank));
    #ifdef FCPP_MPI
        assert(multithread or m_allowed_thread == std::this_thread::get_id());
        std::lock_guard<std::mutex> l(m_isend_mutex);
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


mpi_message mpi_manager::recv(int tag, int rank) {
    assert(0 <= tag and tag < n_tags);
    assert(rank == MPI_ANY_SOURCE or (0 <= rank and rank < n_procs and rank != this->rank));
    #ifdef FCPP_MPI
        assert(multithread or m_allowed_thread == std::this_thread::get_id());
        mpi_message m;
        MPI_Message message;
        MPI_Status status;
        MPI_Mprobe(rank, tag, MPI_COMM_WORLD, &message, &status);
        int size;
        MPI_Get_count(&status, MPI_CHAR, &size);
        m.data.resize(size);
        MPI_Mrecv(m.data.data(), m.data.size(), MPI_CHAR, &message, MPI_STATUS_IGNORE);
        m.rank = status.MPI_SOURCE;
        return m;
    #else
        return {};
    #endif
}

option<mpi_message> mpi_manager::irecv(int tag, int rank) {
    assert(0 <= tag and tag < n_tags);
    assert(rank == MPI_ANY_SOURCE or (0 <= rank and rank < n_procs and rank != this->rank));
    #ifdef FCPP_MPI
        assert(multithread or m_allowed_thread == std::this_thread::get_id());
        option<mpi_message> m;
        int irank = rank >= 0 ? rank : n_procs;
        int count;
        MPI_Status status;
        promise& p = m_recvs[irank][tag];
        if (p.request == MPI_REQUEST_NULL) {
            MPI_Message message;
            MPI_Improbe(rank, tag, MPI_COMM_WORLD, &count, &message, &status);
            if (not count) return m;
            MPI_Get_count(&status, MPI_CHAR, &count);
            p.data.resize(count);
            MPI_Imrecv(p.data.data(), p.data.size(), MPI_CHAR, &message, &p.request);
        }
        MPI_Test(&p.request, &count, &status);
        if (count) m.emplace(status.MPI_SOURCE, std::move(p.data));
        return m;
    #else
        return {};
    #endif
}


#ifdef FCPP_MPI
    bool mpi_manager::get_initialized(bool& multithread) {
        int init;
        MPI_Initialized(&init);
        if (init) return false;
        int noargc = 0;
        char** noargv = nullptr;
        int provided;
        MPI_Init_thread(&noargc, &noargv, multithread ? MPI_THREAD_MULTIPLE : MPI_THREAD_FUNNELED, &provided);
        if (provided < MPI_THREAD_MULTIPLE) multithread = false;
        return true;
    }
#endif


} // namespace common


} // namespace fcpp
