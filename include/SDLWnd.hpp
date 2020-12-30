#pragma once

#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <SDL.h>
#include <SDL_opengl.h> 
#include <SDL_ttf.h>

#include "Types.hpp"


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
	SDLWindow();
	virtual ~SDLWindow();

	virtual void Render() = 0;
	virtual void Update() = 0;

	virtual void InitGL() noexcept (false) = 0;
	virtual void InitSimulation() = 0;

	virtual void PollEvents();
	virtual void OnProcessEvents(Uint32 type);

	const Vec3& GetCamPos() const;
	const Vec3& GetCamOrient() const;
	const Vec3& GetCamLookAt() const;

	void SetCameraOrientation(const Vec3& orientation);
	void SetCamera(const Vec3& pos, const Vec3& lookAt, const Vec3& orient);
	void AdjustCamera();
	int GetFPS() const;
	void ScaleAxis(float scale);
	double GetFOV() const;
	Vec2 GetWindowPos(GLfloat x, GLfloat y, GLfloat z);

	SDL_Event _event;

	float _fov;			///< Length of an axis
	
	std::string _caption;

	int _width;			///< Width of the window in pixel
	int _height;		///< Height of the window in pixel
	int _fps;

	Vec3 _camPos;		///< Position of the camera
	Vec3 _camLookAt;	///< Point atwhich the camera is aimed
	Vec3 _camOrient;	///< orientation of the camera (rotation as it aims at its target)

	SDL_Window *_pSdlWnd;
	SDL_Renderer *_pSdlRenderer;
	SDL_GLContext _sdcGlContext;

	glm::mat4 _matProjection;
	glm::mat4 _matView;

	volatile bool _bRunning;
	bool _stopEventPolling;
};
