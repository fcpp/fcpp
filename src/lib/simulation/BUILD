cc_library(
    name = 'batch',
    hdrs = ['batch.hpp'],
    srcs = ['batch.cpp'],
    deps = [
        "//lib/common:algorithm",
        "//lib/common:tagged_tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'physical_connector',
    hdrs = ['physical_connector.hpp'],
    srcs = ['physical_connector.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:distribution",
        "//lib/common:mutex",
        "//lib/common:profiler",
        "//lib/common:tagged_tuple",
        "//lib/data:vec",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'physical_position',
    hdrs = ['physical_position.hpp'],
    srcs = ['physical_position.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:profiler",
        "//lib/common:tagged_tuple",
        "//lib/data:field",
        "//lib/data:vec",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'spawner',
    hdrs = ['spawner.hpp'],
    srcs = ['spawner.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:distribution",
        "//lib/common:profiler",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
    ],
    visibility = [
        '//visibility:public',
    ],
)
