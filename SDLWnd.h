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

	SDLWindow(int width, int height, double axisLen, const std::string& caption);
	virtual ~SDLWindow();
	void MainLoop();
	void ExitMainLoop();

	int GetWidth() const;
	int GetHeight() const;
	
	virtual void Render() = 0;

protected:

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
	SDL_Window* Surface();
	SDL_Event m_event;

	void InitFont();
	static void KillFont();
	static void TextOut(const char* fmt, ...);
	static void TextOut(int x, int y, const char* fmt, ...);
	static Vec3D GetOGLPos(int x, int y);

	static GLuint s_fontBase;

protected:

	double m_fov;		///< Length of an axis
	
	TTF_Font *_pFont;
	int _width;			///< Width of the window in pixel
	int _height;		///< Height of the window in pixel
	int m_fps;
	int m_idxSnapshot;

	Vec3D m_camPos;    ///< Position of the camera
	Vec3D m_camLookAt; ///< Point atwhich the camera is aimed
	Vec3D m_camOrient; ///< orientation of the camera (rotation as it aims at its target)

	SDL_Window* m_pScreen;
	SDL_GLContext m_context;

	GLuint m_fontBase;
	GLuint m_texStar;

	volatile bool m_bRunning;

protected:

	void InitGL();
};
