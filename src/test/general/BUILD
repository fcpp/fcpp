cc_test(
    name = "slowdistance",
    srcs = ["slowdistance.hpp", "slowdistance.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/component:identifier",
        "//lib/component:storage",
        "//lib/simulation:physical_position",
        "//lib/coordination:spreading",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
