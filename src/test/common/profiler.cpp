// Copyright © 2021 Giorgio Audrito. All Rights Reserved.

#include <sstream>

#include "gtest/gtest.h"

#include "lib/common/profiler.hpp"

namespace fcpp {

// slow computation
int workhard(int n=15) {
    if (n <= 1) return 1;
    return (workhard(n-1) + workhard(n-2))/2;
}

TEST(ProfilerTest, Main) {
    std::stringstream ss;
    {
        PROFILE_COUNT("setup");
        {
            PROFILE_COUNT("setup/main");
            workhard();
        }
        workhard();
    }
    {
        PROFILE_COUNT("teardown");
        workhard();
    }
    PROFILE_REPORT(ss);
}

}
