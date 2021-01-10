load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

#
# Libraries
#

cc_library(
    name = "table",
    srcs = ["table.cc"],
    hdrs = ["table.h"],
    deps = ["@com_google_absl//absl/strings",
            "@com_google_absl//absl/status:status",
            "@com_google_absl//absl/status:statusor"]
)

#
# Binaries
#

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [":table",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse", 
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/status:status",
        "@com_google_absl//absl/status:statusor",
    ],
)