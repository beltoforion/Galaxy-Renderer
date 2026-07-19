#!/usr/bin/env bash
#
# Cross-compile the Windows (x64) binaries of galaxy_renderer on Linux with
# MinGW-w64. Produces a portable folder in dist-win/ containing
# galaxy_renderer.exe, the required DLLs, assets and presets.
#
# Prerequisites (Debian/Ubuntu):
#   sudo apt install cmake make g++-mingw-w64-x86-64-posix curl
#
# Usage:
#   ./build_windows.sh            # build dependencies (once) + application
#   ./build_windows.sh clean      # remove all Windows build output
#
# The dependencies (SDL2, FreeType, SDL2_ttf, GLEW) are downloaded as source
# tarballs and cross-compiled once into build-win-deps/prefix; subsequent runs
# only rebuild the application.

set -euo pipefail

SDL2_VER=2.32.10
TTF_VER=2.24.0
FT_VER=2.14.2
GLEW_VER=2.1.0

ROOT="$(cd "$(dirname "$0")" && pwd)"
DEPS="$ROOT/build-win-deps"
PREFIX="$DEPS/prefix"
BUILD="$ROOT/build-win"
DIST="$ROOT/dist-win"
TOOLCHAIN="$ROOT/cmake/toolchain-mingw64.cmake"
JOBS="$(nproc)"

if [[ "${1:-}" == "clean" ]]; then
    rm -rf "$DEPS" "$BUILD" "$DIST"
    echo "Removed $DEPS, $BUILD and $DIST"
    exit 0
fi

# ---------------------------------------------------------------------------
# Sanity checks
# ---------------------------------------------------------------------------
CC_BIN="$(command -v x86_64-w64-mingw32-gcc-posix || command -v x86_64-w64-mingw32-gcc || true)"
if [[ -z "$CC_BIN" ]]; then
    echo "error: MinGW-w64 cross compiler not found." >&2
    echo "       Install it with: sudo apt install g++-mingw-w64-x86-64-posix" >&2
    exit 1
fi

# ---------------------------------------------------------------------------
# Download helper: try each URL until one delivers a real tarball
# ---------------------------------------------------------------------------
fetch() { # fetch <output-file> <url> [url...]
    local out="$1"; shift
    [[ -s "$out" ]] && return 0
    local url
    for url in "$@"; do
        echo "Downloading $(basename "$out") from $url"
        if curl -fsSL --retry 3 -o "$out.part" "$url" && tar tf "$out.part" >/dev/null 2>&1; then
            mv "$out.part" "$out"
            return 0
        fi
        rm -f "$out.part"
    done
    echo "error: could not download $(basename "$out")" >&2
    exit 1
}

mkdir -p "$DEPS/src"
cd "$DEPS/src"

fetch "sdl2-$SDL2_VER.tar.gz" \
    "https://github.com/libsdl-org/SDL/releases/download/release-$SDL2_VER/SDL2-$SDL2_VER.tar.gz" \
    "https://www.libsdl.org/release/SDL2-$SDL2_VER.tar.gz" \
    "http://archive.ubuntu.com/ubuntu/pool/main/libs/libsdl2/libsdl2_$SDL2_VER+dfsg.orig.tar.gz"

fetch "freetype-$FT_VER.tar.xz" \
    "https://download.savannah.gnu.org/releases/freetype/freetype-$FT_VER.tar.xz" \
    "http://archive.ubuntu.com/ubuntu/pool/main/f/freetype/freetype_$FT_VER+dfsg.orig.tar.xz"

fetch "sdl2_ttf-$TTF_VER.tar.gz" \
    "https://github.com/libsdl-org/SDL_ttf/releases/download/release-$TTF_VER/SDL2_ttf-$TTF_VER.tar.gz" \
    "https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-$TTF_VER.tar.gz" \
    "http://archive.ubuntu.com/ubuntu/pool/universe/libs/libsdl2-ttf/libsdl2-ttf_$TTF_VER+dfsg.orig.tar.gz"

fetch "glew-$GLEW_VER.tgz" \
    "https://github.com/nigels-com/glew/releases/download/glew-$GLEW_VER/glew-$GLEW_VER.tgz" \
    "https://downloads.sourceforge.net/project/glew/glew/$GLEW_VER/glew-$GLEW_VER.tgz"

export MINGW_DEPS_PREFIX="$PREFIX"

# ---------------------------------------------------------------------------
# SDL2 (shared)
# ---------------------------------------------------------------------------
if [[ ! -e "$PREFIX/.stamp-sdl2-$SDL2_VER" ]]; then
    echo "=== Building SDL2 $SDL2_VER ==="
    tar xf "sdl2-$SDL2_VER.tar.gz"
    cmake -S "SDL2-$SDL2_VER" -B sdl2-build \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST=OFF
    cmake --build sdl2-build -j"$JOBS"
    cmake --install sdl2-build
    touch "$PREFIX/.stamp-sdl2-$SDL2_VER"
fi

# ---------------------------------------------------------------------------
# FreeType (static, linked into SDL2_ttf.dll)
# ---------------------------------------------------------------------------
if [[ ! -e "$PREFIX/.stamp-freetype-$FT_VER" ]]; then
    echo "=== Building FreeType $FT_VER ==="
    tar xf "freetype-$FT_VER.tar.xz"
    cmake -S "freetype-$FT_VER" -B ft-build \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DBUILD_SHARED_LIBS=OFF \
        -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_PNG=ON \
        -DFT_DISABLE_BROTLI=ON -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_ZLIB=ON
    cmake --build ft-build -j"$JOBS"
    cmake --install ft-build
    touch "$PREFIX/.stamp-freetype-$FT_VER"
fi

# ---------------------------------------------------------------------------
# SDL2_ttf (shared, against our FreeType)
# ---------------------------------------------------------------------------
if [[ ! -e "$PREFIX/.stamp-sdl2_ttf-$TTF_VER" ]]; then
    echo "=== Building SDL2_ttf $TTF_VER ==="
    tar xf "sdl2_ttf-$TTF_VER.tar.gz"
    cmake -S "SDL2_ttf-$TTF_VER" -B ttf-build \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$PREFIX" \
        -DBUILD_SHARED_LIBS=ON \
        -DSDL2TTF_VENDORED=OFF -DSDL2TTF_HARFBUZZ=OFF -DSDL2TTF_SAMPLES=OFF
    cmake --build ttf-build -j"$JOBS"
    cmake --install ttf-build
    touch "$PREFIX/.stamp-sdl2_ttf-$TTF_VER"
fi

# ---------------------------------------------------------------------------
# GLEW (shared; built like upstream's Makefile.mingw: -nostdlib)
# ---------------------------------------------------------------------------
if [[ ! -e "$PREFIX/.stamp-glew-$GLEW_VER" ]]; then
    echo "=== Building GLEW $GLEW_VER ==="
    tar xf "glew-$GLEW_VER.tgz"
    ( cd "glew-$GLEW_VER"
      "$CC_BIN" -O2 -DGLEW_BUILD -Iinclude -c src/glew.c -o glew.o
      "$CC_BIN" -shared -nostdlib -o glew32.dll glew.o \
          -lopengl32 -lgdi32 -luser32 -lkernel32 \
          -Wl,--out-implib,libglew32.dll.a )
    mkdir -p "$PREFIX/include/GL" "$PREFIX/lib" "$PREFIX/bin"
    cp "glew-$GLEW_VER/include/GL/glew.h" "glew-$GLEW_VER/include/GL/wglew.h" "$PREFIX/include/GL/"
    cp "glew-$GLEW_VER/libglew32.dll.a" "$PREFIX/lib/"
    cp "glew-$GLEW_VER/glew32.dll" "$PREFIX/bin/"
    touch "$PREFIX/.stamp-glew-$GLEW_VER"
fi

# ---------------------------------------------------------------------------
# Application
# ---------------------------------------------------------------------------
echo "=== Building galaxy_renderer ==="
cmake -S "$ROOT" -B "$BUILD" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" \
    -DMINGW_DEPS_PREFIX="$PREFIX" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD" -j"$JOBS"

# ---------------------------------------------------------------------------
# Stage portable dist-win folder
# ---------------------------------------------------------------------------
echo "=== Staging $DIST ==="
rm -rf "$DIST"
mkdir -p "$DIST"
cp "$BUILD/galaxy_renderer.exe" "$DIST/"
cp "$PREFIX/bin/SDL2.dll" "$PREFIX/bin/SDL2_ttf.dll" "$PREFIX/bin/glew32.dll" "$DIST/"
cp -r "$ROOT/assets" "$DIST/assets"
cp -r "$ROOT/presets" "$DIST/presets"

echo
echo "Done. Portable Windows build staged in: $DIST"
echo "Note: video export additionally requires ffmpeg.exe in PATH on the target machine."
