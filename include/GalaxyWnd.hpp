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
#include "VideoRecorder.hpp"


/** \brief Main window of th n-body simulation. */
class GalaxyWnd final : public SDLWindow
{
public:

	GalaxyWnd();
	~GalaxyWnd();

	void SetVideoOptions(int width, int height, int fps);

protected:
	virtual void Render() override;
	virtual void Update() override;

	virtual void OnProcessEvents(Uint32 type) override;

	void InitGL() noexcept (false) override;
	void InitSimulation() override;

private:

	enum class DisplayItem : uint32_t
	{
		NONE          = 0,
		AXIS          = 1 << 1,
		STARS         = 1 << 2,
		PAUSE         = 1 << 3,
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
	};

	float _time;
	uint32_t _flags;	///< The display flags
	Galaxy _galaxy;

	uint32_t _renderUpdateHint;

	VertexBufferLines _vertDensityWaves;
	VertexBufferLines _vertAxis;
	VertexBufferLines _vertVelocityCurve;
	VertexBufferStars _vertStars;

	TextBuffer _textAxisLabel;
	TextBuffer _textGalaxyLabels;

	VideoRecorder _videoRecorder;
	int _videoWidth;
	int _videoHeight;
	int _videoFps;

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

	void RenderScene(glm::mat4& matView, glm::mat4& matProjection, bool overlays);
	void RenderUI();
	void ToggleVideoRecording();

	// --- Dear ImGui control panel state -----------------------------------
	bool _showUi = true;   ///< Visibility of the control panel (toggled with F1)

	// Cache for parameters whose edit triggers an expensive star/dust rebuild.
	// Widgets bind to these; the model is only updated when a widget is
	// released (see RenderUI). Kept in sync with the model while no widget is
	// being dragged, so keyboard shortcuts stay reflected in the panel.
	struct UiCache
	{
		int   radCore = 0;
		int   radGalaxy = 0;
		float exInner = 0.0f;
		float exOuter = 0.0f;
		float angOff = 0.0f;
		float baseTemp = 0.0f;
		float fov = 0.0f;
	} _ui;
};

