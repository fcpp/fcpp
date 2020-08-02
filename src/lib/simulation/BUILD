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
        "//lib/component:base",
        "//lib/data:vec",
        "//lib/option:connect",
        "//lib/option:distribution",
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
        "//lib/component:base",
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
        "//lib/component:base",
        "//lib/option:sequence",
    ],
    visibility = [
        '//visibility:public',
    ],
)
