cc_test(
    name = "batch",
    srcs = ["batch.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:mutex",
        "//lib/simulation:batch",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "physical_connector",
    srcs = ["physical_connector.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:scheduler",
        "//lib/simulation:physical_connector",
        "//lib/simulation:physical_position",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "physical_position",
    srcs = ["physical_position.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:scheduler",
        "//lib/simulation:physical_position",
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
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
