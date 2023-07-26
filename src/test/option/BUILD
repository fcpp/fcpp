cc_test(
    name = "aggregator",
    srcs = ["aggregator.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/option:aggregator",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "connect",
    srcs = ["connect.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/option:connect",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "distribution",
    srcs = ["distribution.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/option:distribution",
        "//lib/data:color",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "filter",
    srcs = ["filter.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/option:filter",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "functor",
    srcs = ["functor.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/option:distribution",
        "//lib/option:functor",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "metric",
    srcs = ["metric.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/data:vec",
        "//lib/option:metric",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "sequence",
    srcs = ["sequence.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/option:sequence",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
