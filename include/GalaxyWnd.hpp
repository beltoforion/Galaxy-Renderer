#pragma once

#include <cstdint>
#include <vector>
#include <string>
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

	/// A galaxy configuration loaded from a text file in the "presets" folder.
	/// Values a file does not mention keep their current setting when applied.
	struct GalaxyPreset
	{
		std::string name;           ///< file name without extension
		Galaxy::GalaxyParam param;
		float fov = 0;              ///< field of view; 0 = keep the current one
		float h2SizeMax = 0;        ///< H2 render size; 0 = keep the current one
		float h2Threshold = 0;      ///< H2 ignition threshold; 0 = keep the current one
		uint32_t displayFlags = 0;  ///< display feature bits set by the file
		uint32_t displayMask = 0;   ///< display feature bits the file specified at all
	};

	std::vector<GalaxyPreset> _presets;
	int _selectedPreset = -1;       ///< index into _presets shown in the combo box
	char _presetSaveName[64] = {};  ///< edit buffer of the "Save as" input field

	void LoadPresets();
	void ApplyPreset(int idx);
	bool SavePreset(const std::string& name);

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

	// Framerate limiting
	float _h2SizeMax = 100.0f;      ///< Point size of a fully ignited H2 region (px)
	float _h2Threshold = 1.2f;      ///< Density wave crowding factor at which H2 regions ignite
	bool _limitFramerate = true;    ///< Cap the render loop to _targetFps
	int _targetFps = 60;            ///< Target framerate when limiting is on
	uint32_t _lastFrameTicks = 0;   ///< SDL_GetTicks() at the previous frame

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
		int   numStars = 0;
		int   numDust = 0;
		int   numH2 = 0;
		bool  hasBar = false;
		int   barRadius = 3000;
		float barEx = 0.55f;
	} _ui;
};

