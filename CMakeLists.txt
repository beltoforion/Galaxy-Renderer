cmake_minimum_required(VERSION 3.16)

project(galaxy_renderer
    VERSION 1.2.0
    DESCRIPTION "Spiral galaxy renderer based on the density wave theory (Dear ImGui UI)"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

# OpenGL (+ GLU where available)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# GLEW
find_package(GLEW REQUIRED)

# SDL2
find_package(SDL2 REQUIRED)

# SDL2_ttf: prefer a CMake config; fall back to pkg-config on distros that
# ship SDL2_ttf without one.
find_package(SDL2_ttf QUIET)
if(NOT SDL2_ttf_FOUND AND NOT TARGET SDL2_ttf::SDL2_ttf)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2_TTF REQUIRED IMPORTED_TARGET SDL2_ttf)
endif()

# glm (header-only, vendored)
add_library(glm_headers INTERFACE)
target_include_directories(glm_headers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm)

# ---------------------------------------------------------------------------
# Dear ImGui (fetched, built as a small static library)
# ---------------------------------------------------------------------------
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.5
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends)
# The OpenGL3 backend ships its own GL loader (since 1.82); it coexists with
# the application's GLEW usage, so no IMGUI_IMPL_OPENGL_LOADER_* define needed.
target_link_libraries(imgui PUBLIC SDL2::SDL2)

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------
add_executable(galaxy_renderer
    CumulativeDistributionFunction.cpp
    Galaxy.cpp
    GalaxyWnd.cpp
    Helper.cpp
    main.cpp
    SDLWnd.cpp
    TextBuffer.cpp
    VideoRecorder.cpp)

target_include_directories(galaxy_renderer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(galaxy_renderer PRIVATE
    glm_headers
    imgui
    GLEW::GLEW
    OpenGL::GL
    SDL2::SDL2)

# GLU is optional (used by some GL contexts); link it when present.
if(TARGET OpenGL::GLU)
    target_link_libraries(galaxy_renderer PRIVATE OpenGL::GLU)
endif()

# SDL2main on platforms that need it (Windows)
if(TARGET SDL2::SDL2main)
    target_link_libraries(galaxy_renderer PRIVATE SDL2::SDL2main)
endif()

# SDL2_ttf via config target or pkg-config fallback
if(TARGET SDL2_ttf::SDL2_ttf)
    target_link_libraries(galaxy_renderer PRIVATE SDL2_ttf::SDL2_ttf)
elseif(TARGET PkgConfig::SDL2_TTF)
    target_link_libraries(galaxy_renderer PRIVATE PkgConfig::SDL2_TTF)
endif()

if(MSVC)
    target_compile_definitions(galaxy_renderer PRIVATE
        _USE_MATH_DEFINES NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()

# ---------------------------------------------------------------------------
# Assets: fonts are loaded via relative "assets/..." paths, so copy the
# assets folder next to the binary after every build.
# ---------------------------------------------------------------------------
add_custom_command(TARGET galaxy_renderer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:galaxy_renderer>/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/presets
        $<TARGET_FILE_DIR:galaxy_renderer>/presets
    COMMENT "Copying assets and presets next to the galaxy_renderer binary")

# ---------------------------------------------------------------------------
# Dist: stage a portable, runnable deliverable in <repo>/dist
# (binary + assets + presets; run the app from inside that folder)
# ---------------------------------------------------------------------------
set(DIST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dist)
add_custom_target(dist
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DIST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:galaxy_renderer> ${DIST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets ${DIST_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/presets ${DIST_DIR}/presets
    DEPENDS galaxy_renderer
    COMMENT "Staging portable dist folder in ${DIST_DIR}")

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS galaxy_renderer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY assets presets DESTINATION ${CMAKE_INSTALL_BINDIR})
