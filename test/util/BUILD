cc_test(
    name = "traits",
    srcs = ["traits.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/util:traits",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)