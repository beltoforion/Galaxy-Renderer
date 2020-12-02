#pragma once

#include <stdint.h>
#include <fstream>
#include <SDL_ttf.h>

#include "SDLWnd.h"
#include "Galaxy.h"


/** \brief Main window of th n-body simulation. */
class GalaxyWnd final : public SDLWindow
{
public:

	GalaxyWnd();

	virtual void Render();
	virtual void OnProcessEvents(Uint32 type);


protected:
	void InitGL() override;
	void InitSimulation() override;

private:
	enum EDisp
	{
		dspNONE = 0,
		dspAXIS = 1 << 0,
		dspSTARS = 1 << 1,
		dspSTAT = 1 << 2,
		dspPAUSE = 1 << 3,
		dspHELP = 1 << 4,
		dspROI = 1 << 5,
		dspDENSITY_WAVES = 1 << 6,
		dspRADII = 1 << 7,
		dspVELOCITY = 1 << 8,
		dspDUST = 1 << 9,
		dspH2 = 1 << 10
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
	void DrawEllipsis(double a, double b, double angle);
	Color ColorFromTemperature(double temp) const;

	int m_camOrient;    ///< Index of the camera orientation to use
	int m_starRenderType;
	double m_roi;       ///< Radius of the region of interest
	uint32_t m_flags;   ///< The display flags

	Galaxy m_galaxy;

	// Star color management
	int m_colNum;
	double m_t0, m_t1, m_dt;
	Color m_col[200];

	TTF_Font* _pFont;
	TTF_Font* _pFontCaption;
};
