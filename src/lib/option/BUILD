cc_library(
    name = 'aggregator',
    hdrs = ['aggregator.hpp'],
    srcs = ['aggregator.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:algorithm",
        "//lib/common:tagged_tuple",
        "//lib/option:filter",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'connect',
    hdrs = ['connect.hpp'],
    srcs = ['connect.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:tagged_tuple",
        "//lib/data:vec",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'distribution',
    hdrs = ['distribution.hpp'],
    srcs = ['distribution.cpp'],
    deps = [
        "//lib/common:tagged_tuple",
        "//lib/data:vec",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'filter',
    hdrs = ['filter.hpp'],
    srcs = ['filter.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'functor',
    hdrs = ['functor.hpp'],
    srcs = ['functor.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'metric',
    hdrs = ['metric.hpp'],
    srcs = ['metric.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:option",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'sequence',
    hdrs = ['sequence.hpp'],
    srcs = ['sequence.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:quaternion",
        "//lib/option:distribution",
    ],
    visibility = [
        '//visibility:public',
    ],
)
