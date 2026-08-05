// Copyright © 2026 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/common/utilities.hpp"

#include "test/helper.hpp"

using namespace fcpp;

TEST(UtilitiesTest, MakeIstream) {
    std::string s = "__tmp__", t;
    {
        std::ofstream os(s);
        ASSERT_TRUE(os.is_open());
        os << s << std::endl;
    }
    {
        std::shared_ptr<std::istream> is1 = common::make_istream(s);
        *is1 >> t;
        EXPECT_EQ(t, s);
    }
    {
        std::shared_ptr<std::istream> is2 = common::make_istream(s.c_str());
        *is2 >> t;
        EXPECT_EQ(t, s);
    }
    {
        std::ifstream is(s);
        std::shared_ptr<std::istream> is3 = common::make_istream(&is);
        *is3 >> t;
        EXPECT_EQ(t, s);
    }
    std::remove(s.c_str());
}
