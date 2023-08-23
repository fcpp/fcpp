cc_library(
    name = 'basics',
    hdrs = ['basics.hpp'],
    srcs = ['basics.cpp'],
    deps = [
        "//lib/data:field",
        "//lib/internal:trace",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'collection',
    hdrs = ['collection.hpp'],
    srcs = ['collection.cpp'],
    deps = [
        "//lib/coordination:utils",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'election',
    hdrs = ['election.hpp'],
    srcs = ['election.cpp'],
    deps = [
        "//lib/coordination:utils",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'geometry',
    hdrs = ['geometry.hpp'],
    srcs = ['geometry.cpp'],
    deps = [
        "//lib/coordination:utils",
        "//lib/data:vec",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'spreading',
    hdrs = ['spreading.hpp'],
    srcs = ['spreading.cpp'],
    deps = [
        "//lib/coordination:utils",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'time',
    hdrs = ['time.hpp'],
    srcs = ['time.cpp'],
    deps = [
        "//lib/coordination:utils",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'utils',
    hdrs = ['utils.hpp'],
    srcs = ['utils.cpp'],
    deps = [
        "//lib/common:algorithm",
        "//lib/coordination:basics",
    ],
    visibility = [
        '//visibility:public',
    ],
)
