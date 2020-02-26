cc_test(
    name = "spawner",
    srcs = ["spawner.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:distribution",
        "//lib/common:sequence",
        "//lib/component:base",
        "//lib/component:identifier",
        "//lib/component:storage",
        "//lib/component:timer",
        "//lib/simulation:spawner",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
