#!/usr/bin/env bash
# Cross-compiles galaxy_renderer for 64-bit Windows using MinGW-w64 and
# stages a portable folder in <repo>/dist/windows (exe + DLLs + assets +
# presets). Windows builds of SDL2/SDL2_ttf and the GLEW sources are
# downloaded once and cached in <repo>/dependencies/windows.
#
# Prerequisite: sudo apt install g++-mingw-w64-x86-64
set -euo pipefail

SRC_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$SRC_DIR/build-windows"
DEPS_DIR="$SRC_DIR/dependencies/windows"
OUT_DIR="$SRC_DIR/dist/windows"

SDL2_VERSION=2.32.10
SDL2_TTF_VERSION=2.24.0
GLEW_VERSION=2.2.0

if ! command -v x86_64-w64-mingw32-g++-posix >/dev/null 2>&1 \
   && ! command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
    echo "error: MinGW-w64 compiler not found." >&2
    echo "Install it with: sudo apt install g++-mingw-w64-x86-64" >&2
    exit 1
fi

# ---------------------------------------------------------------------------
# Fetch Windows dependencies (cached)
# ---------------------------------------------------------------------------
mkdir -p "$DEPS_DIR"

fetch_and_extract() {
    local url="$1" marker="$2"
    if [ ! -e "$DEPS_DIR/$marker" ]; then
        local archive="$DEPS_DIR/$(basename "$url")"
        echo "Fetching $(basename "$url")..."
        curl -fL -o "$archive" "$url"
        tar -xf "$archive" -C "$DEPS_DIR"
        rm -f "$archive"
    fi
}

fetch_and_extract \
    "https://github.com/libsdl-org/SDL/releases/download/release-$SDL2_VERSION/SDL2-devel-$SDL2_VERSION-mingw.tar.gz" \
    "SDL2-$SDL2_VERSION"
fetch_and_extract \
    "https://github.com/libsdl-org/SDL_ttf/releases/download/release-$SDL2_TTF_VERSION/SDL2_ttf-devel-$SDL2_TTF_VERSION-mingw.tar.gz" \
    "SDL2_ttf-$SDL2_TTF_VERSION"
fetch_and_extract \
    "https://github.com/nigels-com/glew/releases/download/glew-$GLEW_VERSION/glew-$GLEW_VERSION.tgz" \
    "glew-$GLEW_VERSION"

SDL2_ROOT="$DEPS_DIR/SDL2-$SDL2_VERSION/x86_64-w64-mingw32"
SDL2_TTF_ROOT="$DEPS_DIR/SDL2_ttf-$SDL2_TTF_VERSION/x86_64-w64-mingw32"
GLEW_ROOT="$DEPS_DIR/glew-$GLEW_VERSION"

# ---------------------------------------------------------------------------
# Configure & build
# ---------------------------------------------------------------------------
cmake -S "$SRC_DIR" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$SRC_DIR/packaging/toolchain-mingw64.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$SDL2_ROOT;$SDL2_TTF_ROOT" \
    -DCMAKE_FIND_ROOT_PATH="$SDL2_ROOT;$SDL2_TTF_ROOT" \
    -DGLEW_SOURCE_DIR="$GLEW_ROOT" \
    -DSDL2_NO_MWINDOWS=1
cmake --build "$BUILD_DIR" -j"$(nproc)"

# ---------------------------------------------------------------------------
# Stage portable folder
# ---------------------------------------------------------------------------
# Clean contents only — deleting the directory itself would strand any shell
# whose working directory is inside it.
mkdir -p "$OUT_DIR"
rm -rf "$OUT_DIR"/* "$OUT_DIR"/.[!.]* 2>/dev/null || true

cp "$BUILD_DIR/galaxy_renderer.exe" "$OUT_DIR/"
cp "$SDL2_ROOT/bin/SDL2.dll" "$OUT_DIR/"
cp "$SDL2_TTF_ROOT/bin/SDL2_ttf.dll" "$OUT_DIR/"
cp -r "$SRC_DIR/assets" "$OUT_DIR/assets"
cp -r "$SRC_DIR/presets" "$OUT_DIR/presets"

echo "Windows build staged in $OUT_DIR/"
