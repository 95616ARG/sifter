py_binary(
    name = "letter_analogy",
    srcs = ["letter_analogy.py"],
    deps = [
        ":letter_tactics",
        "//:mapper",
        "//:ts_lib",
        "//:ts_utils",
        "//runtime",
    ],
)

py_binary(
    name = "letter_tactics",
    srcs = ["letter_tactics.py"],
    deps = [
        "//:analogy_utils",
        "//:tactic_utils",
    ],
)

py_test(
    name = "test_letter_analogy",
    size = "small",
    srcs = ["test_letter_analogy.py"],
    deps = [
        ":letter_analogy",
        "@bazel_python//:pytest_helper",
    ],
)
