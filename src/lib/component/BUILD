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
        "//lib/component:base",
        "//lib/internal:context",
        "//lib/internal:trace",
        "//lib/internal:twin",
        "//lib/option:metric",
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
        "//lib/common:algorithm",
        "//lib/common:random_access_map",
        "//lib/component:base",
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
        "//lib/common:plot",
        "//lib/component:base",
        "//lib/option:aggregator",
        "//lib/option:functor",
        "//lib/option:sequence",
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
        "//lib/component:base",
        "//lib/option:distribution",
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
        "//lib/component:base",
        "//lib/option:sequence",
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
        "//lib/component:base",
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
        "//lib/component:base",
        "//lib/data:field",
    ],
    visibility = [
        '//visibility:public',
    ],
)
