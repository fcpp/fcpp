cc_library(
    name = 'helper',
    hdrs = ['helper.hpp'],
    srcs = ['helper.cpp'],
    deps = [
        "@gtest//:main",
        "//lib:beautify",
        "//lib/common:ostream",
        "//lib/common:traits",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    visibility = [
        '//visibility:public',
    ],
)
