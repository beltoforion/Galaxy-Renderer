#pragma once

#include <cstdint>
#include <vector>
#include <SDL_ttf.h>

#include "SDLWnd.hpp"
#include "Galaxy.hpp"
#include "VertexBuffer.hpp"

/** \brief Main window of th n-body simulation. */
class GalaxyWnd final : public SDLWindow
{
public:

	GalaxyWnd();
	~GalaxyWnd();

protected:
	virtual void Render();
	virtual void Update();

	virtual void OnProcessEvents(Uint32 type);

	void InitGL() override;
	void InitSimulation() override;

private:

	enum class DisplayItem : uint32_t
	{
		NONE          = 0,
		AXIS          = 0b000000001,
		STARS         = 0b000000010,
		PAUSE         = 0b000000100,
		HELP          = 0b000001000,
		ROI           = 0b000010000,
		DENSITY_WAVES = 0b000100000,
		VELOCITY      = 0b001000000,
		DUST          = 0b010000000,
		H2            = 0b100000000
	};

	enum RenderUpdateHint : uint32_t
	{
		ruhNONE = 0,
		ruhDENSITY_WAVES = 1 << 1
	};

	struct Color
	{
		double r;
		double g;
		double b;
	};

	GalaxyWnd(const GalaxyWnd& orig);

	void DrawStars();
	void DrawDust();
	void DrawH2();
	void DrawStat();
	void DrawHelp();
	void DrawGalaxyRadii();
	void DrawAxis(const Vec2D& origin);
	void DrawDensityWaves(int num, double rad);
	void DrawVelocity();
	
	void AddEllipsisVertices(std::vector<VertexColor>& vert, std::vector<int>& vertIdx, double a, double b, double angle, uint32_t pertNum, double pertAmp, float red, float green, float blue) const;

	void DrawEllipsis(double a, double b, double angle, GLfloat width = 2);
	Color ColorFromTemperature(double temp) const;

	void UpdateDensityWaves();

	int _camOrient;    ///< Index of the camera orientation to use
	int _starRenderType;
	double _roi;       ///< Radius of the region of interest
	uint32_t _flags;   ///< The display flags

	Galaxy _galaxy;

	// Star color management
	int _colNum;
	double _t0, _t1, _dt;
	Color _col[200];
	
	uint32_t _renderUpdateHint;

	VertexBuffer _vertDensityWaves;

	TTF_Font *_pSmallFont;
	TTF_Font *_pFont;
	TTF_Font *_pFontCaption;
};
