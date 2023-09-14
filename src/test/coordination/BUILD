cc_test(
    name = "basics",
    srcs = ["basics.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:basics",
        "//test:test_net",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "collection",
    srcs = ["collection.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:collection",
        "//test:test_net",
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
        "//test:test_net",
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
        "//lib/simulation:simulated_positioner",
        "//test:test_net",
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
        "//test:test_net",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "time",
    srcs = ["time.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/component:timer",
        "//lib/coordination:time",
        "//test:test_net",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "utils",
    srcs = ["utils.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:utils",
        "//test:test_net",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
