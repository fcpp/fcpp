// Copyright © 2019 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/data/twin.hpp"


class TwinTest : public ::testing::Test {
protected:
    virtual void SetUp() {
    }
    
    fcpp::twin<int, true> mirrored;
    fcpp::twin<int, false> separate;
};


TEST_F(TwinTest, Mirrored) {
    mirrored.first() = 42;
	EXPECT_EQ(42, mirrored.second());
    mirrored.second() = 17;
    EXPECT_EQ(17, mirrored.first());
}

TEST_F(TwinTest, Separate) {
    separate.first() = 42;
    separate.second() = 17;
    EXPECT_EQ(42, separate.first());
    EXPECT_EQ(17, separate.second());
}
