py_binary(
    name = "turing_machine",
    srcs = ["turing_machine.py"],
    deps = [
        "//:ts_lib",
        "//:ts_utils",
        "//runtime",
    ],
)

py_test(
    name = "test_turing_machine",
    size = "small",
    srcs = ["test_turing_machine.py"],
    deps = [
        ":turing_machine",
        "@bazel_python//:pytest_helper",
    ],
)
