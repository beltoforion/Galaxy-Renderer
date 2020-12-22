#pragma once

#include <cstdint>
#include <vector>
#include <SDL_ttf.h>

#include "SDLWnd.hpp"
#include "Galaxy.hpp"
#include "VertexBufferBase.hpp"
#include "VertexBufferLines.hpp"
#include "VertexBufferStars.hpp"
#include "TextBuffer.hpp"

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

	void InitGL() noexcept (false) override;
	void InitSimulation() override;

private:

	enum class DisplayItem : uint32_t
	{
		NONE          = 0,
		AXIS          = 0b000000001,
		STARS         = 0b000000010,
		PAUSE         = 0b000000100,
		HELP          = 0b000001000,
		DENSITY_WAVES = 0b000100000,
		VELOCITY      = 0b001000000,
		DUST          = 0b010000000,
		H2            = 0b100000000
	};

	enum RenderUpdateHint : uint32_t
	{
		ruhNONE = 0,
		ruhDENSITY_WAVES = 1 << 1,
		ruhAXIS = 1 << 2,
		ruhSTARS = 1 << 3,
		ruhDUST = 1 << 4,
		ruhH2 = 1 << 5,
		ruhCREATE_VELOCITY_CURVE = 1 << 6,
		ruhCREATE_TEXT = 1 << 7
	};

	GalaxyWnd(const GalaxyWnd& orig);

	void DrawDust();
	void DrawH2();
	
	void AddEllipsisVertices(
		std::vector<VertexColor>& vert, 
		std::vector<int>& vertIdx, 
		float a,
		float b,
		float angle,
		uint32_t pertNum, 
		float pertAmp,
		Color col) const;

	Color ColorFromTemperature(float temp) const;

	void UpdateDensityWaves();
	void UpdateAxis();
	void UpdateStars();
	void UpdateDust();
	void UpdateH2();
	void UpdateVelocityCurve(bool updateOnly);
	void UpdateText();

	uint32_t _flags;	///< The display flags

	Galaxy _galaxy;

	// Star color management
	int _colNum;
	double _t0, _t1, _dt;
	Color _col[200];
	
	uint32_t _renderUpdateHint;
	bool _useDirectMode;

	VertexBufferLines _vertDensityWaves;
	VertexBufferLines _vertAxis;
	VertexBufferLines _vertVelocityCurve;
	VertexBufferStars _vertStars;
	VertexBufferStars _vertDust;

	TextBuffer _textHelp;
	TextBuffer _textAxisLabel;
	TextBuffer _textGalaxyLabels;

	std::vector<Galaxy::GalaxyParam> _predefinedGalaxies;

	GLuint _texStar;

	static const float TimeStepSize;
};

