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
    name = 'physics_position',
    hdrs = ['physics_position.hpp'],
    srcs = ['physics_position.cpp'],
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
