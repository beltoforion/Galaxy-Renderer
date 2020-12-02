#pragma once

#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
// i don't support on screen text output on windows for the time being.
	#define GLEW_STATIC
	#include <gl/glew.h>
	#include <SDL.h>
	#include <SDL_opengl.h> 
	#include <SDL_ttf.h>
#else
	#include <GL/gl.h>	// Header File For The OpenGL32 Library
	#include <GL/glext.h>
	#include <GL/glu.h>	// Header File For The GLu32 Library
	#include <SDL/SDL.h>
	#include <SDL/SDL_gfxPrimitives.h>
	#include <SDL/SDL_opengl.h> // opengl support
#endif

#include "Star.h"


/** \brief Basic infrastructure for grafical output using SDL/OpenGL */
class SDLWindow
{
public:
	void Init(int width, int height, double axisLen, const std::string& caption);

	void MainLoop();
	void ExitMainLoop();

	int GetWidth() const;
	int GetHeight() const;
	
	virtual void Render() = 0;

protected:
	SDLWindow();
	virtual ~SDLWindow();

	virtual void InitGL() = 0;
	virtual void InitSimulation() = 0;

	virtual void PollEvents();
	virtual void OnProcessEvents(Uint32 type);

	const Vec3D& GetCamPos() const;
	const Vec3D& GetCamOrient() const;
	const Vec3D& GetCamLookAt() const;

	void SetCameraOrientation(const Vec3D& orientation);
	void SetCamera(const Vec3D& pos, const Vec3D& lookAt, const Vec3D& orient);
	void AdjustCamera();
	void DrawAxis(const Vec2D& origin);
	int GetFPS() const;
	void ScaleAxis(double scale);
	double GetFOV() const;
	SDL_Event m_event;

	void TextOut(const char* fmt, ...);
	void TextOut(TTF_Font *pFont, int x, int y, const char* fmt, ...);

	static Vec3D GetOGLPos(int x, int y);

	double _fov;		///< Length of an axis
	
	std::string _caption;

	int _width;			///< Width of the window in pixel
	int _height;		///< Height of the window in pixel
	int _fps;

	Vec3D _camPos;		///< Position of the camera
	Vec3D _camLookAt;	///< Point atwhich the camera is aimed
	Vec3D _camOrient;	///< orientation of the camera (rotation as it aims at its target)

	SDL_Window *_pSdlWnd;
	SDL_Renderer *_pSdlRenderer;
	SDL_GLContext _sdcGlContext;


	GLuint _texStar;

	volatile bool _bRunning;
};
