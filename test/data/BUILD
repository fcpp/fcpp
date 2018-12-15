cc_test(
    name = "field",
    srcs = ["field.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/data:field",
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
