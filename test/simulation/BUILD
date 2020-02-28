cc_test(
    name = "physics_position",
    srcs = ["physics_position.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:distribution",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/component:scheduler",
        "//lib/data:field",
        "//lib/simulation:physics_position",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "spawner",
    srcs = ["spawner.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:distribution",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
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
