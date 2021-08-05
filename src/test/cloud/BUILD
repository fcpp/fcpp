cc_test(
    name = "graph_connector",
    srcs = ["graph_connector.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:identifier",
        "//lib/component:scheduler",
        "//lib/cloud:graph_connector",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "graph_spawner",
    srcs = ["graph_spawner.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:base",
        "//lib/component:identifier",
        "//lib/component:storage",
        "//lib/component:timer",
        "//lib/cloud:graph_connector",
        "//lib/cloud:graph_spawner",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
