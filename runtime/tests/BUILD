py_test(
    name = "test_cpp_structure",
    size = "small",
    srcs = ["test_cpp_structure.py"],
    deps = [
        "//:ts_lib",
        "//runtime:cpp_structure",
        "@bazel_python//:pytest_helper",
    ],
)

py_test(
    name = "test_utils",
    size = "small",
    srcs = ["test_utils.py"],
    deps = [
        "//runtime:utils",
        "@bazel_python//:pytest_helper",
    ],
)
