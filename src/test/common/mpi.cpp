// Copyright © 2026 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"
#include "lib/common/mpi.hpp"

using namespace fcpp;

namespace {
    // Shared global manager pointer for all tests
    std::unique_ptr<common::mpi_manager> mpi;
}

class MpiEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        // Create 10 communication tags for the test suite
        mpi = std::make_unique<common::mpi_manager>(10);
        std::cerr << ("Running tests on MPI node " + std::to_string(mpi->rank) + "/" + std::to_string(mpi->n_procs)) << std::endl;
    }

    void TearDown() override {
        std::cerr << ("Ending tests on MPI node " + std::to_string(mpi->rank) + "/" + std::to_string(mpi->n_procs)) << std::endl;
        mpi.reset();
    }
};

::testing::Environment* const mpi_env = ::testing::AddGlobalTestEnvironment(new MpiEnvironment);


TEST(MpiTest, Init) {
    ASSERT_NE(mpi, nullptr);
#ifdef FCPP_MPI
    EXPECT_TRUE(mpi->initialized);
#else
    EXPECT_FALSE(mpi->initialized);
#endif
    EXPECT_GE(mpi->rank, 0);
    EXPECT_LT(mpi->rank, mpi->n_procs);
    EXPECT_EQ(mpi->n_tags, 10);
    {
        common::mpi_manager m2(5, false);
        EXPECT_FALSE(m2.initialized);
        EXPECT_EQ(m2.rank, mpi->rank);
        EXPECT_EQ(m2.n_procs, mpi->n_procs);
        EXPECT_EQ(m2.n_tags, 5);
        m2.barrier();
    }
    mpi->barrier();
}

TEST(MpiTest, Blocking) {
    constexpr int tag = 1;
    std::vector<char> payload = {'H', 'E', 'L', 'L', 'O'};
    // master sends to everyone else
    if (mpi->rank == 0) {
        for (int r = 1; r < mpi->n_procs; ++r)
            mpi->send(tag, {r, payload});
    } else {
        common::mpi_message msg = mpi->recv(tag, 0);
        EXPECT_EQ(msg.rank, 0);
        EXPECT_EQ(msg.data, payload);
    }
    mpi->barrier();
    // everyone sends to the master
    if (mpi->rank == 0) {
        std::vector<bool> incoming(mpi->n_procs);
        for (int r = 1; r < mpi->n_procs; ++r) {
            // receives from any source
            common::mpi_message msg = mpi->recv(tag);
            EXPECT_GT(msg.rank, 0);
            EXPECT_EQ(msg.data, payload);
            EXPECT_FALSE(incoming[msg.rank]);
            incoming[msg.rank] = true;
        }
    } else {
        mpi->send(tag, {0, payload});
    }
    mpi->barrier();
}

TEST(MpiTest, NonBlocking) {
    constexpr int tag = 2;
    std::vector<char> payload = {'A', 'S', 'Y', 'N', 'C'};
    if (mpi->rank == 0) {
        // master sends to everyone then listens for replies
        for (int r = 1; r < mpi->n_procs; ++r) {
            mpi->isend(tag, {r, payload});
        }
        std::vector<bool> incoming(mpi->n_procs);
        for (int r = 1; r < mpi->n_procs; ) {
            common::option<common::mpi_message> msg = mpi->irecv(tag);
            if (not msg.empty()) {
                EXPECT_GT(msg.front().rank, 0);
                EXPECT_EQ(msg.front().data, payload);
                EXPECT_FALSE(incoming[msg.front().rank]);
                incoming[msg.front().rank] = true;
                ++r;
            }
        }
    } else {
        // everyone waits for master message and echoes it back
        common::option<common::mpi_message> msg;
        while (msg.empty()) {
            msg = mpi->irecv(tag, 0);
        }
        EXPECT_EQ(msg.front().rank, 0);
        EXPECT_EQ(msg.front().data, payload);
        mpi->isend(tag, {0, msg.front().data});
    }
    mpi->barrier();
}
