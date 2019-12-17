cc_test(
    name = "data_component",
    srcs = ["data_component.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/device:data_component",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "multi_component",
    srcs = ["multi_component.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/device:multi_component",
        "//lib/device:data_component",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
