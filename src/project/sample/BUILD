cc_binary(
    name = "runner",
    srcs = ["runner.cpp"],
    deps = [
        "//lib/common:aggregator",
        "//lib/common:array",
        "//lib/common:distribution",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/component:exporter",
        "//lib/component:identifier",
        "//lib/component:randomizer",
        "//lib/component:scheduler",
        "//lib/component:storage",
        "//lib/simulation:physical_position",
        "//lib/simulation:physical_connector",
        "//lib/simulation:spawner",
        "//lib/coordination:spreading",
        "//lib/data:field",
        "//lib/data:trace",
        ":slowdistance",
    ],
)

cc_library(
    name = "slowdistance",
    hdrs = ["slowdistance.hpp"],
    srcs = ['slowdistance.cpp'],
    deps = [
        "//lib/common:array",
        "//lib/common:tagged_tuple",
        "//lib/data:field",
        "//lib/data:trace",
    ],
)

cc_test(
    name = "tester",
    srcs = ["tester.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:array",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/component:identifier",
        "//lib/component:storage",
        "//lib/simulation:physical_position",
        "//lib/coordination:spreading",
        "//lib/data:field",
        "//lib/data:trace",
        ":slowdistance",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
