cc_test(
    name = "collection",
    srcs = ["collection.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:collection",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "spreading",
    srcs = ["spreading.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/coordination:spreading",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
