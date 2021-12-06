cc_test(
    name = "batch",
    srcs = ["batch.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:mutex",
        "//lib/common:ostream",
        "//lib/simulation:batch",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "simulated_connector",
    srcs = ["simulated_connector.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:scheduler",
        "//lib/simulation:simulated_connector",
        "//lib/simulation:simulated_positioner",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "simulated_map",
    srcs = ["simulated_map.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/simulation:simulated_map",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "simulated_positioner",
    srcs = ["simulated_positioner.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:scheduler",
        "//lib/simulation:simulated_positioner",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "spawner",
    srcs = ["spawner.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/component:identifier",
        "//lib/component:storage",
        "//lib/component:timer",
        "//lib/simulation:spawner",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
