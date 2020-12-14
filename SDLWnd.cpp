#include "SDLWnd.hpp"

#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cmath>

#include <SDL_ttf.h>
#include <glm/gtc/matrix_transform.hpp>

#include "MathHelper.hpp"


Vec2D SDLWindow::GetWindowPos(GLfloat x, GLfloat y, GLfloat z)
{
	GLdouble modelview[16];
	GLdouble projection[16];
	GLdouble screen[3];
	GLint viewport[4];

	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluProject(x, y, z,	modelview, projection, viewport, screen + 0, screen + 1, screen + 2);

	return { (float)screen[0], (float)screen[1] };
}


SDLWindow::SDLWindow()
	: _event()
	, _fov(0)
	, _width(0)
	, _height(0)
	, _caption()
	, _fps(0)
	, _camPos({ 0, 0, 2 })
	, _camLookAt({ 0, 0, 0 })
	, _camOrient({ 0, 1, 0 })
	, _pSdlWnd(nullptr)
	, _pSdlRenderer(nullptr)
	, _bRunning(true)
	, _matProjection()
	, _matView()
{}

SDLWindow::~SDLWindow()
{
	SDL_Quit();
}

/*
void SDLWindow::Close()
{
	SDL_DestroyRenderer(_pSdlRenderer);
	SDL_DestroyWindow(screen.window);
	screen.renderer = nullptr;
	screen.window = nullptr;
	SDL_Quit();
}
*/

void SDLWindow::Init(int width, int height, float axisLen, const std::string& caption)
{
	_fov = axisLen;
	_width = width;
	_height = height;

	SDL_Init(SDL_INIT_VIDEO);
	atexit(SDL_Quit);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

	_pSdlWnd = SDL_CreateWindow(
		caption.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height,
		SDL_WINDOW_OPENGL);

	if (!_pSdlWnd)
		throw std::runtime_error(SDL_GetError());

	_pSdlRenderer = SDL_CreateRenderer(_pSdlWnd, -1, SDL_RENDERER_ACCELERATED);
	_sdcGlContext = SDL_GL_CreateContext(_pSdlWnd);

	glewInit();

	std::cout << "OpenGL Version Information:" << glGetString(GL_VERSION) << std::endl;
	std::cout << "- OpenGL:     " << glGetString(GL_VERSION) << std::endl;
	std::cout << "- GLSL:       " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
	std::cout << "- Vendor/GPU: " << glGetString(GL_VENDOR) << "/" << glGetString(GL_RENDERER) << std::endl;

	InitGL();
	InitSimulation();
}

void SDLWindow::ScaleAxis(float scale)
{
	_fov *= scale;
	AdjustCamera();
}

const Vec3D& SDLWindow::GetCamPos() const
{
	return _camPos;
}

const Vec3D& SDLWindow::GetCamOrient() const
{
	return _camOrient;
}

const Vec3D& SDLWindow::GetCamLookAt() const
{
	return _camLookAt;
}

void SDLWindow::SetCameraOrientation(const Vec3D& orient)
{
	_camOrient = orient;
	AdjustCamera();
}

void SDLWindow::SetCamera(const Vec3D& pos, const Vec3D& lookAt, const Vec3D& orient)
{
	_camOrient = orient;
	_camPos = pos;
	_camLookAt = lookAt;
	AdjustCamera();
}

void SDLWindow::AdjustCamera()
{
	double l = _fov / 2.0;
	double aspect = (double)_width / _height;

	// new mvp matrices for glsl shaders via glm:
	_matProjection = glm::ortho(
		-l * aspect, l * aspect, 
		-l, l, 
		-l, l);
	
	glm::dvec3 camPos(_camPos.x, _camPos.y, _camPos.z);
	glm::dvec3 camLookAt(_camLookAt.x, _camLookAt.y, _camLookAt.z);
	glm::dvec3 camOrient(_camOrient.x, _camOrient.y, _camOrient.z);
	_matView = glm::lookAt(camPos, camLookAt, camOrient);

	// old stuff (legacy):
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-l* aspect, l * aspect, -l, l, -l, l);

	gluLookAt(
		_camPos.x, _camPos.y, _camPos.z,
		_camLookAt.x, _camLookAt.y, _camLookAt.z,
		_camOrient.x, _camOrient.y, _camOrient.z);
}

double SDLWindow::GetFOV() const
{
	return _fov;
}

int SDLWindow::GetFPS() const
{
	return _fps;
}


/** \brief Main render loop

  Handles Keyevents advances the time and renders the galaxy.
*/
void SDLWindow::MainLoop()
{
	int ct = 0;
	double dt = 0;
	time_t t1(time(nullptr)), t2;

	while (_bRunning)
	{
		Update();
		Render();

		PollEvents();
		++ct;

		t2 = time(nullptr);
		dt = difftime(t2, t1);
		if (dt > 1)
		{
			_fps = (int)((double)ct / dt);
			ct = 0;
			t1 = t2;
		}
	}
}

int SDLWindow::GetWidth() const
{
	return _width;
}

int SDLWindow::GetHeight() const
{
	return _height;
}

void SDLWindow::ExitMainLoop()
{
	_bRunning = false;
}

void SDLWindow::OnProcessEvents(Uint32 type)
{}

void SDLWindow::PollEvents()
{
	while (SDL_PollEvent(&_event))
	{
		switch (_event.type)
		{
		case SDL_QUIT:
			ExitMainLoop();
			break;

		default:
			OnProcessEvents(_event.type);
			break;
		} // switch event type
	}
}