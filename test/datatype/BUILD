cc_test(
    name = "exports",
    srcs = ["exports.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/datatype:exports",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
