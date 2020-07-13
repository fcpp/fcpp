cc_library(
    name = 'beautify',
    hdrs = ['beautify.hpp'],
    srcs = ['beautify.cpp'],
    deps = [],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'common',
    hdrs = ['common.hpp'],
    srcs = ['common.cpp'],
    deps = [
        "//lib/common:aggregator",
        "//lib/common:algorithm",
        "//lib/common:distribution",
        "//lib/common:multitype_map",
        "//lib/common:mutex",
        "//lib/common:ostream",
        "//lib/common:profiler",
        "//lib/common:random_access_map",
        "//lib/common:sequence",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
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
        "//lib/component:identifier",
        "//lib/component:logger",
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
        "//lib/coordination:election",
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
        "//lib/data:field",
        "//lib/data:tuple",
        "//lib/data:vec",
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
        "//lib:beautify",
        "//lib:common",
        "//lib:component",
        "//lib:coordination",
        "//lib:data",
        "//lib:internal",
        "//lib:settings",
        "//lib:simulation",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'internal',
    hdrs = ['internal.hpp'],
    srcs = ['internal.cpp'],
    deps = [
        "//lib/internal:context",
        "//lib/internal:flat_ptr",
        "//lib/internal:trace",
        "//lib/internal:twin",
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
        "//lib:component",
        "//lib/simulation:batch",
        "//lib/simulation:physical_connector",
        "//lib/simulation:physical_position",
        "//lib/simulation:spawner",
    ],
    visibility = [
        '//visibility:public',
    ],
)
