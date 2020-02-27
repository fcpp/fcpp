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
    name = 'settings',
    hdrs = ['settings.hpp'],
    srcs = ['settings.cpp'],
    visibility = [
        '//visibility:public',
    ],
)
