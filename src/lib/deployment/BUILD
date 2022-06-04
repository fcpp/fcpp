cc_library(
    name = 'hardware_connector',
    hdrs = ['hardware_connector.hpp'],
    srcs = ['hardware_connector.cpp'],
    deps = [
        "//lib/common:serialize",
        "//lib/component:base",
        "//lib/data:field",
        "//lib/deployment:os",
        "//lib/option:distribution",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'hardware_identifier',
    hdrs = ['hardware_identifier.hpp'],
    srcs = ['hardware_identifier.cpp'],
    deps = [
        "//lib/component:base",
        "//lib/deployment:os",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'hardware_logger',
    hdrs = ['hardware_logger.hpp'],
    srcs = ['hardware_logger.cpp'],
    deps = [
        "//lib/component:base",
        "//lib/component:logger",
        "//lib/deployment:os",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'os',
    hdrs = ['os.hpp'],
    srcs = ['os.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:algorithm",
        "//lib/common:mutex",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'persister',
    hdrs = ['persister.hpp'],
    srcs = ['persister.cpp'],
    deps = [
        "//lib/common:serialize",
        "//lib/component:base",
        "//lib/component:calculus",
    ],
    visibility = [
        '//visibility:public',
    ],
)
