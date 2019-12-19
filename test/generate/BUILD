cc_test(
    name = "distribution",
    srcs = ["distribution.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/generate:distribution",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "event",
    srcs = ["event.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/generate:distribution",
        "//lib/generate:event",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
