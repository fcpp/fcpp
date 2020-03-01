cc_test(
    name = "base",
    srcs = ["base.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "calculus",
    srcs = ["calculus.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/data:field",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "exporter",
    srcs = ["exporter.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:aggregator",
        "//lib/common:distribution",
        "//lib/common:sequence",
        "//lib/component:base",
        "//lib/component:exporter",
        "//lib/component:identifier",
        "//lib/component:storage",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "identifier",
    srcs = ["identifier.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:distribution",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/component:identifier",
        "//lib/component:scheduler",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "randomizer",
    srcs = ["randomizer.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:randomizer",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "scheduler",
    srcs = ["scheduler.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:distribution",
        "//lib/common:sequence",
        "//lib/component:base",
        "//lib/component:randomizer",
        "//lib/component:scheduler",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "storage",
    srcs = ["storage.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:storage",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "timer",
    srcs = ["timer.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:timer",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
