cc_library(
    name = 'component',
    hdrs = ['component.hpp'],
    srcs = ['component.cpp'],
    deps = [
        "//lib/device:data_component",
        "//lib/device:multi_component",
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
        "//lib/device:multi_component",
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
    ],
    visibility = [
        '//visibility:public',
    ],
)
