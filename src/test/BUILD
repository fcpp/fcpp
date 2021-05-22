cc_library(
    name = 'fake_os',
    hdrs = ['fake_os.hpp'],
    srcs = ['fake_os.cpp'],
    deps = [
        "@gtest//:main",
        "//lib/common:mutex",
        "//lib/deployment:os",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'helper',
    hdrs = ['helper.hpp'],
    srcs = ['helper.cpp'],
    deps = [
        "@gtest//:main",
        "//lib/common:ostream",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'test_net',
    hdrs = ['test_net.hpp'],
    srcs = ['test_net.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:mutex",
        "//lib/common:tagged_tuple",
        "//lib/data:vec",
        "//test:helper"
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    visibility = [
        '//visibility:public',
    ],
)
