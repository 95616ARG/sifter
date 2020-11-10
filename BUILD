load("@bazel_python//:bazel_python.bzl", "bazel_python_coverage_report", "bazel_python_interpreter")

bazel_python_interpreter(
    name = "bazel_python_venv",
    python_version = "3.7.4",
    requirements_file = "requirements.txt",
    run_after_pip = """
        pushd ts_cpp
        python3 setup.py install || exit 1
        popd
    """,
    run_after_pip_srcs = glob(["ts_cpp/*"]),
    visibility = ["//:__subpackages__"],
)

bazel_python_coverage_report(
    name = "coverage_report",
    code_paths = [
        "*.py",
        "runtime/*.py",
    ],
    test_paths = [
        "tests/*",
        "runtime/tests/*",
        "examples/*/test_*",
    ],
)

py_library(
    name = "ts_lib",
    srcs = ["ts_lib.py"],
    visibility = ["//visibility:public"],
    deps = [],
)

py_library(
    name = "ts_utils",
    srcs = ["ts_utils.py"],
    visibility = ["//visibility:public"],
    deps = [],
)

py_library(
    name = "mapper",
    srcs = ["mapper.py"],
    visibility = ["//visibility:public"],
    deps = [":ts_utils"],
)

py_library(
    name = "tactic_utils",
    srcs = ["tactic_utils.py"],
    visibility = ["//visibility:public"],
    deps = [
        ":ts_utils",
        "//runtime:matcher",
    ],
)

py_library(
    name = "analogy_utils",
    srcs = ["analogy_utils.py"],
    visibility = ["//visibility:public"],
    deps = [
        ":tactic_utils",
    ],
)
