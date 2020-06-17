cc_binary(
    name = "binary",
    srcs = ["binary.cpp"],
    deps = [
        "//lib/common:ostream",
        "//lib/common:tagged_tuple",
    ],
)

cc_library(
    name = "library",
    hdrs = ["library.hpp"],
    srcs = ['library.cpp'],
)

cc_library(
    name = "libinary",
    srcs = ["library.hpp"],
)

cc_test(
    name = "test",
    srcs = ["test.cpp"],
    deps = [
        "@gtest//:main",
        ":library",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
