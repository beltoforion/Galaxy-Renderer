#!/usr/bin/env bash
# Packages the built galaxy_renderer binary as a portable Linux AppImage.
# Invoked by the CMake "appimage" target; not meant to be run standalone
# unless galaxy_renderer has already been built in <build-dir>.
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <build-dir> <source-dir>" >&2
    exit 1
fi

BUILD_DIR="$1"
SRC_DIR="$2"

BIN="$BUILD_DIR/galaxy_renderer"
if [ ! -x "$BIN" ]; then
    echo "error: $BIN not found; build the galaxy_renderer target first" >&2
    exit 1
fi

TOOLS_DIR="$BUILD_DIR/appimage-tools"
LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_URL="https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"

mkdir -p "$TOOLS_DIR"
if [ ! -x "$LINUXDEPLOY" ]; then
    echo "Fetching linuxdeploy..."
    curl -fL -o "$LINUXDEPLOY" "$LINUXDEPLOY_URL"
    chmod +x "$LINUXDEPLOY"
fi

APPDIR="$BUILD_DIR/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin"

cp "$BIN" "$APPDIR/usr/bin/"
cp -r "$SRC_DIR/assets" "$APPDIR/usr/bin/assets"
cp -r "$SRC_DIR/presets" "$APPDIR/usr/bin/presets"

OUT_DIR="$SRC_DIR/dist"
mkdir -p "$OUT_DIR"
rm -f "$OUT_DIR"/*.AppImage

(
    cd "$BUILD_DIR"
    # extract-and-run avoids depending on FUSE being available on the host
    export APPIMAGE_EXTRACT_AND_RUN=1
    ARCH=x86_64 "$LINUXDEPLOY" \
        --appdir "$APPDIR" \
        --desktop-file "$SRC_DIR/packaging/galaxy_renderer.desktop" \
        --icon-file "$SRC_DIR/packaging/galaxy_renderer.png" \
        --output appimage
)

mv "$BUILD_DIR"/Galaxy*.AppImage "$OUT_DIR/"

echo "AppImage staged in $OUT_DIR/"
