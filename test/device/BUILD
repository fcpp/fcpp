cc_test(
    name = "device",
    srcs = ["device.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/device:device",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
