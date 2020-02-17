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

cc_test(
    name = "randomizer",
    srcs = ["randomizer.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:randomizer",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "storage",
    srcs = ["storage.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:storage",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
