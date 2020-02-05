cc_library(
    name = 'component',
    hdrs = ['component.hpp'],
    srcs = ['component.cpp'],
    deps = [
        "//lib/device:base_component",
        "//lib/device:data_component",
        "//lib/device:multi_component",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'base_component',
    hdrs = ['base_component.hpp'],
    srcs = ['base_component.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'data_component',
    hdrs = ['data_component.hpp'],
    srcs = ['data_component.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:tagged_tuple",
        "//lib/device:base_component",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'multi_component',
    hdrs = ['multi_component.hpp'],
    srcs = ['multi_component.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
        "//lib/device:base_component",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'device',
    hdrs = ['device.hpp'],
    srcs = ['device.cpp'],
    deps = [
        "//lib/common:multitype_map",
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
