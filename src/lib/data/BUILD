cc_library(
    name = 'context',
    hdrs = ['context.hpp'],
    srcs = ['context.cpp'],
    deps = [
        "//lib/common:flat_ptr",
        "//lib/common:multitype_map",
        "//lib/common:traits",
        "//lib/data:field",
        "//lib/data:trace",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'field',
    hdrs = ['field.hpp'],
    srcs = ['field.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:traits",
    ],
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
