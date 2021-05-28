cc_library(
    name = 'algorithm',
    hdrs = ['algorithm.hpp'],
    srcs = ['algorithm.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'multitype_map',
    hdrs = ['multitype_map.hpp'],
    srcs = ['multitype_map.cpp'],
    deps = [
        "//lib/common:traits",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'mutex',
    hdrs = ['mutex.hpp'],
    srcs = ['mutex.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'option',
    hdrs = ['option.hpp'],
    srcs = ['option.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'ostream',
    hdrs = ['ostream.hpp'],
    srcs = ['ostream.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:traits",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'plot',
    hdrs = ['plot.hpp'],
    srcs = ['plot.cpp'],
    deps = [
        "//lib/common:mutex",
        "//lib/common:serialize",
        "//lib/common:tagged_tuple",
        "//lib/option:aggregator",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'profiler',
    hdrs = ['profiler.hpp'],
    srcs = ['profiler.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'quaternion',
    hdrs = ['quaternion.hpp'],
    srcs = ['quaternion.cpp'],
    deps = [
        "//lib:settings",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'random_access_map',
    hdrs = ['random_access_map.hpp'],
    srcs = ['random_access_map.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'serialize',
    hdrs = ['serialize.hpp'],
    srcs = ['serialize.cpp'],
    deps = [
        "//lib/internal:trace",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'tagged_tuple',
    hdrs = ['tagged_tuple.hpp'],
    srcs = ['tagged_tuple.cpp'],
    deps = [
        "//lib/common:traits",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'traits',
    hdrs = ['traits.hpp'],
    srcs = ['traits.cpp'],
    visibility = [
        '//visibility:public',
    ],
)
