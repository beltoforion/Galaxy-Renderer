[![GitHub issues](https://img.shields.io/github/issues/beltoforion/Galaxy-Renderer.svg?maxAge=360)](https://github.com/beltoforion/Galaxy-Renderer/issues)
[![Version](https://img.shields.io/github/release/beltoforion/Galaxy-Renderer.svg?maxAge=360)](https://github.com/beltoforion/Galaxy-Renderer/blob/master/CHANGELOG)
[![Github All Releases](https://img.shields.io/github/downloads/beltoforion/Galaxy-Renderer/total.svg)](https://github.com/beltoforion/Galaxy-Renderer/releases/tag/v1.1.0)

# Spiral Galaxy Renderer

A Program for modelling a two dimensional galaxy based on the density wave theory. This archive contains the source code for an 
article at beltoforion.de about simulating a galaxy with the density wave theory.

For more Details please read the Articles.

* https://beltoforion.de/en/spiral_galaxy_renderer [english]
* https://beltoforion.de/de/rendern_von_spiralgalaxien [german]

A typescript implementation of this code is available here:

* https://github.com/beltoforion/Galaxy-Renderer-Typescript

An online demo of the typescript version can be viewed here:

* https://beltoforion.de/en/spiral_galaxy_renderer/spiral-galaxy-renderer.html

The output of the C++ version is virtually the same.

![galaxy-renderer-cpp](https://user-images.githubusercontent.com/2202567/183997359-e4480044-986e-445d-935c-a50d55db21f3.jpg)

Hers is a list of galaxies created by the algorithm:
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

### Cross-compiling the Windows binaries on Linux

The Windows (x64) binaries can be built on Linux with MinGW-w64. Install the toolchain and run the
build script:

```
sudo apt install cmake make g++-mingw-w64-x86-64-posix curl
./build_windows.sh
```

The script downloads and cross-compiles SDL2, FreeType, SDL2_ttf and GLEW once (into
`build-win-deps/`), then builds the application and stages a portable folder in `dist-win/`
containing `galaxy_renderer.exe`, the required DLLs (`SDL2.dll`, `SDL2_ttf.dll`, `glew32.dll`),
`assets/` and `presets/`. Copy that folder to a Windows machine and run the exe from inside it.
The GCC runtime is linked statically, so no MinGW DLLs are needed at runtime; video export needs
`ffmpeg.exe` in the `PATH` as on the other platforms.

`./build_windows.sh clean` removes all Windows build output.

-----------

## User Interface

All parameters can be edited live through a **Dear ImGui** control panel (sliders, checkboxes,
combo boxes and buttons). Press **[F1]** to show or hide the panel. The classic keyboard shortcuts
still work as well (e.g. `[F2]` axis, `[F3]` dust, `[+]`/`[-]` zoom, `[Space]` pause, the numeric
keypad for the predefined galaxies).

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

For old system or GPU unsupported OpenGL 3.3 use overload MESA version for running application.
In Linux.
```
MESA_GL_VERSION_OVERRIDE=3.3 MESA_GLSL_VERSION_OVERRIDE=330 ./galaxy_rendere
```

