cc_test(
    name = "collection_compare",
    srcs = ["collection_compare.hpp", "collection_compare.cpp"],
    deps = [
        "@gtest//:main",
        "//lib:fcpp",
        "//test:test_net",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "slow_distance",
    srcs = ["slow_distance.hpp", "slow_distance.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/component:identifier",
        "//lib/component:storage",
        "//lib/simulation:physical_position",
        "//lib/coordination:spreading",
        "//test:test_net",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
