cc_library(
    name = "Constants",
    hdrs = ["Constants.h"],
)

cc_library(
    name = "CumulativeDistributionFunction",
    srcs = ["CumulativeDistributionFunction.cpp"],
    hdrs = ["CumulativeDistributionFunction.h"],
)

cc_library(
    name = "FastMath",
    srcs = ["FastMath.cpp"],
    hdrs = ["FastMath.h"],
)

cc_library(
    name = "Galaxy",
    srcs = ["Galaxy.cpp"],
    hdrs = ["Galaxy.h"],
    deps = [
        ":Constants",
        ":CumulativeDistributionFunction",
        ":FastMath",
        ":Star",
        ":Vector",
    ],
)

cc_library(
    name = "GalaxyProp",
    srcs = ["GalaxyProp.cpp"],
    hdrs = ["GalaxyProp.h"],
)

cc_library(
    name = "Intensity",
    hdrs = ["Intensity.h"],
)

cc_library(
    name = "NBodyWnd",
    srcs = ["NBodyWnd.cpp"],
    hdrs = ["NBodyWnd.h"],
    deps = [
        ":Constants",
        ":FastMath",
        ":Galaxy",
        ":OrbitCalculator",
        ":SDLWnd",
        ":Star",
        "//third_party/specrend",
    ],
)

cc_library(
    name = "OrbitCalculator",
    srcs = ["OrbitCalculator.cpp"],
    hdrs = ["OrbitCalculator.h"],
    deps = [
        ":Constants",
        ":Vector",
    ],
)

# Requires libsdl
# TODO(Frank): Upgrade to 2.0
cc_library(
    name = "SDLWnd",
    srcs = ["SDLWnd.cpp"],
    hdrs = ["SDLWnd.h"],
    linkopts = [
        "-lX11",
        "-lGLU",
        "-lGL",
        "-L/usr/lib/x86_64-linux-gnu",
        "-lSDL",
    ],
    deps = [":Vector"],
)

cc_library(
    name = "Star",
    srcs = ["Star.cpp"],
    hdrs = ["Star.h"],
    deps = [
        ":Constants",
        ":OrbitCalculator",
    ],
)

cc_library(
    name = "Types",
    srcs = ["Types.cpp"],
    hdrs = ["Types.h"],
)

cc_library(
    name = "Vector",
    srcs = ["Vector.cpp"],
    hdrs = ["Vector.h"],
)

cc_binary(
    name = "Galaxy-Renderer",
    srcs = ["main.cpp"],
    data = [":texture"],
    deps = [
        ":NBodyWnd",
    ],
)

# TODO(Frank): Texture in its own folder
filegroup(
    name = "texture",
    srcs = glob(["*.bmp"]),
)
