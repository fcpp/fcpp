cc_binary(
    name = "runner",
    srcs = ["runner.cpp"],
    deps = [
        "//lib:fcpp",
        ":collection_compare",
    ],
)

cc_library(
    name = "collection_compare",
    hdrs = ["collection_compare.hpp"],
    srcs = ['collection_compare.cpp'],
    deps = [
        "//lib/coordination:collection",
        "//lib/coordination:geometry",
        "//lib/coordination:spreading",
    ],
)
