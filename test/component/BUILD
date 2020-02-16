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
