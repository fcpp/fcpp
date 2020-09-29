cc_test(
    name = "aggregator",
    srcs = ["aggregator.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/option:aggregator",
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
        "//test:helper",
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
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)