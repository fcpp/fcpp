cc_library(
    name = 'base',
    hdrs = ['base.hpp'],
    srcs = ['base.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:mutex",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'randomizer',
    hdrs = ['randomizer.hpp'],
    srcs = ['randomizer.cpp'],
    deps = [
        "//lib/common:distribution",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'scheduler',
    hdrs = ['scheduler.hpp'],
    srcs = ['scheduler.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'storage',
    hdrs = ['storage.hpp'],
    srcs = ['storage.cpp'],
    deps = [
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'timer',
    hdrs = ['timer.hpp'],
    srcs = ['timer.cpp'],
    deps = [
        "//lib:settings",
        "//lib/data:field",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)
