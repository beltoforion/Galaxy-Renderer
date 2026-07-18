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

## Video Export

The renderer can export its animation as an H.264 encoded MP4 video. The video is rendered into an 
offscreen framebuffer whose resolution is independent of the window size, so you can record 4K 
(or higher) footage on a normal desktop. The raw frames are piped to [ffmpeg](https://ffmpeg.org) 
for encoding, so ffmpeg must be installed and available in the search path.

* Press **[F7]** to start and stop the recording. The video is written to a file named 
  `galaxy-YYYYMMDD-HHMMSS.mp4` in the current working directory.
* The default video resolution is 3840x2160 (4K UHD) at 60 fps. It can be changed on the command line:

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

