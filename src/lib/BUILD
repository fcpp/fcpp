cc_library(
    name = 'beautify',
    hdrs = ['beautify.hpp'],
    srcs = ['beautify.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'cloud',
    hdrs = ['cloud.hpp'],
    srcs = ['cloud.cpp'],
    deps = [
        "//lib:component",
        "//lib/cloud:graph_connector",
        "//lib/cloud:graph_spawner",
        "//lib/simulation:displayer",
        "//lib/simulation:simulated_positioner",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'common',
    hdrs = ['common.hpp'],
    srcs = ['common.cpp'],
    deps = [
        "//lib/common:algorithm",
        "//lib/common:multitype_map",
        "//lib/common:mutex",
        "//lib/common:option",
        "//lib/common:ostream",
        "//lib/common:profiler",
        "//lib/common:random_access_map",
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
        "//lib/coordination:basics",
        "//lib/coordination:collection",
        "//lib/coordination:election",
        "//lib/coordination:geometry",
        "//lib/coordination:spreading",
        "//lib/coordination:time",
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
        "//lib/data:bloom",
        "//lib/data:color",
        "//lib/data:field",
        "//lib/data:hyperloglog",
        "//lib/data:ordered",
        "//lib/data:shape",
        "//lib/data:tuple",
        "//lib/data:vec",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'deployment',
    hdrs = ['deployment.hpp'],
    srcs = ['deployment.cpp'],
    deps = [
        "//lib:component",
        "//lib/deployment:hardware_connector",
        "//lib/deployment:hardware_identifier",
        "//lib/deployment:hardware_logger",
        "//lib/deployment:os",
        "//lib/deployment:persister",
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
        "//lib:deployment",
        "//lib:internal",
        "//lib:option",
        "//lib:settings",
        "//lib:simulation",
        "//lib:cloud",
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
    name = 'option',
    hdrs = ['option.hpp'],
    srcs = ['option.cpp'],
    deps = [
        "//lib/option:aggregator",
        "//lib/option:connect",
        "//lib/option:distribution",
        "//lib/option:filter",
        "//lib/option:functor",
        "//lib/option:metric",
        "//lib/option:sequence",
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
        "//lib/simulation:displayer",
        "//lib/simulation:simulated_connector",
        "//lib/simulation:simulated_positioner",
        "//lib/simulation:simulated_map",
        "//lib/simulation:spawner",
    ],
    visibility = [
        '//visibility:public',
    ],
)
