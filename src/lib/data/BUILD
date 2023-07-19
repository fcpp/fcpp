cc_library(
    name = 'bloom',
    hdrs = ['bloom.hpp'],
    srcs = ['bloom.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'color',
    hdrs = ['color.hpp'],
    srcs = ['color.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'field',
    hdrs = ['field.hpp'],
    srcs = ['field.cpp'],
    deps = [
        "//lib:settings",
        "//lib/common:serialize",
        "//lib/data:tuple",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = "hyperloglog",
    hdrs = ["hyperloglog.hpp"],
    srcs = ["hyperloglog.cpp"],
    deps = [
        "//lib:settings",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = "ordered",
    hdrs = ["ordered.hpp"],
    srcs = ["ordered.cpp"],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'shape',
    hdrs = ['shape.hpp'],
    srcs = ['shape.cpp'],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'tuple',
    hdrs = ['tuple.hpp'],
    srcs = ['tuple.cpp'],
    deps = [
        "//lib/common:traits",
    ],
    visibility = [
        '//visibility:public',
    ],
)

cc_library(
    name = 'vec',
    hdrs = ['vec.hpp'],
    srcs = ['vec.cpp'],
    deps = [
        "//lib:settings",
    ],
    visibility = [
        '//visibility:public',
    ],
)
