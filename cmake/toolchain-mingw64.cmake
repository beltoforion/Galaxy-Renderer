# CMake toolchain file for cross-compiling to 64 bit Windows with MinGW-w64.
#
# Usage:
#   cmake -B build-win -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake \
#         -DCMAKE_PREFIX_PATH=<prefix with SDL2/SDL2_ttf/GLEW mingw builds>
#
# The build_windows.sh script at the repository root drives the full build
# including the dependencies.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

find_program(CMAKE_C_COMPILER   NAMES ${TOOLCHAIN_PREFIX}-gcc-posix ${TOOLCHAIN_PREFIX}-gcc REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES ${TOOLCHAIN_PREFIX}-g++-posix ${TOOLCHAIN_PREFIX}-g++ REQUIRED)
find_program(CMAKE_RC_COMPILER  NAMES ${TOOLCHAIN_PREFIX}-windres REQUIRED)

set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

# Prefix with cross-built dependencies (SDL2, SDL2_ttf, GLEW, ...).
# Set by build_windows.sh; can also be passed as -DMINGW_DEPS_PREFIX=...
if(NOT DEFINED MINGW_DEPS_PREFIX AND DEFINED ENV{MINGW_DEPS_PREFIX})
    set(MINGW_DEPS_PREFIX "$ENV{MINGW_DEPS_PREFIX}")
endif()
if(MINGW_DEPS_PREFIX)
    list(APPEND CMAKE_FIND_ROOT_PATH "${MINGW_DEPS_PREFIX}")
    list(APPEND CMAKE_PREFIX_PATH "${MINGW_DEPS_PREFIX}")
endif()

# Search programs on the host, headers/libraries only in the target roots
# (CMAKE_PREFIX_PATH entries are treated as target roots as well).
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# Keep the runtime dependencies down to the DLLs we ship: link the GCC
# runtime, libstdc++ and winpthread statically into the executable.
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static-libgcc -static-libstdc++ -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive,-Bdynamic")
