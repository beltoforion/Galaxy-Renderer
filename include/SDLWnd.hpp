#pragma once

#include <string>

#include <gl/glew.h>
#include <glm/glm.hpp>

#include <SDL.h>
#include <SDL_opengl.h> 
#include <SDL_ttf.h>

#include "Star.hpp"


/** \brief Basic infrastructure for grafical output using SDL/OpenGL */
class SDLWindow
{
public:
	void Init(int width, int height, float axisLen, const std::string& caption);

	void MainLoop();
	void ExitMainLoop();

	int GetWidth() const;
	int GetHeight() const;
	
protected:
	enum class TextCoords
	{
		Model,
		Window
	};

	SDLWindow();
	virtual ~SDLWindow();

	virtual void Render() = 0;
	virtual void Update() = 0;

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
	int GetFPS() const;
	void ScaleAxis(float scale);
	double GetFOV() const;
	SDL_Event m_event;

	void DrawText(TTF_Font *pFont, TextCoords coords, float x, float y, const char* fmt, ...);

	static Vec3D GetOGLPos(int x, int y);
	static Vec2D GetWindowPos(GLfloat x, GLfloat y, GLfloat z);

	float _fov;		///< Length of an axis
	
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

	glm::mat4 _matProjection;
	glm::mat4 _matView;

	GLuint _texStar;

	volatile bool _bRunning;
};
