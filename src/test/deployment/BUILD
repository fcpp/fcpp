cc_test(
    name = "hardware_connector",
    srcs = ["hardware_connector.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:scheduler",
        "//lib/deployment:hardware_connector",
        "//lib/deployment:hardware_identifier",
        "//test:fake_os",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "hardware_identifier",
    srcs = ["hardware_identifier.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:scheduler",
        "//lib/component:storage",
        "//lib/deployment:hardware_identifier",
        "//test:fake_os",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "hardware_logger",
    srcs = ["hardware_logger.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:scheduler",
        "//lib/component:storage",
        "//lib/deployment:hardware_logger",
        "//lib/deployment:hardware_identifier",
        "//test:fake_os",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)

cc_test(
    name = "persister",
    srcs = ["persister.cpp"],
    deps = [
        "@gtest//:main",
        "//lib/component:scheduler",
        "//lib/component:storage",
        "//lib/coordination:basics",
        "//lib/deployment:hardware_identifier",
        "//lib/deployment:persister",
        "//test:fake_os",
        "//test:helper",
    ],
    copts = ['-Iexternal/gtest/googletest/include/'],
    args = ['--gtest_color=yes'],
    timeout = 'short',
)
