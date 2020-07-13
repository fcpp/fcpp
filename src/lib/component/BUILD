cc_library(
    name = 'base',
    hdrs = ['base.hpp'],
    srcs = ['base.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:mutex",
        "//lib/common:profiler",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'calculus',
    hdrs = ['calculus.hpp'],
    srcs = ['calculus.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:multitype_map",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
        "//lib/common:twin",
        "//lib/data:context",
        "//lib/data:field",
        "//lib/data:trace",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'identifier',
    hdrs = ['identifier.hpp'],
    srcs = ['identifier.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:algorithm",
        "//lib/common:mutex",
        "//lib/common:random_access_map",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'logger',
    hdrs = ['logger.hpp'],
    srcs = ['logger.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:algorithm",
        "//lib/common:distribution",
        "//lib/common:mutex",
        "//lib/common:profiler",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
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
        "//lib/common:distribution",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
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
