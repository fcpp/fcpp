cc_test(
    name = "context",
    srcs = ["context.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:multitype_map",
        "//lib/data:context",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "field",
    srcs = ["field.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/data:field",
        "//lib/data:tuple",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "trace",
    srcs = ["trace.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/data:trace",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "tuple",
    srcs = ["tuple.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/data:tuple",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
