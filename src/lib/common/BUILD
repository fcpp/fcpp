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
    name = 'ostream',
    hdrs = ['ostream.hpp'],
    srcs = ['ostream.cpp'],
    deps = [
        "//lib/common:multitype_map",
        "//lib/common:random_access_map",
        "//lib/common:tagged_tuple",
        "//lib/data:tuple",
        "//lib/data:vec",
        "//lib/internal:context",
        "//lib/internal:flat_ptr",
        "//lib/internal:twin",
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
    name = 'random_access_map',
    hdrs = ['random_access_map.hpp'],
    srcs = ['random_access_map.cpp'],
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
