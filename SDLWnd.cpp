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


void SDLWindow::DrawText(TTF_Font* pFont, TextCoords coords, float x, float y, const char* fmt, ...)
{
	if (pFont == nullptr)
		throw std::runtime_error("TextOut failed: font is null!");

	if (fmt == nullptr)
		throw std::runtime_error("TextOut failed: bad format string!");

	char text[256];
	va_list ap;

	va_start(ap, fmt);

	vsprintf(text, fmt, ap);
	va_end(ap);

	auto* pSurface = TTF_RenderText_Blended(pFont, text, { 255, 255, 255 });
	if (pSurface == nullptr)
		return;

	GLuint texId;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);

	SDL_Surface* s = nullptr;
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// It seems textures must be powers of 2 in dimension: 
	// https://stackoverflow.com/questions/30016083/sdl2-opengl-sdl2-ttf-displaying-text
	// Create a surface to the correct size in RGB format, and copy the old image
	int w = MathHelper::PowerTwoFloor(pSurface->w) << 1;
	int h = MathHelper::PowerTwoFloor(pSurface->h) << 1;
	s = SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_BlitSurface(pSurface, nullptr, s, nullptr);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, s->pixels);


	glMatrixMode(GL_PROJECTION);

	if (coords == TextCoords::Model)
	{
		auto pos = GetWindowPos((GLfloat)x, (GLfloat)y, 0);
		x = pos.x;
		y = pos.y;
	}

	GLfloat xp = x;
	GLfloat yp = y;

	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, _width, _height, 0, 0, 1);

	// make a rectangle
	glBegin(GL_TRIANGLES);
	glTexCoord2i(0, 0);
	glVertex3f(xp, yp, 0);
	glTexCoord2i(1, 0);
	glVertex3f(xp + w, yp, 0);
	glTexCoord2i(1, 1);
	glVertex3f(xp + w, yp + h, 0);

	glTexCoord2i(0, 0);
	glVertex3f(xp, yp, 0);
	glTexCoord2i(0, 1);
	glVertex3f(xp, yp + h, 0);
	glTexCoord2i(1, 1);
	glVertex3f(xp + w, yp + h, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	// cleanup
	if (s != nullptr)
		SDL_FreeSurface(s);

	if (pSurface != nullptr)
		SDL_FreeSurface(pSurface);

	glDeleteTextures(1, &texId);
	glPopMatrix();
}

/** \brief get opengl position from a screen position

   see also:  http://nehe.gamedev.net/data/articles/article.asp?article=13
*/
Vec3D SDLWindow::GetOGLPos(int x, int y)
{
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	return {(float)posX, (float)posY, (float)posZ};
}

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
	: m_event()
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
	, _texStar(0)
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
	while (SDL_PollEvent(&m_event))
	{
		switch (m_event.type)
		{
		case SDL_QUIT:
			ExitMainLoop();
			break;

		default:
			OnProcessEvents(m_event.type);
			break;
		} // switch event type
	}
}