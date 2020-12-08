#include "GalaxyWnd.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <cstddef>
#include <iostream>

#include "MathHelper.hpp"
#include "specrend.h"
#include "Star.hpp"


GalaxyWnd::GalaxyWnd()
	: SDLWindow()
	, _camOrient(0)
	, _starRenderType(1)
	, _flags((int)DisplayItem::STARS | (int)DisplayItem::AXIS | (int)DisplayItem::HELP | (int)DisplayItem::DUST | (int)DisplayItem::H2)
	, _galaxy()
	, _colNum(200)
	, _t0(1000)
	, _t1(10000)
	, _dt((_t1 - _t0) / _colNum)
	, _renderUpdateHint(ruhDENSITY_WAVES | ruhAXIS | ruhSTARS | ruhDUST | ruhH2)
	, _vertDensityWaves(2)
	, _vertAxis()
	, _vertStars()
	, _pSmallFont(nullptr)
	, _pFont(nullptr)
	, _pFontCaption(nullptr)
{
	double x, y, z;
	double r, g, b;
	for (int i = 0; i < _colNum; ++i)
	{
		Color& col = _col[i];
		colourSystem* cs = &SMPTEsystem;
		bbTemp = _t0 + _dt * i;
		spectrum_to_xyz(bb_spectrum, &x, &y, &z);

		xyz_to_rgb(cs, x, y, z, &r, &g, &b);
		norm_rgb(&r, &g, &b);

		col.r = (float)r;
		col.g = (float)g;
		col.b = (float)b;
		col.a = 1.f;
	}
}

GalaxyWnd::~GalaxyWnd()
{
	_vertDensityWaves.Release();
	_vertAxis.Release();
	_vertStars.Release();
}

void GalaxyWnd::InitGL() noexcept(false)
{
	// Font initialization
	TTF_Init();
	_pSmallFont = TTF_OpenFont("consola.ttf", 14);
	if (_pSmallFont == nullptr)
		throw std::runtime_error(TTF_GetError());

	_pFont = TTF_OpenFont("arial.ttf", 18);
	if (_pFont == nullptr)
		throw std::runtime_error(TTF_GetError());

	_pFontCaption = TTF_OpenFont("arial.ttf", 40);
	if (_pFontCaption == nullptr)
		throw std::runtime_error(TTF_GetError());

	// GL initialization
	glShadeModel(GL_SMOOTH);
	glViewport(0, 0, GetWidth(), GetHeight());

	SDL_Surface* tex;

	tex = SDL_LoadBMP("particle.bmp");
	if (!tex)
		throw std::runtime_error("Can't load star texture (particle.bmp).");

	// Check that the image's width is a power of 2
	if (tex->w & (tex->w - 1))
		throw std::runtime_error("texture width is not a power of 2.");

	// Also check if the height is a power of 2
	if (tex->h & (tex->h - 1))
		throw std::runtime_error("texture height is not a power of 2.");

	// get the number of channels in the SDL surface
	GLint  nOfColors = tex->format->BytesPerPixel;
	GLenum texture_format;
	if (nOfColors == 4)     // contains an alpha channel
	{
		texture_format = GL_RGBA;
	}
	else if (nOfColors == 3)     // no alpha channel
	{
		texture_format = GL_RGB;
	}
	else
		throw std::runtime_error("image is not truecolor");

	// Have OpenGL generate a texture object handle for us
	glGenTextures(1, &_texStar);

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, _texStar);

	// Set the texture's stretching properties
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Edit the texture object's image data using the information SDL_Surface gives us
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		nOfColors,
		tex->w,
		tex->h,
		0,
		texture_format,
		GL_UNSIGNED_BYTE,
		tex->pixels);

	_vertDensityWaves.Initialize();
	_vertAxis.Initialize();
	_vertStars.Initialize();

	glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, .0f, 0.1f, 0.0f);

	SetCameraOrientation({ 0, 1, 0 });
}

void GalaxyWnd::InitSimulation()
{
	_galaxy.Reset(
		13000,    // radius of the galaxy
		4000,     // radius of the core
		0.0004f,   // angluar offset of the density wave per parsec of radius
		0.85f,     // excentricity at the edge of the core
		0.95f,      // excentricity at the edge of the disk
		30000,    // total number of stars
		true,     // has dark matter
		2,        // Perturbations per full ellipse
		40,       // Amplitude damping factor of perturbation
		100,
		4000);      // dust render size in pixel
}

void GalaxyWnd::UpdateDust()
{
	std::cout << "Updating dust" << std::endl;
	std::vector<VertexColor> vert;
	std::vector<int> idx;

	_renderUpdateHint &= ~ruhDUST;
}

void GalaxyWnd::UpdateH2()
{
	std::cout << "Updating H2 Regions" << std::endl;
	std::vector<VertexColor> vert;
	std::vector<int> idx;

	_renderUpdateHint &= ~ruhH2;
}

void GalaxyWnd::UpdateStars()
{
	std::cout << "Updating stars" << std::endl;

	std::vector<VertexColor> vert;
	std::vector<int> idx;

	int num = _galaxy.GetNumStars();
	Star* pStars = _galaxy.GetStars();

	float a = 1;
	Color color = { 1, 1, 1, a };

	if (_starRenderType == 2)
		color = { 1, 1, 1, a };

	for (int i = 1; i < num; ++i)
	{
		const Vec2D& pos = pStars[i].pos;
		const Color& col = ColorFromTemperature(pStars[i].temp);
		if (_starRenderType == 1)
		{
			color = {
				col.r * pStars[i].mag,
				col.g * pStars[i].mag,
				col.b * pStars[i].mag,
				a };
		}
		else
		{
			color = { 1, 1, 1, a };
		}

		// todo: Render a small portion of the stars as bright distinct stars
		float size =3;
		if (i < num / 30)
		{
			size = 6;
			color.r = std::min(.2f + color.r, 1.f);
			color.g = std::min(.2f + color.g, 1.f);
			color.b = std::min(.2f + color.b, 1.f);
		}
		
		idx.push_back((int)vert.size());
		vert.push_back({ pos.x, pos.y, 0.0f , color.r, color.g, color.b, color.a });
	}

	_vertStars.Update(vert, idx, GL_POINTS);
	_renderUpdateHint &= ~ruhSTARS;
}

void GalaxyWnd::UpdateAxis()
{
	std::vector<VertexColor> vert;
	std::vector<int> idx;

	GLfloat s = (GLfloat)std::pow(10, (int)(std::log10(_fov / 2)));
	GLfloat l = _fov / 100, p = 0;

	float r = 0.3, g = 0.3, b = 0.3, a = 0.8;
	for (int i = 0; p < _fov; ++i)
	{
		p += s;
		idx.push_back((int)vert.size());
		vert.push_back({ p, -l, 0, r, g, b, a });

		idx.push_back((int)vert.size());
		vert.push_back({ p,  l, 0, r, g, b, a });

		idx.push_back((int)vert.size());
		vert.push_back({ -p, -l, 0, r, g, b, a });

		idx.push_back((int)vert.size());
		vert.push_back({ -p,  0, 0, r, g, b, a });

		idx.push_back((int)vert.size());
		vert.push_back({ -l, p, 0, r, g, b, a });

		idx.push_back((int)vert.size());
		vert.push_back({ 0, p, 0, r, g, b, a });

		idx.push_back((int)vert.size());
		vert.push_back({ -l, -p, 0, r, g, b, a });

		idx.push_back((int)vert.size());
		vert.push_back({ 0, -p, 0, r, g, b, a });
	}

	idx.push_back((int)vert.size());
	vert.push_back({ -_fov, 0, 0, r, g, b, a });

	idx.push_back((int)vert.size());
	vert.push_back({ _fov, 0, 0, r, g, b, a });

	idx.push_back((int)vert.size());
	vert.push_back({ 0, -_fov, 0, r, g, b, a });

	idx.push_back((int)vert.size());
	vert.push_back({ 0, _fov, 0, r, g, b, a });

	_vertAxis.Update(vert, idx, GL_LINES);
	_renderUpdateHint &= ~ruhAXIS;
}

/** \brief Update the density wave vertex buffers
*/
void GalaxyWnd::UpdateDensityWaves()
{
	std::cout << "Density render update request received" << std::endl;

	std::vector<VertexColor> vert;
	std::vector<int> idx;

	//
	// First add the density waves
	//

	int num = 100;
	float dr = _galaxy.GetFarFieldRad() / num;
	for (int i = 0; i <= num; ++i)
	{
		float r = dr * (i + 1);
		AddEllipsisVertices(
			vert,
			idx,
			r,
			r * _galaxy.GetExcentricity(r),
			MathHelper::RAD_TO_DEG * _galaxy.GetAngularOffset(r),
			_galaxy.GetPertN(),
			_galaxy.GetPertAmp(),
			{ 1, 1, 1, 0.2f });
	}

	//
	// Add three circles at the boundaries of core, galaxy and galactic medium
	//

	int pertNum = 0;
	float pertAmp = 0;
	auto r = _galaxy.GetCoreRad();
	AddEllipsisVertices(vert, idx, r, r, 0, pertNum, pertAmp, { 1, 1, 0, 0.5 });

	r = _galaxy.GetRad();
	AddEllipsisVertices(vert, idx, r, r, 0, pertNum, pertAmp, { 0, 1, 0, 0.5 });

	r = _galaxy.GetFarFieldRad();
	AddEllipsisVertices(vert, idx, r, r, 0, pertNum, pertAmp, { 1, 0, 0, 0.5 });

	_vertDensityWaves.Update(vert, idx, GL_LINE_STRIP);
	_renderUpdateHint &= ~ruhDENSITY_WAVES;
}

void GalaxyWnd::Update()
{
	if ((_renderUpdateHint & ruhDENSITY_WAVES) != 0)
		UpdateDensityWaves();

	if ((_renderUpdateHint & ruhAXIS) != 0)
		UpdateAxis();

	if ((_renderUpdateHint & ruhSTARS) != 0)
		UpdateStars();

	if ((_renderUpdateHint & ruhDUST) != 0)
		UpdateDust();

	if ((_renderUpdateHint & ruhH2) != 0)
		UpdateH2();
}

void GalaxyWnd::Render()
{
	static long ct = 0;
	if (!(_flags & (int)DisplayItem::PAUSE))
	{
		++ct;
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Vec3D orient;
	switch (_camOrient)
	{
		// Default orientation
	case 0:
		orient.x = 0;
		orient.y = 1;
		orient.z = 0;
		break;

		// Rotate with galaxy core
	case 1:
	{
		auto& p = _galaxy.GetStarPos(1);
		orient.x = p.x;
		orient.y = p.y;
		orient.z = 0;
	}
	break;

	// Rotate with edge of disk
	case 2:
	{
		auto& p = _galaxy.GetStarPos(2);
		orient.x = p.x;
		orient.y = p.y;
		orient.z = 0;
	}
	break;
	}

	Vec3D lookAt = {0, 0, 0};
	Vec3D pos = {0, 0, 5000};

	SetCamera(pos, lookAt, orient);

	if (!(_flags & (int)DisplayItem::PAUSE))
		_galaxy.SingleTimeStep(100000); // time in years

	if (_flags & (int)DisplayItem::AXIS)
		DrawAxis();

	if (_flags & (int)DisplayItem::DUST)
		DrawDust();

	if (_flags & (int)DisplayItem::H2)
		DrawH2();

	if (_flags & (int)DisplayItem::STARS)
		DrawStars();

	if (_flags & (int)DisplayItem::DENSITY_WAVES)
		DrawDensityWaves();

	if (_flags & (int)DisplayItem::VELOCITY)
		DrawVelocity();

	if (_flags & (int)DisplayItem::HELP)
	{
		DrawHelp();
	}

	SDL_GL_SwapWindow(_pSdlWnd);
	SDL_Delay(1);
}

void GalaxyWnd::AddEllipsisVertices(
	std::vector<VertexColor>& vert,
	std::vector<int>& vertIdx,
	float a,
	float b,
	float angle,
	uint32_t pertNum,
	float pertAmp,
	Color col) const
{
	const int steps = 100;
	const float x = 0;
	const float y = 0;

	// Angle is given by Degree Value
	float beta = -angle * MathHelper::DEG_TO_RAD;
	float sinbeta = sin(beta);
	float cosbeta = cos(beta);

	int firstPointIdx = static_cast<int>(vert.size());
	for (int i = 0; i < 360; i += 360 / steps)
	{
		float alpha = i * MathHelper::DEG_TO_RAD;
		float sinalpha = sin(alpha);
		float cosalpha = cos(alpha);

		GLfloat fx = (GLfloat)(x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta));
		GLfloat fy = (GLfloat)(y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta));

		if (pertNum > 0)
		{
			fx += (GLfloat)((a / pertAmp) * sin(alpha * 2 * pertNum));
			fy += (GLfloat)((a / pertAmp) * cos(alpha * 2 * pertNum));
		}

		vertIdx.push_back((int)vert.size());

		VertexColor vc = { fx, fy, 0, col.r, col.g, col.b, col.a };
		vert.push_back(vc);
	}

	// Close the loop and reset the element index array
	vertIdx.push_back(firstPointIdx);
	vertIdx.push_back(0xFFFF);
}

void GalaxyWnd::DrawVelocity()
{
	Star* pStars = _galaxy.GetStars();

	double dt_in_sec = _galaxy.GetTimeStep() * MathHelper::SEC_PER_YEAR;
	glPointSize(1);
	glColor3f(0.5f, 0.7f, 0.5f);
	glBegin(GL_POINTS);
	for (int i = 0; i < _galaxy.GetNumStars(); ++i)
	{
		const Vec2D& vel = pStars[i].vel;
		double r = pStars[i].a; 

		// umrechnen in km/s
		double v = sqrt(vel.x * vel.x + vel.y * vel.y);   // pc / timestep
		v /= dt_in_sec;          // v in pc/sec
		v *= MathHelper::PC_TO_KM; // v in km/s

		glVertex3f((float)r, (float)v * 10, 0.0f);
	}
	glEnd();
}

void GalaxyWnd::DrawStars()
{
//	_vertStars.Draw(_matView, _matProjection);
	
	glBindTexture(GL_TEXTURE_2D, _texStar);

	float maxSize = 0.0f;
	glGetFloatv(GL_POINT_SIZE_MAX, &maxSize);
	glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, maxSize);
	glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0f);
	glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

	glEnable(GL_POINT_SPRITE_ARB);
	glEnable(GL_TEXTURE_2D);       // point sprite texture support
	glEnable(GL_BLEND);            // soft blending of point sprites
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	int num = _galaxy.GetNumStars();
	Star* pStars = _galaxy.GetStars();


	glPointSize(3); //4
	glBegin(GL_POINTS);

	if (_starRenderType == 2)
		glColor3f(1, 1, 1);

	// Render all Stars from the stars array
	for (int i = 1; i < num; ++i)
	{
		const Vec2D& pos = pStars[i].pos;
		const Color& col = ColorFromTemperature(pStars[i].temp);
		if (_starRenderType == 1)
		{
			glColor3f(
				(GLfloat)col.r * pStars[i].mag,
				(GLfloat)col.g * pStars[i].mag,
				(GLfloat)col.b * pStars[i].mag);
		}
		glVertex3f(pos.x, pos.y, 0.0f);

	}
	glEnd();

	// Render a portion of the stars as bright distinct stars
	glPointSize(6); //4
	glBegin(GL_POINTS);


	for (int i = 1; i < num / 30; ++i)
	{
		const Vec2D& pos = pStars[i].pos;
		const Color& col = ColorFromTemperature(pStars[i].temp);
		if (_starRenderType == 1)
		{
			glColor3f(
				0.2f + col.r * pStars[i].mag,
				0.2f + col.g * pStars[i].mag,
				0.2f + col.b * pStars[i].mag);
		}
		glVertex3f(pos.x, pos.y, 0.0f);

	}
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

void GalaxyWnd::DrawDust()
{
	glBindTexture(GL_TEXTURE_2D, _texStar);

	float maxSize = 0.0f;
	glGetFloatv(GL_POINT_SIZE_MAX_ARB, &maxSize);
	glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, maxSize);
	glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0f);
	glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

	glEnable(GL_POINT_SPRITE_ARB);
	glEnable(GL_TEXTURE_2D);       // point sprite texture support
	glEnable(GL_BLEND);            // soft blending of point sprites
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	const Star* pDust = _galaxy.GetDust();
	int num = _galaxy.GetNumDust();

	// size 70 looks ok when the fov is 28174
	glPointSize(std::min((float)(_galaxy.GetDustRenderSize() * 28174 / _fov), maxSize));
	glBegin(GL_POINTS);

	for (int i = 0; i < num; ++i)
	{
		const Vec2D& pos = pDust[i].pos;
		const Color& col = ColorFromTemperature(pDust[i].temp);
		glColor3f(
			col.r * (float)pDust[i].mag,
			col.g * (float)pDust[i].mag,
			col.b * (float)pDust[i].mag);
		glVertex3f(pos.x, pos.y, 0.0f);

	}
	glEnd();

	glDisable(GL_POINT_SPRITE_ARB);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void GalaxyWnd::DrawH2()
{
	glBindTexture(GL_TEXTURE_2D, _texStar);

	float maxSize = 0.0f;
	glGetFloatv(GL_POINT_SIZE_MAX_ARB, &maxSize);
	glPointParameterfARB(GL_POINT_SIZE_MAX_ARB, maxSize);
	glPointParameterfARB(GL_POINT_SIZE_MIN_ARB, 1.0f);
	glTexEnvf(GL_POINT_SPRITE_ARB, GL_COORD_REPLACE_ARB, GL_TRUE);

	glEnable(GL_POINT_SPRITE_ARB);
	glEnable(GL_TEXTURE_2D);       // point sprite texture support
	glEnable(GL_BLEND);            // soft blending of point sprites
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);


	Star* pH2 = _galaxy.GetH2();
	int num = _galaxy.GetNumH2();

	for (int i = 0; i < num; ++i)
	{
		int k1 = 2 * i;
		int k2 = 2 * i + 1;

		const Vec2D& p1 = pH2[k1].pos;
		const Vec2D& p2 = pH2[k2].pos;

		float dst = sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
		float size = ((1000 - dst) / 10) - 50;
		if (size < 1)
			continue;

		glPointSize(2 * size);
		glBegin(GL_POINTS);
		const Color& col = ColorFromTemperature(pH2[k1].temp);
		glColor3f(
			col.r * pH2[i].mag * 2.0f,
			col.g * pH2[i].mag * 0.5f,
			col.b * pH2[i].mag * 0.5f);
		glVertex3f(p1.x, p1.y, 0.0f);
		glEnd();

		glPointSize(size / 6);
		glBegin(GL_POINTS);
		glColor3f(1, 1, 1);
		glVertex3f(p1.x, p1.y, 0.0f);
		glEnd();
	}


	glDisable(GL_POINT_SPRITE_ARB);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
}

void GalaxyWnd::DrawDensityWaves()
{
	_vertDensityWaves.Draw(_matView, _matProjection);

	// Captions (immer noch im immediate mode!)
	glColor3f(1, 1, 0);
	DrawText(_pFont, TextCoords::Model, 0, _galaxy.GetCoreRad() + 500, "Core");
	glColor3f(0, 1, 0);
	DrawText(_pFont, TextCoords::Model, 0, _galaxy.GetRad() + 500, "Disk");
	glColor3f(1, 0, 0);
	DrawText(_pFont, TextCoords::Model, 0, _galaxy.GetFarFieldRad() + 500, "Intergalactic medium");
}

void GalaxyWnd::DrawAxis()
{
	_vertAxis.Draw(_matView, _matProjection);

	glColor3f((GLfloat)0.3, (GLfloat)0.3, (GLfloat)0.3);

	GLfloat s = (GLfloat)std::pow(10, (int)(std::log10(_fov / 2)));
	GLfloat l = _fov / 100, p = 0;

	for (int i = 0; p < _fov; ++i)
	{
		p += s;
		if (i % 2 == 0)
		{
			DrawText(_pFont, TextCoords::Model, p - l, -4 * l, "%2.0f", p);
		}
		else
		{
			glRasterPos2f(p - l, 2 * l);
			DrawText(_pFont, TextCoords::Model, p - l, 2 * l, "%2.0f", p);
		}
	}
}

void GalaxyWnd::DrawHelp()
{
	float x0 = 10, y0 = 60, dy = 20;
	int line = 0;

	glColor3f(0.8f, 0.8f, 1.0f);
	float y = y0 - 60;
	DrawText(_pFontCaption, TextCoords::Window, x0, y0 - 60, "Spiral Galaxy Simulator");

	y0 = 60; // _height - 540;
	float dy1 = (float)TTF_FontHeight(_pFont) + 8;
	float dy2 = (float)TTF_FontHeight(_pSmallFont) + 6;

	glColor4f(1, 1, 1, 0.6f);
	y = y0;	  DrawText(_pFont, TextCoords::Window, x0, y, "Simulation Controls:");
	y += dy1; DrawText(_pSmallFont, TextCoords::Window, x0, y, "FPS:  %d", GetFPS());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "Time: %2.2e y", _galaxy.GetTime());

	y += dy1; DrawText(_pFont, TextCoords::Window, x0, y, "Geometry:");
	y += dy1; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[r],[f] RadCore:     %d pc", (int)_galaxy.GetCoreRad());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[t],[g] RadGalaxy:   %d pc", (int)_galaxy.GetRad());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "        RadFarField: %d pc", (int)_galaxy.GetFarFieldRad());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[q],[a] ExInner:     %2.2f", _galaxy.GetExInner());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[w],[s] ExOuter:     %2.2f", _galaxy.GetExOuter());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[e],[d] AngOff:      %1.4g deg/pc", _galaxy.GetAngularOffset());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[+],[-] FoV:         %1.2f pc", _fov);

	y += dy1; DrawText(_pFont, TextCoords::Window, x0, y, "Spiral Arms:");
	y += dy1; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[Home],[End]    Num pert:  %d", _galaxy.GetPertN());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[PG_UP],[PG_DN] pertDamp:  %1.2f", _galaxy.GetPertAmp());

	y += dy1; DrawText(_pFont, TextCoords::Window, x0, y, "Display Features:");
	y += dy1; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[b],[n]  Dust render size:  %2.2lf", _galaxy.GetDustRenderSize());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[F1] Help Screen");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[F2] Toggle Axis");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[F3] Toggle Stars");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[F4] Toggle Dust");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[F5] Toggle H2 Regions");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[F6] Toggle Density Waves");

	y += dy1; DrawText(_pFont, TextCoords::Window, x0, y, "Physics:");
	y += dy1; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[z],[h] Base Temp.:  %2.2lf K", _galaxy.GetBaseTemp());
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[m] Toggle Dark Matter: %s", _galaxy.HasDarkMatter() ? "ON" : "OFF");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[v] Display Velocity Curve: %s", ((_flags & (int)DisplayItem::VELOCITY) != 0) ? "ON" : "OFF");

	y += dy1; DrawText(_pFont, TextCoords::Window, x0, y, "Predefined Galaxies:");
	y += dy1; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[KP1] - [KP8] Predefined Galaxies");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[Pause]       Halt simulation");

	y += dy1; DrawText(_pFont, TextCoords::Window, x0, y, "Camera Control:");
	y += dy1; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[1] fixed");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[2] rotating with core");
	y += dy2; DrawText(_pSmallFont, TextCoords::Window, x0, y, "[3] rotating with outer disc");

	glColor4f(0.8f, 0.8f, 0.8f, 0.3f);
	DrawText(_pFont, TextCoords::Window, (float)_width - 180, (float)_height - 30, " (C) 2020 Ingo Berg");
}

Color GalaxyWnd::ColorFromTemperature(float temp) const
{
	int idx = (int)((temp - _t0) / (_t1 - _t0) * _colNum);
	idx = std::min(_colNum - 1, idx);
	idx = std::max(0, idx);
	return _col[idx];
}

void GalaxyWnd::OnProcessEvents(Uint32 type)
{
	switch (type)
	{
	case SDL_MOUSEBUTTONDOWN:
		break;

	case SDL_KEYDOWN:
		switch (m_event.key.keysym.sym)
		{
		case SDLK_END:
			_galaxy.SetPertN(std::max(_galaxy.GetPertN() - 1, 0));
			break;

		case SDLK_HOME:
			_galaxy.SetPertN(std::min(_galaxy.GetPertN() + 1, 5));
			break;

		case SDLK_PAGEDOWN:
			_galaxy.SetPertAmp(_galaxy.GetPertAmp() - 2);
			break;

		case SDLK_PAGEUP:
			_galaxy.SetPertAmp(_galaxy.GetPertAmp() + 2);
			break;

		case SDLK_1:
			_camOrient = 0;
			break;

		case SDLK_2:
			_camOrient = 1;
			break;

		case SDLK_3:
			_camOrient = 2;
			break;

		case SDLK_q:
			_galaxy.SetExInner(_galaxy.GetExInner() + 0.05f);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_a:
			_galaxy.SetExInner(std::max(_galaxy.GetExInner() - 0.05f, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_w:
			_galaxy.SetExOuter(_galaxy.GetExOuter() + 0.05f);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_s:
			_galaxy.SetExOuter(std::max(_galaxy.GetExOuter() - 0.05f, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_e:
			_galaxy.SetAngularOffset(_galaxy.GetAngularOffset() + 0.00002f);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_d:
			_galaxy.SetAngularOffset(std::max(_galaxy.GetAngularOffset() - 0.00002f, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_r:
			if (_galaxy.GetRad() > _galaxy.GetCoreRad() + 500)
				_galaxy.SetCoreRad(_galaxy.GetCoreRad() + 500);
			break;

		case SDLK_m:
			_galaxy.ToggleDarkMatter();
			break;

		case SDLK_f:
			_galaxy.SetCoreRad(std::max(_galaxy.GetCoreRad() - 500, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_t:
			_galaxy.SetRad(_galaxy.GetRad() + 1000);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_g:
			_galaxy.SetRad(std::max(_galaxy.GetRad() - 1000, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_z:
			_galaxy.SetBaseTemp(std::min(_galaxy.GetBaseTemp() + 100, 15000.0f));
			break;

		case SDLK_h:
			_galaxy.SetBaseTemp(std::max(_galaxy.GetBaseTemp() - 100, 500.0f));
			break;

		case SDLK_b:
			_galaxy.SetDustRenderSize(_galaxy.GetDustRenderSize() - 5);
			break;

		case SDLK_n:
			_galaxy.SetDustRenderSize(_galaxy.GetDustRenderSize() + 5);
			break;

		case  SDLK_F1:
			_flags ^= (int)DisplayItem::HELP;
			break;

		case SDLK_F2:
			_flags ^= (int)DisplayItem::AXIS;
			break;

		case  SDLK_F3:
			if (_starRenderType == 2)
			{
				_starRenderType = 0;
				_flags &= ~(int)DisplayItem::STARS;
			}
			else
			{
				_starRenderType++;
				_flags |= (int)DisplayItem::STARS;
			}
			break;

		case  SDLK_F4:
			_flags ^= (int)DisplayItem::DUST;
			break;

		case  SDLK_F5:
			_flags ^= (int)DisplayItem::H2;
			break;

		case SDLK_F6:
			_flags ^= (int)DisplayItem::DENSITY_WAVES;
			break;

		case  SDLK_v:
			_flags ^= (int)DisplayItem::VELOCITY;
			break;

		case  SDLK_PAUSE:
			_flags ^= (int)DisplayItem::PAUSE;
			break;

		case SDLK_KP_0:
			_galaxy.Reset(
				13000,    // radius of the galaxy
				4000,     // radius of the core
				0.0004f,   // angluar offset of the density wave per parsec of radius
				0.85f,     // excentricity at the edge of the core
				0.95f,      // excentricity at the edge of the disk
				40000,    // total number of stars
				true,     // has dark matter
				2,        // Perturbations per full ellipse
				40,       // Amplitude damping factor of perturbation
				90,
				3600);      // dust render size in pixel
			_fov = 33960;
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_KP_1:
			_galaxy.Reset(
				16000,    // radius of the galaxy
				4000,     // radius of the core
				0.0003f,   // angluar offset of the density wave per parsec of radius
				0.8f,      // excentricity at the edge of the core
				0.85f,     // excentricity at the edge of the disk
				40000,    // total number of stars
				true,     // has dark matter
				0,        // Perturbations per full ellipse
				40,       // Amplitude damping factor of perturbation
				100,
				4500);
			_fov = 46585;
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_KP_2:
			_galaxy.Reset(
				13000,    // radius of the galaxy
				4000,     // radius of the core
				0.00064f,   // angluar offset of the density wave per parsec of radius
				0.9f,      // excentricity at the edge of the core
				0.9f,      // excentricity at the edge of the disk
				40000,
				true,
				0,
				0,
				85,
				4100);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;
		case SDLK_KP_3:
			_galaxy.Reset(
				13000,    // radius of the galaxy
				4000,     // radius of the core
				0.0004f,   // angluar offset of the density wave per parsec of radius
				1.35f,      // excentricity at the edge of the core
				1.05f,      // excentricity at the edge of the disk
				40000,
				true,
				0,
				0,
				70,   // total number of stars
				4500);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_KP_4:
			_galaxy.Reset(
				13000,    // radius of the galaxy
				4500,     // radius of the core
				0.0002f,   // angluar offset of the density wave per parsec of radius
				0.65f,     // excentricity at the edge of the core
				0.95f,      // excentricity at the edge of the disk
				40000,    // total number of stars
				true,     // has dark matter
				3,        // Perturbations per full ellipse
				72,       // Amplitude damping factor of perturbation
				90,      // dust render size in pixel
				4000);
			_fov = 35000;
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

			// Typ SBb
		case SDLK_KP_5:
			_galaxy.Reset(
				15000,    // radius of the galaxy
				4000,     // radius of the core
				0.0003f,   // angluar offset of the density wave per parsec of radius
				1.45f,     // excentricity at the edge of the core
				1.0f,      // excentricity at the edge of the disk
				40000,
				true,
				0,
				0,
				100,
				4500);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_KP_6:
			_galaxy.Reset(
				14000,    // radius of the galaxy
				12500,    // radius of the core
				0.0002f,  // angluar offset of the density wave per parsec of radius
				0.65f,    // excentricity at the edge of the core
				0.95f,    // excentricity at the edge of the disk
				40000,    // total number of stars
				true,     // has dark matter
				3,        // Perturbations per full ellipse
				72,       // Amplitude damping factor of perturbation
				85,       // dust render size in pixel
				2200);
			_fov = 36982;
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;


		case SDLK_KP_7:
			_galaxy.Reset(
				13000,    // radius of the galaxy
				1500,     // radius of the core
				0.0004f,  // angluar offset of the density wave per parsec of radius
				1.1f,     // excentricity at the edge of the core
				1.0f,     // excentricity at the edge of the disk
				40000,    // total number of stars
				true,     // has dark matter
				1,        // Perturbations per full ellipse
				20,       // Amplitude damping factor of perturbation
				80,
				2800);    // dust render size in pixel
			_fov = 41091;
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;


		case SDLK_KP_8:
			_galaxy.Reset(
				13000,    // radius of the galaxy
				4000,     // radius of the core
				0.0004f,  // angluar offset of the density wave per parsec of radius
				0.85f,    // excentricity at the edge of the core
				0.95f,    // excentricity at the edge of the disk
				40000,    // total number of stars
				true,     // has dark matter
				1,        // Perturbations per full ellipse
				20,       // Amplitude damping factor of perturbation
				80,       // dust render size in pixel
				4500);
			_fov = 41091;
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhH2;
			break;

		case SDLK_PLUS:
		case SDLK_KP_PLUS:
			ScaleAxis(0.9f);
			SetCameraOrientation({0, 1, 0});
			_renderUpdateHint |= ruhAXIS;
			break;

		case SDLK_MINUS:
		case SDLK_KP_MINUS:
			ScaleAxis(1.1f);
			SetCameraOrientation({0, 1, 0});
			_renderUpdateHint |= ruhAXIS;
			break;

		default:
			break;
		}

		break;
	}
}
