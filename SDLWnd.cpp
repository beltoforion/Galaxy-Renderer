#include "SDLWnd.h"

#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cmath>

#include <SDL_ttf.h>

#if defined(_WIN32) || defined(_WIN64)

#else
#include <GL/gl.h>	// Header File For The OpenGL32 Library
#include <GL/glu.h>	// Header File For The GLu32 Library
#include <GL/glx.h> 
#endif


void SDLWindow::InitFont()
{
	TTF_Init();
	_pFont = TTF_OpenFont("arial.ttf", 12); 
	if (_pFont == nullptr)
		throw std::runtime_error(TTF_GetError());
}


void SDLWindow::TextOut(const char* fmt, ...)
{
/*
	char text[256]; 
	va_list ap;     

	if (fmt == nullptr)
		return;

	va_start(ap, fmt);

	vsprintf(text, fmt, ap);
	va_end(ap);

	glPushAttrib(GL_LIST_BIT);     // Pushes the Display List Bits
	glListBase(s_fontBase - 32);   // Sets base character to 32
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text); // Draws the text
	glPopAttrib();                 // Pops the Display List Bits
*/
}

void SDLWindow::TextOut(int x, int y, const char* fmt, ...)
{
	return;

	char text[256];
	va_list ap;

	if (fmt == nullptr)
		return;

	va_start(ap, fmt);

	vsprintf(text, fmt, ap);
	va_end(ap);

	Vec3D p = GetOGLPos(x, y);

	auto *pSurface = TTF_RenderText_Solid(_pFont, text, { 255, 255,  255 });
	if (pSurface == nullptr)
		return;

	SDL_Texture *pTexture = SDL_CreateTextureFromSurface(_pSdlRenderer, pSurface);
	if (pTexture == nullptr)
		return;

//	glRasterPos2f((GLfloat)x, (GLfloat)y);

	GLfloat xp = x, yp = y;
	GLfloat w = 100, h = 20;
	SDL_GL_BindTexture(pTexture, nullptr, nullptr);
	
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_TEXTURE_2D);       // point sprite texture support

	// make a rectangle
	glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(xp, yp, 0);

		glTexCoord2i(1, 0);
		glVertex3f(xp + w, yp, 0);

		glTexCoord2i(1, 1);
		glVertex3f(xp + w, yp + h, 0);

		glTexCoord2i(0, 1);
		glVertex3f(xp, yp + h, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	
	SDL_GL_UnbindTexture(pTexture);

	if (pSurface!=nullptr)
		SDL_FreeSurface(pSurface);

	if (pTexture!=nullptr)
		SDL_DestroyTexture(pTexture);
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

	return Vec3D(posX, posY, posZ);
}


SDLWindow::SDLWindow()
	: m_event()
	, _fov(0)
	, _width(0)
	, _height(0)
	, _caption()
	, _fps(0)
	, _camPos(0, 0, 2)
	, _camLookAt(0, 0, 0)
	, _camOrient(0, 1, 0)
	, _pSdlWnd(nullptr)
	, _pSdlRenderer(nullptr)
	, m_fontBase(0)
	, m_texStar(0)
	, _bRunning(true)
{
}

SDLWindow::~SDLWindow()
{
	SDL_Quit();
}

/*
void SDLWindow::Close()
{
	SDL_DestroyRenderer(_pSdlRenderer);
	SDL_DestroyWindow(screen.window);
	screen.renderer = NULL;
	screen.window = NULL;
	SDL_Quit();
}
*/

void SDLWindow::Init(int width, int height, double axisLen, const std::string& caption)
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

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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

	InitFont();
	InitGL();
	InitSimulation();
}

void SDLWindow::ScaleAxis(double scale)
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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double l = _fov / 2.0;
	glOrtho(-l, l, -l, l, -l, l);
	gluLookAt(
		_camPos.x, _camPos.y, _camPos.z,
		_camLookAt.x, _camLookAt.y, _camLookAt.z,
		_camOrient.x, _camOrient.y, _camOrient.z);
	glMatrixMode(GL_MODELVIEW);
}

double SDLWindow::GetFOV() const
{
	return _fov;
}

int SDLWindow::GetFPS() const
{
	return _fps;
}

void SDLWindow::DrawAxis(const Vec2D& origin)
{
	glColor3f((GLfloat)0.3, (GLfloat)0.3, (GLfloat)0.3);

	double s = std::pow(10, (int)(std::log10(_fov / 2))),
		l = _fov / 100,
		p = 0;

	glPushMatrix();
	glTranslated(origin.x, origin.y, 0);

	for (int i = 0; p < _fov; ++i)
	{
		p += s;

		if (i % 2 == 0)
		{
			glRasterPos2f(p - l, -4 * l);
			TextOut("%2.0f", p);
		}
		else
		{
			glRasterPos2f(p - l, 2 * l);
			TextOut("%2.0f", p);
		}

		glBegin(GL_LINES);
		glVertex3f(p, -l, 0);
		glVertex3f(p, l, 0);

		glVertex3f(-p, -l, 0);
		glVertex3f(-p, 0, 0);
		glVertex3f(-l, p, 0);
		glVertex3f(0, p, 0);
		glVertex3f(-l, -p, 0);
		glVertex3f(0, -p, 0);
		glEnd();

	}

	glBegin(GL_LINES);
	glVertex3f((GLfloat)-_fov, 0, 0);
	glVertex3f((GLfloat)_fov, 0, 0);
	glVertex3f(0, (GLfloat)-_fov, 0);
	glVertex3f(0, (GLfloat)_fov, 0);
	glEnd();

	glPopMatrix();
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