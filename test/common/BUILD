cc_test(
    name = "multitype_map",
    srcs = ["multitype_map.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:multitype_map",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "traits",
    srcs = ["traits.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:traits",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "twin",
    srcs = ["twin.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/common:twin",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
