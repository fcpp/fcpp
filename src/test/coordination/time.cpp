// Copyright © 2020 Giorgio Audrito. All Rights Reserved.

#include "gtest/gtest.h"

#include "lib/component/base.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/time.hpp"

#include "test/test_net.hpp"

using namespace fcpp;
using namespace component::tags;


template <int O>
DECLARE_OPTIONS(options,
    exports<int, double>,
    export_pointer<(O & 1) == 1>,
    export_split<(O & 2) == 2>,
    online_drop<(O & 4) == 4>
);

DECLARE_COMBINE(calc_only, component::calculus);

template <int O>
using combo = calc_only<options<O>>;


MULTI_TEST(UtilsTest, Counter, O, 3) {
    test_net<combo<O>, std::tuple<int>()> n{
        [&](auto& node){
            return std::make_tuple(
                coordination::counter(node, 0)
            );
        }
    };
    EXPECT_ROUND(n, {1, 1, 1});
    EXPECT_ROUND(n, {2, 2, 2});
    EXPECT_ROUND(n, {3, 3, 3});
    EXPECT_ROUND(n, {4, 4, 4});
}
