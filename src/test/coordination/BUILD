cc_test(
    name = "collection",
    srcs = ["collection.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:collection",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "election",
    srcs = ["election.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:election",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "geometry",
    srcs = ["geometry.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/component:randomizer",
        "//lib/coordination:geometry",
        "//lib/simulation:physical_position",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "spreading",
    srcs = ["spreading.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:spreading",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
