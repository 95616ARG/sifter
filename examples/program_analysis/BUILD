py_binary(
    name = "api_migration",
    srcs = ["api_migration.py"],
    deps = [
        ":analyzelib",
        ":lazy_structure",
        "//:tactic_utils",
        "//examples/program_analysis/ui:serve",
        "//runtime",
    ],
)

py_binary(
    name = "transform_learning",
    srcs = ["transform_learning.py"],
    deps = [
        ":analyzelib",
        ":lazy_structure",
        "//:tactic_utils",
        "//examples/program_analysis/ui:serve",
        "//runtime",
    ],
)

py_binary(
    name = "program_understanding",
    srcs = ["program_understanding.py"],
    deps = [
        ":analyzelib",
        ":lazy_structure",
        "//:tactic_utils",
        "//examples/program_analysis/ui:serve",
        "//runtime",
    ],
)

py_library(
    name = "lazy_structure",
    srcs = ["lazy_structure.py"],
    deps = [
        "//:mapper",
        "//:tactic_utils",
        "//:ts_lib",
        "//:ts_utils",
        "//examples/program_analysis/ui:serve",
        "//runtime",
    ],
)

py_library(
    name = "analyzelib",
    srcs = ["analyzelib.py"],
    deps = [
        "//:analogy_utils",
        "//:mapper",
        "//:tactic_utils",
        "//:ts_lib",
        "//:ts_utils",
        "//examples/program_analysis/ui:serve",
        "//runtime",
    ],
)
