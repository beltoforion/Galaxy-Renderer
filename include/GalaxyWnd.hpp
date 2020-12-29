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
		AXIS          = 1 << 1,
		STARS         = 1 << 2,
		PAUSE         = 1 << 3,
		HELP          = 1 << 4,
		DENSITY_WAVES = 1 << 5,
		VELOCITY      = 1 << 6,
		DUST          = 1 << 7,
		H2            = 1 << 8,
		FILAMENTS     = 1 << 9,
	};

	enum RenderUpdateHint : uint32_t
	{
		ruhNONE = 0,
		ruhDENSITY_WAVES = 1 << 1,
		ruhAXIS = 1 << 2,
		ruhSTARS = 1 << 3,
		ruhDUST = 1 << 4,
		ruhCREATE_VELOCITY_CURVE = 1 << 5,
		ruhCREATE_TEXT = 1 << 7
	};

	float _time;
	uint32_t _flags;	///< The display flags
	Galaxy _galaxy;

	uint32_t _renderUpdateHint;

	VertexBufferLines _vertDensityWaves;
	VertexBufferLines _vertAxis;
	VertexBufferLines _vertVelocityCurve;
	VertexBufferStars _vertStars;

	TextBuffer _textHelp;
	TextBuffer _textAxisLabel;
	TextBuffer _textGalaxyLabels;

	std::vector<Galaxy::GalaxyParam> _predefinedGalaxies;

	static const float TimeStepSize;

	GalaxyWnd(const GalaxyWnd& orig);

	void AddEllipsisVertices(
		std::vector<VertexColor>& vert, 
		std::vector<int>& vertIdx, 
		float a,
		float b,
		float angle,
		uint32_t pertNum, 
		float pertAmp,
		Color col) const;

	void UpdateDensityWaves();
	void UpdateAxis();
	void UpdateStars();
	void UpdateVelocityCurve();
	void UpdateText();
};

