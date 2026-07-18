#include "VideoRecorder.hpp"

#include <iostream>
#include <sstream>

#ifdef _WIN32
	#define POPEN _popen
	#define PCLOSE _pclose
	static const char* popenMode = "wb";
#else
	#include <csignal>
	#define POPEN popen
	#define PCLOSE pclose
	static const char* popenMode = "w";
#endif


VideoRecorder::VideoRecorder()
	: _pipe(nullptr)
	, _fbo(0)
	, _colorBuffer(0)
	, _width(0)
	, _height(0)
	, _fps(0)
	, _frames(0)
	, _filename()
	, _frameData()
{}

VideoRecorder::~VideoRecorder()
{
	Stop();
}

bool VideoRecorder::Start(int width, int height, int fps, const std::string& filename)
{
	if (IsRecording())
		return false;

	// yuv420p output needs even frame dimensions
	width -= width % 2;
	height -= height % 2;

	GLint maxSize = 0;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxSize);
	if (width > maxSize || height > maxSize)
	{
		std::cout << "VideoRecorder: " << width << "x" << height
		          << " exceeds the maximum renderbuffer size of this GPU (" << maxSize << ")" << std::endl;
		return false;
	}

	glGenFramebuffers(1, &_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

	glGenRenderbuffers(1, &_colorBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, _colorBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorBuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "VideoRecorder: could not create an offscreen framebuffer of size "
		          << width << "x" << height << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		ReleaseGlResources();
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifndef _WIN32
	// Dying encoder processes must not take the renderer down with them
	std::signal(SIGPIPE, SIG_IGN);
#endif

	std::ostringstream cmd;
	cmd << "ffmpeg -hide_banner -loglevel error -y"
	    << " -f rawvideo -pixel_format rgb24"
	    << " -video_size " << width << "x" << height
	    << " -framerate " << fps
	    << " -i -"
	    << " -vf vflip"
	    << " -c:v libx264 -preset slow -crf 16 -pix_fmt yuv420p"
	    << " -movflags +faststart"
	    << " \"" << filename << "\"";

	_pipe = POPEN(cmd.str().c_str(), popenMode);
	if (_pipe == nullptr)
	{
		std::cout << "VideoRecorder: could not start ffmpeg. Is it installed and in the search path?" << std::endl;
		ReleaseGlResources();
		return false;
	}

	_width = width;
	_height = height;
	_fps = fps;
	_frames = 0;
	_filename = filename;
	_frameData.resize((size_t)width * height * 3);

	std::cout << "VideoRecorder: recording " << width << "x" << height << " at " << fps
	          << " fps to \"" << filename << "\"" << std::endl;
	return true;
}

void VideoRecorder::Stop()
{
	if (_pipe != nullptr)
	{
		int status = PCLOSE(_pipe);
		_pipe = nullptr;

		if (status != 0)
		{
			std::cout << "VideoRecorder: ffmpeg exited with an error. Is it installed and in the search path?" << std::endl;
		}
		else
		{
			std::cout << "VideoRecorder: wrote " << _frames << " frames ("
			          << (double)_frames / _fps << " s) to \"" << _filename << "\"" << std::endl;
		}
	}

	ReleaseGlResources();
}

void VideoRecorder::BindFramebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
	glViewport(0, 0, _width, _height);
}

bool VideoRecorder::CaptureFrame()
{
	if (!IsRecording())
		return false;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, _fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, _width, _height, GL_RGB, GL_UNSIGNED_BYTE, _frameData.data());
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	size_t written = fwrite(_frameData.data(), 1, _frameData.size(), _pipe);
	if (written != _frameData.size())
	{
		std::cout << "VideoRecorder: could not send frame data to ffmpeg; recording stopped" << std::endl;
		Stop();
		return false;
	}

	++_frames;
	if (_frames % (_fps * 5) == 0)
	{
		std::cout << "VideoRecorder: " << _frames << " frames ("
		          << (double)_frames / _fps << " s) recorded" << std::endl;
	}

	return true;
}

bool VideoRecorder::IsRecording() const
{
	return _pipe != nullptr;
}

int VideoRecorder::GetWidth() const
{
	return _width;
}

int VideoRecorder::GetHeight() const
{
	return _height;
}

int VideoRecorder::GetFps() const
{
	return _fps;
}

int VideoRecorder::GetFrameCount() const
{
	return _frames;
}

const std::string& VideoRecorder::GetFilename() const
{
	return _filename;
}

void VideoRecorder::ReleaseGlResources()
{
	if (_colorBuffer != 0)
	{
		glDeleteRenderbuffers(1, &_colorBuffer);
		_colorBuffer = 0;
	}

	if (_fbo != 0)
	{
		glDeleteFramebuffers(1, &_fbo);
		_fbo = 0;
	}
}
