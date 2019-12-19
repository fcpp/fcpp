cc_library(
    name = 'distribution',
    hdrs = ['distribution.hpp'],
    srcs = ['distribution.cpp'],
    deps = [
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'event',
    hdrs = ['event.hpp'],
    srcs = ['event.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:traits",
        "//lib/generate:distribution",
    ],
    visibility = [
        '//visibility:public',
    ],
)
