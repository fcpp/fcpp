cc_library(
    name = 'graph_connector',
    hdrs = ['graph_connector.hpp'],
    srcs = ['graph_connector.cpp'],
    deps = [
        "//lib/common:option",
        "//lib/common:serialize",
        "//lib/component:base",
        "//lib/data:field",
        "//lib/internal:twin",
        "//lib/option:distribution",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'graph_spawner',
    hdrs = ['graph_spawner.hpp'],
    srcs = ['graph_spawner.cpp'],
    deps = [
        "//lib/component:base",
        "//lib/component:storage",
    ],
    visibility = [
        '//visibility:public',
    ],
)
