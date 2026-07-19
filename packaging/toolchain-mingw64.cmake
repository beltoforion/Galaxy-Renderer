# CMake toolchain file for cross-compiling to 64-bit Windows with MinGW-w64.
# Used by packaging/build-windows.sh; needs the Ubuntu/Debian package
# g++-mingw-w64-x86-64 (or an equivalent x86_64-w64-mingw32 toolchain).

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

# Prefer the -posix flavour (win32 thread model lacks std::thread & friends).
find_program(MINGW_CXX NAMES ${TOOLCHAIN_PREFIX}-g++-posix ${TOOLCHAIN_PREFIX}-g++ REQUIRED)
find_program(MINGW_CC  NAMES ${TOOLCHAIN_PREFIX}-gcc-posix ${TOOLCHAIN_PREFIX}-gcc REQUIRED)
set(CMAKE_C_COMPILER   ${MINGW_CC})
set(CMAKE_CXX_COMPILER ${MINGW_CXX})
set(CMAKE_RC_COMPILER  ${TOOLCHAIN_PREFIX}-windres)

# Callers (build-windows.sh) pre-seed CMAKE_FIND_ROOT_PATH with the
# directories of the downloaded Windows dependencies; keep those entries.
list(APPEND CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
