cc_library(
    name = 'context',
    hdrs = ['context.hpp'],
    srcs = ['context.cpp'],
    deps = [
        "//lib/common:multitype_map",
        "//lib/data:field",
        "//lib/internal:flat_ptr",
        "//lib/internal:trace",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'flat_ptr',
    hdrs = ['flat_ptr.hpp'],
    srcs = ['flat_ptr.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'trace',
    hdrs = ['trace.hpp'],
    srcs = ['trace.cpp'],
    deps = [
        "//lib:settings",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'twin',
    hdrs = ['twin.hpp'],
    srcs = ['twin.cpp'],
    visibility = [
        '//visibility:public',
    ],
)
