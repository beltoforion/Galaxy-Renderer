[![GitHub issues](https://img.shields.io/github/issues/beltoforion/Galaxy-Renderer.svg?maxAge=360)](https://github.com/beltoforion/Galaxy-Renderer/issues)
[![Version](https://img.shields.io/github/release/beltoforion/Galaxy-Renderer.svg?maxAge=360)](https://github.com/beltoforion/Galaxy-Renderer/blob/master/CHANGELOG)
[![Github All Releases](https://img.shields.io/github/downloads/beltoforion/Galaxy-Renderer/total.svg)](https://github.com/beltoforion/Galaxy-Renderer/releases/tag/v1.1.0)

# Spiral Galaxy Renderer

A program for modelling a two dimensional galaxy based on the density wave theory. This repository contains the source code for an
article at beltoforion.de about simulating a galaxy with the density wave theory.

For more details please read the articles:

* https://beltoforion.de/en/spiral_galaxy_renderer [english]
* https://beltoforion.de/de/rendern_von_spiralgalaxien [german]

A typescript implementation of this code is available here:

* https://github.com/beltoforion/Galaxy-Renderer-Typescript

An online demo of the typescript version can be viewed here:

* https://beltoforion.de/en/spiral_galaxy_renderer/spiral-galaxy-renderer.html

The output of the C++ version is virtually the same.
<img width="1920" height="1080" alt="Bildschirmfoto vom 2026-07-19 21-58-13" src="https://github.com/user-attachments/assets/aff7e84a-f03d-4544-88ef-c013f5052c27" />

Here is a selection of galaxies created by the algorithm:
![galaxy-renderer-overview](https://user-images.githubusercontent.com/2202567/183997403-eff97429-b0d2-4933-a49e-ac16e0cab27c.jpg)

-----------

## Building

The project is built with [CMake](https://cmake.org) (3.16+) and a C++17 compiler. It depends on
**SDL2**, **SDL2_ttf**, **GLEW** and **OpenGL**. [Dear ImGui](https://github.com/ocornut/imgui) is
fetched automatically by CMake, and [glm](https://github.com/g-truc/glm) is bundled in
`dependencies/`.

Install the dependencies:

* **Linux (Debian/Ubuntu):** `sudo apt install cmake libsdl2-dev libsdl2-ttf-dev libglew-dev`
* **macOS (Homebrew):** `brew install cmake sdl2 sdl2_ttf glew`
* **Windows ([vcpkg](https://vcpkg.io)):** `vcpkg install sdl2 sdl2-ttf glew` and configure with the
  vcpkg toolchain file.

Then configure and build:

```
cmake -S . -B build
cmake --build build
```

The executable is written to `build/` and the `assets/` folder (fonts) is copied next to it, so run
it from there:

```
cd build
./galaxy_renderer
```

To stage a portable copy (binary + assets + presets) in `dist/`:

```
cmake --build build --target dist
```

On Linux, to additionally package that into a self-contained `dist/Galaxy_Renderer-x86_64.AppImage`
(fetches [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) on first run):

```
cmake --build build --target appimage
```

### Cross-compiling for Windows (on Linux)

With the MinGW-w64 toolchain installed (`sudo apt install g++-mingw-w64-x86-64`), a portable
Windows build (exe + DLLs + assets + presets) can be staged in `dist/windows/` with:

```
./packaging/build-windows.sh
```

Windows builds of SDL2/SDL2_ttf and the GLEW sources are downloaded automatically on first run and
cached in `dependencies/windows/`.

-----------

## User Interface

All parameters can be edited live through a **Dear ImGui** control panel (sliders, checkboxes,
combo boxes and buttons). Press **[F1]** to show or hide the panel. The classic keyboard shortcuts
still work as well (e.g. `[F2]` axis, `[F3]` dust, `[+]`/`[-]` zoom, `[Space]` pause, the numeric
keypad for the predefined galaxies).

-----------

## Presets

Galaxy configurations are stored as plain text files. The presets shipped with the application live
in the `presets/` folder next to the executable; your own saves go to the per-user data directory
(`~/.local/share/beltoforion/GalaxyRenderer/presets/` on Linux,
`%APPDATA%\beltoforion\GalaxyRenderer\presets\` on Windows). A user preset with the same name
overrides a shipped one. Presets can be selected, saved and overwritten from the *Presets* section
of the control panel.

-----------

## Video Export

The renderer can export its animation as an H.264 encoded MP4 video. The video is rendered into an 
offscreen framebuffer whose resolution is independent of the window size, so you can record 4K 
(or higher) footage on a normal desktop. The raw frames are piped to [ffmpeg](https://ffmpeg.org) 
for encoding, so ffmpeg must be installed and available in the search path.

* Press **[F7]** (or use the *Video Export* section of the control panel) to start and stop the 
  recording. The video is written to a file named `galaxy-YYYYMMDD-HHMMSS.mp4` in the current 
  working directory.
* The default video resolution is 3840x2160 (4K UHD) at 60 fps. It can be set in the control panel 
  or on the command line:

```
./galaxy_renderer --video-size 1920x1080 --video-fps 30
```

Notes:

* One simulation step is recorded per video frame, so the exported video plays back the evolution 
  of the galaxy at a fixed rate independent of how fast your machine renders.
* Text overlays (help screen, axis labels) are not part of the video; use [F2]/[F6] etc. to choose 
  which visual elements (axis, density waves, dust, ...) are included.
* Recording 4K footage is demanding. If recording is slow, this only affects the time it takes to 
  record - the resulting video always plays back smoothly at the selected frame rate.

-----------

## Troubleshooting

The renderer requires OpenGL 3.3. On old systems or GPUs whose driver does not advertise it, the
Mesa version override may help (Linux):

```
MESA_GL_VERSION_OVERRIDE=3.3 MESA_GLSL_VERSION_OVERRIDE=330 ./galaxy_renderer
```

