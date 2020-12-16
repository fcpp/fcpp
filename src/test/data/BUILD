cc_test(
    name = "color",
    srcs = ["color.cpp"],
    deps = [
        "//lib/data:color",
        "@gtest//:main",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "field",
    srcs = ["field.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/data:field",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "tuple",
    srcs = ["tuple.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/data:tuple",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "vec",
    srcs = ["vec.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/data:vec",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
