cc_test(
    name = "algorithm",
    srcs = ["algorithm.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:algorithm",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "multitype_map",
    srcs = ["multitype_map.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:multitype_map",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "mutex",
    srcs = ["mutex.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:algorithm",
        "//lib/common:mutex",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "ostream",
    srcs = ["ostream.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:ostream",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "random_access_map",
    srcs = ["random_access_map.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:random_access_map",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "tagged_tuple",
    srcs = ["tagged_tuple.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:tagged_tuple",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "traits",
    srcs = ["traits.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:traits",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
