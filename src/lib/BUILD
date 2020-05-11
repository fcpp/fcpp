cc_library(
    name = 'common',
    hdrs = ['common.hpp'],
    srcs = ['common.cpp'],
    deps = [
        "//lib/common:aggregator",
        "//lib/common:algorithm",
        "//lib/common:array",
        "//lib/common:distribution",
        "//lib/common:flat_ptr",
        "//lib/common:multitype_map",
        "//lib/common:mutex",
        "//lib/common:ostream",
        "//lib/common:random_access_map",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
        "//lib/common:twin",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'component',
    hdrs = ['component.hpp'],
    srcs = ['component.cpp'],
    deps = [
        "//lib/component:base",
        "//lib/component:calculus",
        "//lib/component:exporter",
        "//lib/component:identifier",
        "//lib/component:randomizer",
        "//lib/component:scheduler",
        "//lib/component:storage",
        "//lib/component:timer",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'coordination',
    hdrs = ['coordination.hpp'],
    srcs = ['coordination.cpp'],
    deps = [
        "//lib/coordination:collection",
        "//lib/coordination:geometry",
        "//lib/coordination:spreading",
        "//lib/coordination:utils",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'data',
    hdrs = ['data.hpp'],
    srcs = ['data.cpp'],
    deps = [
        "//lib/data:context",
        "//lib/data:field",
        "//lib/data:trace",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'fcpp',
    hdrs = ['fcpp.hpp'],
    srcs = ['fcpp.cpp'],
    deps = [
        "//lib:common",
        "//lib:component",
        "//lib:coordination",
        "//lib:data",
        "//lib:settings",
        "//lib:simulation",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'settings',
    hdrs = ['settings.hpp'],
    srcs = ['settings.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'simulation',
    hdrs = ['simulation.hpp'],
    srcs = ['simulation.cpp'],
    deps = [
        "//lib/simulation:physical_connector",
        "//lib/simulation:physical_position",
        "//lib/simulation:spawner",
    ],
    visibility = [
        '//visibility:public',
    ],
)
