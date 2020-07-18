cc_test(
    name = "context",
    srcs = ["context.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:multitype_map",
        "//lib/internal:context",
        "//test:test_net",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "flat_ptr",
    srcs = ["flat_ptr.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/internal:flat_ptr",
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
        "//lib/internal:trace",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "twin",
    srcs = ["twin.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/internal:twin",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
