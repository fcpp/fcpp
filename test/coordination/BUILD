cc_test(
    name = "spreading",
    srcs = ["spreading.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:spreading",
        "//lib/data:field",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
