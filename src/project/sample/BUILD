cc_test(
    name = "slowdistancetest",
    srcs = ["slowdistancetest.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:array",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/component:identifier",
        "//lib/component:storage",
        "//lib/simulation:physical_position",
        "//lib/coordination:spreading",
        "//lib/data:field",
        "//lib/data:trace",
        "//project/sample:slowdistance",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_library(
    name = "slowdistance",
    hdrs = ["slowdistance.hpp"],
    srcs = ['slowdistance.cpp'],
    deps = [
        "//lib/common:array",
        "//lib/common:tagged_tuple",
        "//lib/data:field",
        "//lib/data:trace",
    ],
)
