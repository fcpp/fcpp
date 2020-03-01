cc_library(
    name = 'component',
    hdrs = ['component.hpp'],
    srcs = ['component.cpp'],
    deps = [
        "//lib/component:base",
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
        "//lib/coordination:spreading",
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
        "//lib:component",
        "//lib:coordination",
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
