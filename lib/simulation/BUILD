cc_library(
    name = 'spawner',
    hdrs = ['spawner.hpp'],
    srcs = ['spawner.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:distribution",
        "//lib/common:tagged_tuple",
        "//lib/common:traits",
        "//lib/component:timer",
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
        "//lib:settings",
        "//lib/common:array",
        "//lib/common:tagged_tuple",
        "//lib/data:field",
    ],
    visibility = [
        '//visibility:public',
    ],
)
