py_test(
    name = "test_ts_lib",
    size = "small",
    srcs = ["test_ts_lib.py"],
    deps = [
        "//:ts_lib",
        "@bazel_python//:pytest_helper",
    ],
)

py_test(
    name = "test_ts_utils",
    size = "small",
    srcs = ["test_ts_utils.py"],
    deps = [
        "//:ts_lib",
        "//:ts_utils",
        "@bazel_python//:pytest_helper",
    ],
)
