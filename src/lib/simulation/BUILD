cc_library(
    name = 'batch',
    hdrs = ['batch.hpp'],
    srcs = ['batch.cpp'],
    deps = [
        "//lib/common:algorithm",
        "//lib/common:option",
        "//lib/common:tagged_tuple",
        "//lib/component:logger",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'displayer',
    hdrs = ['displayer.hpp'],
    srcs = ['displayer.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'simulated_connector',
    hdrs = ['simulated_connector.hpp'],
    srcs = ['simulated_connector.cpp'],
    deps = [
        "//lib/common:option",
        "//lib/common:serialize",
        "//lib/component:base",
        "//lib/data:field",
        "//lib/data:vec",
        "//lib/option:connect",
        "//lib/option:distribution",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'simulated_map',
    hdrs = ['simulated_map.hpp'],
    srcs = ['simulated_map.cpp'],
    deps = [
        "//lib/component:base",
        "//lib/data:color",
        "//lib/data:vec",
        "@stb_image//:main",
    ],
    visibility = [
        '//visibility:public',
    ],
)


cc_library(
    name = 'simulated_positioner',
    hdrs = ['simulated_positioner.hpp'],
    srcs = ['simulated_positioner.cpp'],
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
