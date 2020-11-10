package(default_visibility = ["//:__subpackages__"])

py_library(
    name = "runtime",
    srcs = ["runtime.py"],
    deps = [
        ":cpp_structure",
        ":interactive",
        ":matcher",
        ":production_rule",
    ],
)

py_library(
    name = "cpp_structure",
    srcs = ["cpp_structure.py"],
    deps = [
        ":utils",
    ],
)

py_library(
    name = "production_rule",
    srcs = ["production_rule.py"],
    deps = [
        ":assignment",
        ":pattern",
        ":utils",
    ],
)

py_library(
    name = "interactive",
    srcs = ["interactive.py"],
    deps = [
        ":shadow_input",
    ],
)

py_library(
    name = "shadow_input",
    srcs = ["shadow_input.py"],
    deps = [],
)

py_library(
    name = "utils",
    srcs = ["utils.py"],
    deps = [],
)

py_library(
    name = "assignment",
    srcs = ["assignment.py"],
    deps = [
        ":utils",
    ],
)

py_library(
    name = "pattern",
    srcs = ["pattern.py"],
    deps = [
        ":utils",
    ],
)

py_library(
    name = "matcher",
    srcs = ["matcher.py"],
    deps = [
        ":utils",
    ],
)
