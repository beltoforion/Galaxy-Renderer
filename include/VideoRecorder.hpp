#pragma once

#include <string>
#include <cstdio>
#include <cstdint>
#include <vector>

#include <GL/glew.h>


/** \brief Records video by rendering into an offscreen framebuffer of arbitrary size
           (i.e. 4K) and piping the raw frames into an external ffmpeg process. */
class VideoRecorder
{
public:
	VideoRecorder();
	~VideoRecorder();

	bool Start(int width, int height, int fps, const std::string& filename);
	void Stop();

	/// Bind the offscreen framebuffer. Subsequent rendering goes into the video frame.
	void BindFramebuffer();

	/// Read the offscreen framebuffer and send its content to the encoder.
	bool CaptureFrame();

	bool IsRecording() const;
	int GetWidth() const;
	int GetHeight() const;
	int GetFps() const;
	int GetFrameCount() const;
	const std::string& GetFilename() const;

private:
	FILE* _pipe;
	GLuint _fbo;
	GLuint _colorBuffer;

	int _width;
	int _height;
	int _fps;
	int _frames;

	std::string _filename;
	std::vector<uint8_t> _frameData;

	void ReleaseGlResources();
};
