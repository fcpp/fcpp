cc_library(
    name = 'spawner',
    hdrs = ['spawner.hpp'],
    srcs = ['spawner.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:distribution",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
        "//lib/component:base",
        "//lib/component:timer",
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
        "//lib/common:array",
        "//lib/common:distribution",
        "//lib/common:mutex",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
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
        "//lib/common:array",
        "//lib/common:tagged_tuple",
        "//lib/component:base",
        "//lib/data:field",
    ],
    visibility = [
        '//visibility:public',
    ],
)
