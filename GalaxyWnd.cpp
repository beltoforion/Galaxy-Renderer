#include "GalaxyWnd.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <cstddef>
#include <cstdio>
#include <cfloat>
#include <ctime>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "Helper.hpp"
#include "Types.hpp"

const float GalaxyWnd::TimeStepSize = 100000.0f;

GalaxyWnd::GalaxyWnd()
	: SDLWindow()
	, _flags((int)DisplayItem::STARS | (int)DisplayItem::AXIS | (int)DisplayItem::DUST | (int)DisplayItem::H2 | (int)DisplayItem::FILAMENTS)
	, _galaxy()
	, _renderUpdateHint(ruhDENSITY_WAVES | ruhAXIS | ruhSTARS | ruhDUST | ruhCREATE_VELOCITY_CURVE)
	, _vertDensityWaves(2)
	, _vertAxis()
	, _vertVelocityCurve(1, GL_DYNAMIC_DRAW)
	, _vertStars(GL_FUNC_ADD, GL_ONE)
	, _textAxisLabel()
	, _textGalaxyLabels()
	, _videoRecorder()
	, _videoWidth(3840)
	, _videoHeight(2160)
	, _videoFps(60)
{
	LoadPresets();
}

void GalaxyWnd::LoadPresets()
{
	_presets.clear();

	namespace fs = std::filesystem;
	const fs::path dir = "presets";
	std::error_code ec;
	if (fs::is_directory(dir, ec))
	{
		for (const auto& entry : fs::directory_iterator(dir, ec))
		{
			if (!entry.is_regular_file() || entry.path().extension() != ".txt")
				continue;

			GalaxyPreset preset;
			preset.name = entry.path().stem().string();
			preset.param = { 13000, 4000, .0004f, .85f, .95f, 40000, true, 2, 40, 70, 4000 };

			std::ifstream file(entry.path());
			std::string line;
			while (std::getline(file, line))
			{
				const auto sep = line.find('=');
				if (line.empty() || line[0] == '#' || sep == std::string::npos)
					continue;

				std::string key = line.substr(0, sep);
				key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
				const float val = std::strtof(line.c_str() + sep + 1, nullptr);

				auto displayFlag = [&preset, val](DisplayItem item)
				{
					preset.displayMask |= (uint32_t)item;
					if (val != 0)
						preset.displayFlags |= (uint32_t)item;
				};

				auto& p = preset.param;
				if      (key == "radius")         p.rad = val;
				else if (key == "coreRadius")     p.radCore = val;
				else if (key == "angularOffset")  p.deltaAng = val;
				else if (key == "exInner")        p.ex1 = val;
				else if (key == "exOuter")        p.ex2 = val;
				else if (key == "numStars")       p.numStars = (int)val;
				else if (key == "numDust")        p.numDust = (int)val;
				else if (key == "numH2")          p.numH2 = (int)val;
				else if (key == "hasDarkMatter")  p.hasDarkMatter = val != 0;
				else if (key == "pertN")          p.pertN = (int)val;
				else if (key == "pertAmp")        p.pertAmp = val;
				else if (key == "dustRenderSize") p.dustRenderSize = val;
				else if (key == "baseTemp")       p.baseTemp = val;
				else if (key == "fov")            preset.fov = val;
				else if (key == "h2SizeMax")      preset.h2SizeMax = val;
				else if (key == "h2Threshold")    preset.h2Threshold = val;
				else if (key == "showStars")         displayFlag(DisplayItem::STARS);
				else if (key == "showAxis")          displayFlag(DisplayItem::AXIS);
				else if (key == "showDust")          displayFlag(DisplayItem::DUST);
				else if (key == "showH2")            displayFlag(DisplayItem::H2);
				else if (key == "showFilaments")     displayFlag(DisplayItem::FILAMENTS);
				else if (key == "showDensityWaves")  displayFlag(DisplayItem::DENSITY_WAVES);
				else if (key == "showVelocityCurve") displayFlag(DisplayItem::VELOCITY);
			}
			_presets.push_back(preset);
		}

		std::sort(_presets.begin(), _presets.end(),
			[](const GalaxyPreset& a, const GalaxyPreset& b) { return a.name < b.name; });
	}

	// No preset files found: fall back to the built-in classics so the combo
	// box and the numpad shortcuts keep working.
	if (_presets.empty())
	{
		_presets = {
			{ "Galaxy 1", { 13000,  4000, .0004f,  .85f,  .95f, 40000, true, 2, 40,  90, 3600 }, 33960 },
			{ "Galaxy 2", { 16000,  4000, .0003f,   .8f,  .85f, 40000, true, 0, 40, 100, 4500 }, 46585 },
			{ "Galaxy 3", { 13000,  4000, .00064f,  .9f,   .9f, 40000, true, 0,  0,  85, 4100 }, 0 },
			{ "Galaxy 4", { 13000,  4000, .0004f, 1.35f, 1.05f, 40000, true, 0,  0,  70, 4500 }, 0 },
			{ "Galaxy 5", { 13000,  4500, .0002f,  .65f,  .95f, 40000, true, 3, 72,  90, 4000 }, 35000 },
			{ "Galaxy 6", { 15000,  4000, .0003f, 1.45f,  1.0f, 40000, true, 0,  0, 100, 4500 }, 0 },
			{ "Galaxy 7", { 14000, 12500, .0002f, 0.65f, 0.95f, 40000, true, 3, 72,  85, 2200 }, 36982 },
			{ "Galaxy 8", { 13000,  1500, .0004f,  1.1f,  1.0f, 40000, true, 1, 20,  80, 2800 }, 41091 },
			{ "Galaxy 9", { 13000,  4000, .0004f,  .85f,  .95f, 40000, true, 1, 20,  80, 4500 }, 41091 },
		};
	}
}

void GalaxyWnd::ApplyPreset(int idx)
{
	if (idx < 0 || idx >= (int)_presets.size())
		return;

	const auto& preset = _presets[idx];
	_galaxy.Reset(preset.param);
	if (preset.fov > 0)
	{
		_fov = preset.fov;
		AdjustCamera();
		SetCameraOrientation({ 0, 1, 0 });
	}
	if (preset.h2SizeMax > 0)
		_h2SizeMax = preset.h2SizeMax;
	if (preset.h2Threshold > 0)
		_h2Threshold = preset.h2Threshold;
	_flags = (_flags & ~preset.displayMask) | (preset.displayFlags & preset.displayMask);
	_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST | ruhCREATE_VELOCITY_CURVE | ruhAXIS;
	_selectedPreset = idx;
}

bool GalaxyWnd::SavePreset(const std::string& name)
{
	// Keep only file-system friendly characters of the requested name.
	std::string safeName;
	for (char c : name)
	{
		if (std::isalnum((unsigned char)c) || c == ' ' || c == '-' || c == '_' || c == '.')
			safeName += c;
	}
	if (safeName.empty())
		return false;

	namespace fs = std::filesystem;
	std::error_code ec;
	fs::create_directories("presets", ec);

	std::ofstream file(fs::path("presets") / (safeName + ".txt"));
	if (!file)
		return false;

	file << "# Galaxy preset\n";
	file << "radius=" << _galaxy.GetRad() << "\n";
	file << "coreRadius=" << _galaxy.GetCoreRad() << "\n";
	file << "angularOffset=" << _galaxy.GetAngularOffset() << "\n";
	file << "exInner=" << _galaxy.GetExInner() << "\n";
	file << "exOuter=" << _galaxy.GetExOuter() << "\n";
	file << "numStars=" << _galaxy.GetNumStars() << "\n";
	file << "numDust=" << _galaxy.GetNumDust() << "\n";
	file << "numH2=" << _galaxy.GetNumH2() << "\n";
	file << "hasDarkMatter=" << (_galaxy.HasDarkMatter() ? 1 : 0) << "\n";
	file << "pertN=" << _galaxy.GetPertN() << "\n";
	file << "pertAmp=" << _galaxy.GetPertAmp() << "\n";
	file << "dustRenderSize=" << _galaxy.GetDustRenderSize() << "\n";
	file << "baseTemp=" << _galaxy.GetBaseTemp() << "\n";
	file << "fov=" << _fov << "\n";
	file << "h2SizeMax=" << _h2SizeMax << "\n";
	file << "h2Threshold=" << _h2Threshold << "\n";
	file << "showStars=" << ((_flags & (uint32_t)DisplayItem::STARS) ? 1 : 0) << "\n";
	file << "showAxis=" << ((_flags & (uint32_t)DisplayItem::AXIS) ? 1 : 0) << "\n";
	file << "showDust=" << ((_flags & (uint32_t)DisplayItem::DUST) ? 1 : 0) << "\n";
	file << "showH2=" << ((_flags & (uint32_t)DisplayItem::H2) ? 1 : 0) << "\n";
	file << "showFilaments=" << ((_flags & (uint32_t)DisplayItem::FILAMENTS) ? 1 : 0) << "\n";
	file << "showDensityWaves=" << ((_flags & (uint32_t)DisplayItem::DENSITY_WAVES) ? 1 : 0) << "\n";
	file << "showVelocityCurve=" << ((_flags & (uint32_t)DisplayItem::VELOCITY) ? 1 : 0) << "\n";
	return file.good();
}


void GalaxyWnd::SetVideoOptions(int width, int height, int fps)
{
	_videoWidth = width;
	_videoHeight = height;
	_videoFps = fps;
}

GalaxyWnd::~GalaxyWnd()
{
	// Shut down Dear ImGui while the GL context is still valid (before the
	// base SDLWindow destructor calls SDL_Quit).
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	_videoRecorder.Stop();
	_vertDensityWaves.Release();
	_vertAxis.Release();
	_vertVelocityCurve.Release();
	_vertStars.Release();
}

void GalaxyWnd::InitGL() noexcept(false)
{
	// GL initialization
	glViewport(0, 0, GetWidth(), GetHeight());

	_vertDensityWaves.Initialize();
	_vertAxis.Initialize();
	_vertVelocityCurve.Initialize();
	_vertStars.Initialize();

	// Font initialization
	_textAxisLabel.Initialize();
	_textGalaxyLabels.Initialize();

	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, .0f, 0.08f, 0.0f);
	SetCameraOrientation({ 0, 1, 0 });

	// Dear ImGui initialization (context is created by SDLWindow::Init()).
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(_pSdlWnd, _sdcGlContext);
	ImGui_ImplOpenGL3_Init("#version 330");
}

void GalaxyWnd::InitSimulation()
{
	_galaxy.Reset({
			13000,		// radius of the galaxy
			4000,		// radius of the core
			0.0004f,	// angluar offset of the density wave per parsec of radius
			0.85f,		// excentricity at the edge of the core
			0.95f,		// excentricity at the edge of the disk
			100000,		// total number of stars
			true,		// has dark matter
			2,			// Perturbations per full ellipse
			40,			// Amplitude damping factor of perturbation
			70,			// dust render size in pixel
			4000 });
}

void GalaxyWnd::UpdateStars()
{
	std::vector<VertexStar> vert;
	std::vector<int> idx;

	const auto& stars = _galaxy.GetStars();

	float a = 1;
	Color color = { 1, 1, 1, a };

	for (int i = 1; i < stars.size(); ++i)
	{
		const Color& col = Helper::ColorFromTemperature(stars[i].temp);
		color = { col.r, col.g, col.b, a };

		idx.push_back((int)vert.size());
		vert.push_back({ stars[i], color });
	}

	_vertStars.CreateBuffer(vert, idx, GL_POINTS);
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

	_vertAxis.CreateBuffer(vert, idx, GL_LINES);

	//
	// Update Axis Labels
	//

	_textAxisLabel.BeginUpdate();

	s = (GLfloat)std::pow(10, (int)(std::log10(_fov / 2)));
	l = _fov / 100, p = 0;

	for (int i = 0; p < _fov; ++i)
	{
		p += s;
		if (i % 2 == 0)
		{
			_textAxisLabel.AddText(1, GetWindowPos(p - l, -4.f * l, 0), "%2.0f", p);
		}
		else
		{
			_textAxisLabel.AddText(1, GetWindowPos(p - l, 2 * l, 0), "%2.0f", p);
		}
	}
	_textAxisLabel.EndUpdate();

	_renderUpdateHint &= ~ruhAXIS;
}

void GalaxyWnd::UpdateVelocityCurve()
{
	// I don't need every star for the curve.
	const auto& stars = _galaxy.GetStars();

	std::vector<VertexColor> vert;
	vert.reserve(1000);

	std::vector<int> idx;
	idx.reserve(1000);

	float dt_in_sec = GalaxyWnd::TimeStepSize * Helper::SEC_PER_YEAR;
	float r = 0, v = 0;
	float cr = 0.5, cg = 1, cb = 1, ca = 1;
	for (int r = 0; r < _galaxy.GetFarFieldRad(); r += 100)
	{
		idx.push_back((int)vert.size());

		if (_galaxy.HasDarkMatter())
			vert.push_back({ (float)r, Helper::VelocityWithDarkMatter((float)r) * 10.f, 0,  cr, cg, cb, ca });
		else
			vert.push_back({ (float)r, Helper::VelocityWithoutDarkMatter((float)r) * 10.f, 0,  cr, cg, cb, ca });
	}

	_vertVelocityCurve.CreateBuffer(vert, idx, GL_POINTS);
	_renderUpdateHint &= ~ruhCREATE_VELOCITY_CURVE;
}

/** \brief Update the density wave vertex buffers
*/
void GalaxyWnd::UpdateDensityWaves()
{
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
			Helper::RAD_TO_DEG * _galaxy.GetAngularOffset(r),
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

	_vertDensityWaves.CreateBuffer(vert, idx, GL_LINE_STRIP);

	//
	// Update Labels
	//

	_textGalaxyLabels.BeginUpdate();
	_textGalaxyLabels.AddText(1, GetWindowPos(0, _galaxy.GetCoreRad() + 500.f, 0), "Core");
	_textGalaxyLabels.AddText(1, GetWindowPos(0, _galaxy.GetRad() + 500 + 500.f, 0), "Disk");
	_textGalaxyLabels.AddText(1, GetWindowPos(0, _galaxy.GetFarFieldRad() + 500 + 500.f, 0), "Intergalactic medium");
	_textGalaxyLabels.EndUpdate();

	_renderUpdateHint &= ~ruhDENSITY_WAVES;
}

void GalaxyWnd::Update()
{
	if (!(_flags & (int)DisplayItem::PAUSE))
		_time += GalaxyWnd::TimeStepSize;

	if ((_renderUpdateHint & ruhDENSITY_WAVES) != 0)
		UpdateDensityWaves();

	if ((_renderUpdateHint & ruhAXIS) != 0)
		UpdateAxis();

	if ((_renderUpdateHint & ruhSTARS) != 0)
		UpdateStars();

	if ((_renderUpdateHint & ruhCREATE_VELOCITY_CURVE) != 0)
		UpdateVelocityCurve();

	_camOrient = { 0, 1, 0 };
	_camPos = { 0, 0, 5000 };
	_camLookAt = { 0, 0, 0 };
}

void GalaxyWnd::Render()
{
	static long ct = 0;
	if (!(_flags & (int)DisplayItem::PAUSE))
	{
		++ct;
	}

	AdjustCamera();

	if (_videoRecorder.IsRecording())
	{
		// Render the scene a second time into the offscreen video framebuffer.
		// Text overlays are omitted; their pixel positions are computed for the
		// window and the video is meant to show the galaxy only.
		_videoRecorder.BindFramebuffer();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double l = _fov / 2.0;
		double aspect = (double)_videoRecorder.GetWidth() / _videoRecorder.GetHeight();
		glm::mat4 matProjVideo = glm::ortho(
			-l * aspect, l * aspect,
			-l, l,
			-l, l);

		_vertStars.SetSizeFactor((float)_videoRecorder.GetHeight() / (float)_height);
		RenderScene(_matView, matProjVideo, false);
		_vertStars.SetSizeFactor(1.0f);

		_videoRecorder.CaptureFrame();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, _width, _height);
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	RenderScene(_matView, _matProjection, true);

	// Dear ImGui overlay (window pass only, never in the video framebuffer).
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();
	RenderUI();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	SDL_GL_SwapWindow(_pSdlWnd);

	// Optionally cap the render loop to the target framerate.
	if (_limitFramerate && _targetFps > 0)
	{
		const Uint32 frameMs = 1000u / (Uint32)_targetFps;
		const Uint32 elapsed = SDL_GetTicks() - _lastFrameTicks;
		if (elapsed < frameMs)
			SDL_Delay(frameMs - elapsed);
	}
	else
	{
		SDL_Delay(1);
	}
	_lastFrameTicks = SDL_GetTicks();
}

void GalaxyWnd::RenderUI()
{
	// Copyright is drawn independently of the panel so it stays visible even
	// when the control panel is hidden (F1).
	ImGui::GetForegroundDrawList()->AddText(
		ImVec2((float)_width - 155.0f, (float)_height - 22.0f),
		IM_COL32(200, 200, 200, 200), "(C) 2026 Ingo Berg");

	if (!_showUi)
		return;

	// Mirror the model into the edit cache whenever the user is not actively
	// dragging a widget. This keeps the sliders in sync with keyboard-driven
	// changes while never fighting an in-progress drag of a deferred control.
	if (!ImGui::IsAnyItemActive())
	{
		_ui.radCore   = (int)_galaxy.GetCoreRad();
		_ui.radGalaxy = (int)_galaxy.GetRad();
		_ui.exInner   = _galaxy.GetExInner();
		_ui.exOuter   = _galaxy.GetExOuter();
		_ui.angOff    = _galaxy.GetAngularOffset();
		_ui.baseTemp  = _galaxy.GetBaseTemp();
		_ui.fov       = _fov;
		_ui.numStars  = _galaxy.GetNumStars();
		_ui.numDust   = _galaxy.GetNumDust();
		_ui.numH2     = _galaxy.GetNumH2();
	}

	// Let the panel auto-size to its content so it never shows a vertical
	// scrollbar while it still fits inside the window; only cap the height at
	// the available window height (a scrollbar then appears only when needed).
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(360.0f, 0.0f),
		ImVec2(FLT_MAX, viewport->WorkSize.y - 20.0f));
	if (!ImGui::Begin("Galaxy Controls", &_showUi, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	// Fixed widget width so the auto-resized window has a deterministic size
	// and the labels to the right are never clipped.
	ImGui::PushItemWidth(150.0f);

	// Draws a collapsing header tinted with its own colour so the sections are
	// easy to tell apart at a glance.
	// Draws a collapsing header tinted with its own colour and, when the
	// section is open, tints the widgets inside it (frame backgrounds, slider
	// grabs, check marks and buttons) with the same colour so each section is
	// visually coherent. Every open beginSection() must be paired with an
	// endSection() to pop the pushed colours.
	auto scaleCol = [](ImVec4 c, float f, float a) -> ImVec4
	{
		return ImVec4(std::min(c.x * f, 1.0f), std::min(c.y * f, 1.0f), std::min(c.z * f, 1.0f), a);
	};
	auto beginSection = [&](const char* label, ImVec4 col) -> bool
	{
		ImGui::PushStyleColor(ImGuiCol_Header, col);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, scaleCol(col, 1.3f, 0.90f));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, scaleCol(col, 1.6f, 1.00f));
		bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::PopStyleColor(3);

		if (open)
		{
			ImGui::PushStyleColor(ImGuiCol_FrameBg,          scaleCol(col, 0.45f, 0.55f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,   scaleCol(col, 0.65f, 0.70f));
			ImGui::PushStyleColor(ImGuiCol_FrameBgActive,    scaleCol(col, 0.85f, 0.80f));
			ImGui::PushStyleColor(ImGuiCol_SliderGrab,       scaleCol(col, 1.7f, 1.00f));
			ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, scaleCol(col, 2.0f, 1.00f));
			ImGui::PushStyleColor(ImGuiCol_CheckMark,        scaleCol(col, 2.0f, 1.00f));
			ImGui::PushStyleColor(ImGuiCol_Button,           scaleCol(col, 0.75f, 0.85f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered,    scaleCol(col, 1.05f, 0.90f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,     scaleCol(col, 1.35f, 1.00f));
			// Scope widget IDs to the section so the same label may appear in
			// different sections (e.g. the "Stars" slider and checkbox).
			ImGui::PushID(label);
		}
		return open;
	};
	auto endSection = []() { ImGui::PopID(); ImGui::PopStyleColor(9); };

	ImGui::Text("%d FPS", GetFPS());
	ImGui::SameLine();
	ImGui::TextDisabled("(F1 toggles this panel)");

	// --- Geometry (applied live while dragging) ---------------------------
	if (beginSection("Geometry", ImVec4(0.15f, 0.30f, 0.55f, 1.0f)))
	{
		if (ImGui::SliderInt("Core radius (pc)", &_ui.radCore, 0, _ui.radGalaxy))
		{
			_galaxy.SetCoreRad((float)_ui.radCore);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
		}

		if (ImGui::SliderInt("Galaxy radius (pc)", &_ui.radGalaxy, 1000, 40000))
		{
			_galaxy.SetRad((float)_ui.radGalaxy);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
		}

		if (ImGui::SliderFloat("Excentricity inner", &_ui.exInner, 0.0f, 2.0f, "%.2f"))
		{
			_galaxy.SetExInner(_ui.exInner);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
		}

		if (ImGui::SliderFloat("Excentricity outer", &_ui.exOuter, 0.0f, 2.0f, "%.2f"))
		{
			_galaxy.SetExOuter(_ui.exOuter);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
		}

		if (ImGui::SliderFloat("Angular offset (deg/pc)", &_ui.angOff, 0.0f, 0.001f, "%.5f"))
		{
			_galaxy.SetAngularOffset(_ui.angOff);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
		}

		// Field of view is cheap (no star rebuild) -> apply immediately.
		if (ImGui::SliderFloat("Field of view (pc)", &_ui.fov, 1000.0f, 60000.0f, "%.0f", ImGuiSliderFlags_Logarithmic))
		{
			_fov = _ui.fov;
			AdjustCamera();
			SetCameraOrientation({ 0, 1, 0 });
			_renderUpdateHint |= ruhAXIS | ruhDENSITY_WAVES;
		}

		ImGui::Text("Far field radius: %d pc", (int)_galaxy.GetFarFieldRad());
		endSection();
	}

	// --- Spiral arms (cheap edits) ----------------------------------------
	if (beginSection("Spiral Arms", ImVec4(0.10f, 0.42f, 0.45f, 1.0f)))
	{
		int pertN = _galaxy.GetPertN();
		if (ImGui::SliderInt("Perturbations", &pertN, 0, 5))
		{
			_galaxy.SetPertN(pertN);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
		}

		float pertAmp = _galaxy.GetPertAmp();
		if (ImGui::SliderFloat("Perturbation damping", &pertAmp, 0.0f, 100.0f, "%.1f"))
		{
			_galaxy.SetPertAmp(pertAmp);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
		}
		endSection();
	}

	// --- Particles (counts and render sizes, applied live) -----------------
	if (beginSection("Particles", ImVec4(0.32f, 0.32f, 0.42f, 1.0f)))
	{
		if (ImGui::SliderInt("Stars", &_ui.numStars, 0, 500000))
		{
			_galaxy.SetNumStars(_ui.numStars);
			_renderUpdateHint |= ruhSTARS | ruhDUST;
		}

		if (ImGui::SliderInt("Dust clouds", &_ui.numDust, 0, 500000))
		{
			_galaxy.SetNumDust(_ui.numDust);
			_renderUpdateHint |= ruhSTARS | ruhDUST;
		}

		if (ImGui::SliderInt("H2 regions", &_ui.numH2, 0, 2000))
		{
			_galaxy.SetNumH2(_ui.numH2);
			_renderUpdateHint |= ruhSTARS | ruhDUST;
		}

		float dustSize = _galaxy.GetDustRenderSize();
		if (ImGui::SliderFloat("Dust render size (px)", &dustSize, 1.0f, 200.0f, "%.0f"))
			_galaxy.SetDustRenderSize(dustSize);   // cheap: no rebuild

		// Pure shader parameters of the H2 orbit-crowding model: no rebuild.
		ImGui::SliderFloat("H2 max size (px)", &_h2SizeMax, 10.0f, 300.0f, "%.0f");
		ImGui::SliderFloat("H2 ignition threshold", &_h2Threshold, 1.0f, 3.0f, "%.2f");
		endSection();
	}

	// --- Display features (flag toggles) -----------------------------------
	if (beginSection("Display Features", ImVec4(0.15f, 0.45f, 0.25f, 1.0f)))
	{
		// Two-column checkbox grid to save vertical space.
		int checkboxIdx = 0;
		auto flagCheckbox = [&](const char* label, DisplayItem item)
		{
			if (checkboxIdx++ % 2 == 1)
				ImGui::SameLine(180.0f);
			bool on = (_flags & (int)item) != 0;
			if (ImGui::Checkbox(label, &on))
			{
				if (on) _flags |= (int)item;
				else    _flags &= ~(int)item;
			}
		};

		flagCheckbox("Stars", DisplayItem::STARS);
		flagCheckbox("Axis", DisplayItem::AXIS);
		flagCheckbox("Dust", DisplayItem::DUST);
		flagCheckbox("H2 regions", DisplayItem::H2);
		flagCheckbox("Filaments", DisplayItem::FILAMENTS);
		flagCheckbox("Density waves", DisplayItem::DENSITY_WAVES);
		flagCheckbox("Velocity curve", DisplayItem::VELOCITY);
		endSection();
	}

	// --- Physics ----------------------------------------------------------
	if (beginSection("Physics", ImVec4(0.55f, 0.35f, 0.12f, 1.0f)))
	{
		if (ImGui::SliderFloat("Base temperature (K)", &_ui.baseTemp, 1000.0f, 10000.0f, "%.0f"))
		{
			_galaxy.SetBaseTemp(_ui.baseTemp);
			_renderUpdateHint |= ruhSTARS | ruhDUST;
		}

		bool darkMatter = _galaxy.HasDarkMatter();
		if (ImGui::Checkbox("Dark matter", &darkMatter))
		{
			_galaxy.ToggleDarkMatter();
			_renderUpdateHint |= ruhSTARS | ruhDUST | ruhCREATE_VELOCITY_CURVE;
		}
		endSection();
	}

	// --- Simulation -------------------------------------------------------
	if (beginSection("Simulation", ImVec4(0.42f, 0.20f, 0.52f, 1.0f)))
	{
		bool paused = (_flags & (int)DisplayItem::PAUSE) != 0;
		if (ImGui::Checkbox("Pause", &paused))
		{
			if (paused) _flags |= (int)DisplayItem::PAUSE;
			else        _flags &= ~(int)DisplayItem::PAUSE;
		}

		ImGui::Checkbox("Limit framerate", &_limitFramerate);
		ImGui::BeginDisabled(!_limitFramerate);
		ImGui::SliderInt("Target FPS", &_targetFps, 10, 144);
		ImGui::EndDisabled();
		endSection();
	}

	// --- Predefined galaxies ---------------------------------------------
	if (beginSection("Predefined Galaxies", ImVec4(0.55f, 0.20f, 0.22f, 1.0f)))
	{
		const char* preview = (_selectedPreset >= 0 && _selectedPreset < (int)_presets.size())
			? _presets[_selectedPreset].name.c_str()
			: "Select preset...";
		if (ImGui::BeginCombo("Preset", preview))
		{
			for (int i = 0; i < (int)_presets.size(); ++i)
			{
				const bool selected = (i == _selectedPreset);
				if (ImGui::Selectable(_presets[i].name.c_str(), selected))
					ApplyPreset(i);
				if (selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// Writes the current state to a preset file and refreshes the list,
		// keeping the saved preset selected.
		auto saveAndReload = [&](const std::string& name)
		{
			if (!SavePreset(name))
				return;
			LoadPresets();
			for (int i = 0; i < (int)_presets.size(); ++i)
				if (_presets[i].name == name)
					_selectedPreset = i;
		};

		ImGui::InputTextWithHint("##presetName", "new preset name", _presetSaveName, sizeof(_presetSaveName));
		ImGui::SameLine();
		if (ImGui::Button("Save as") && _presetSaveName[0] != '\0')
		{
			saveAndReload(_presetSaveName);
			_presetSaveName[0] = '\0';
		}

		ImGui::SameLine();
		const bool hasSelection = _selectedPreset >= 0 && _selectedPreset < (int)_presets.size();
		ImGui::BeginDisabled(!hasSelection);
		if (ImGui::Button("Save") && hasSelection)
			saveAndReload(_presets[_selectedPreset].name);
		ImGui::EndDisabled();
		if (hasSelection && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Overwrite \"%s\"", _presets[_selectedPreset].name.c_str());

		ImGui::TextDisabled("Files: presets/*.txt (numpad 1-9 applies 1st-9th)");
		endSection();
	}

	// --- Video export -----------------------------------------------------
	if (beginSection("Video Export", ImVec4(0.52f, 0.45f, 0.12f, 1.0f)))
	{
		const bool recording = _videoRecorder.IsRecording();

		ImGui::BeginDisabled(recording);

		// Resolution presets (16:9 UHD family).
		ImGui::TextUnformatted("Resolution:");
		ImGui::SameLine();
		if (ImGui::Button("2K")) { _videoWidth = 2560; _videoHeight = 1440; }
		ImGui::SameLine();
		if (ImGui::Button("4K")) { _videoWidth = 3840; _videoHeight = 2160; }
		ImGui::SameLine();
		if (ImGui::Button("8K")) { _videoWidth = 7680; _videoHeight = 4320; }

		ImGui::InputInt("Width", &_videoWidth);
		ImGui::InputInt("Height", &_videoHeight);
		ImGui::SliderInt("FPS", &_videoFps, 24, 120);
		ImGui::EndDisabled();
		if (_videoWidth < 16)  _videoWidth = 16;
		if (_videoHeight < 16) _videoHeight = 16;

		if (ImGui::Button(recording ? "Stop Recording" : "Start Recording", ImVec2(-1, 0)))
			ToggleVideoRecording();

		if (recording)
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "REC  %s  (%d frames)",
				_videoRecorder.GetFilename().c_str(), _videoRecorder.GetFrameCount());
		else
			ImGui::TextDisabled("Requires ffmpeg (libx264) in PATH");
		endSection();
	}

	ImGui::PopItemWidth();
	ImGui::End();
}

void GalaxyWnd::RenderScene(glm::mat4& matView, glm::mat4& matProjection, bool overlays)
{
	if (_flags & (int)DisplayItem::AXIS)
	{
		_vertAxis.Draw(matView, matProjection);
		CHECK_GL_ERROR

		if (overlays)
		{
			_textAxisLabel.Draw(_width, _height, matView, matProjection);
			CHECK_GL_ERROR
		}
	}

	int features = 0;
	if (_flags & (int)DisplayItem::STARS)
		features |= 1 << 0;

	if (_flags & (int)DisplayItem::DUST)
		features |= 1 << 1;

	if (_flags & (int)DisplayItem::FILAMENTS)
		features |= 1 << 2;

	if (_flags & (int)DisplayItem::H2)
		features |= 1 << 3;

	if (features != 0)
	{
		_vertStars.UpdateShaderVariables(_time, _galaxy.GetPertN(), _galaxy.GetPertAmp(), (int)_galaxy.GetDustRenderSize(), features);
		_vertStars.UpdateH2Params({
			_galaxy.GetCoreRad(),
			_galaxy.GetRad(),
			_galaxy.GetFarFieldRad(),
			_galaxy.GetExInner(),
			_galaxy.GetExOuter(),
			_galaxy.GetAngularOffset(),
			_h2SizeMax,
			_h2Threshold });
		_vertStars.Draw(matView, matProjection);
	}

	if (_flags & (int)DisplayItem::DENSITY_WAVES)
	{
		_vertDensityWaves.Draw(matView, matProjection);

		if (overlays)
			_textGalaxyLabels.Draw(_width, _height, matView, matProjection);
	}

	if (_flags & (int)DisplayItem::VELOCITY)
		_vertVelocityCurve.Draw(matView, matProjection);
}

void GalaxyWnd::ToggleVideoRecording()
{
	if (_videoRecorder.IsRecording())
	{
		_videoRecorder.Stop();
		return;
	}

	std::time_t now = std::time(nullptr);
	char timestamp[32];
	std::strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H%M%S", std::localtime(&now));

	std::string filename = std::string("galaxy-") + timestamp + ".mp4";
	_videoRecorder.Start(_videoWidth, _videoHeight, _videoFps, filename);
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
	float beta = -angle * Helper::DEG_TO_RAD;
	float sinbeta = std::sin(beta);
	float cosbeta = std::cos(beta);

	int firstPointIdx = static_cast<int>(vert.size());
	for (int i = 0; i < 360; i += 360 / steps)
	{
		float alpha = i * Helper::DEG_TO_RAD;
		float sinalpha = std::sin(alpha);
		float cosalpha = std::cos(alpha);

		GLfloat fx = (GLfloat)(x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta));
		GLfloat fy = (GLfloat)(y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta));

		if (pertNum > 0)
		{
			fx += (GLfloat)((a / pertAmp) * std::sin(alpha * 2 * pertNum));
			fy += (GLfloat)((a / pertAmp) * std::cos(alpha * 2 * pertNum));
		}

		vertIdx.push_back((int)vert.size());

		VertexColor vc = { fx, fy, 0, col.r, col.g, col.b, col.a };
		vert.push_back(vc);
	}

	// Close the loop and reset the element index array
	vertIdx.push_back(firstPointIdx);
	vertIdx.push_back(0xFFFF);
}

void GalaxyWnd::OnProcessEvents(Uint32 type)
{
	switch (type)
	{
	case SDL_MOUSEBUTTONDOWN:
		break;

	case SDL_MOUSEWHEEL:
		// Zoom the field of view; multiplicative steps match the
		// logarithmic FoV slider (wheel up = zoom in).
		if (_event.wheel.y != 0)
		{
			_fov *= std::pow(0.9f, (float)_event.wheel.y);
			_fov = std::clamp(_fov, 1000.0f, 60000.0f);
			AdjustCamera();
			SetCameraOrientation({ 0, 1, 0 });
			_renderUpdateHint |= ruhAXIS | ruhDENSITY_WAVES;
		}
		break;

	case SDL_KEYDOWN:
		switch (_event.key.keysym.sym)
		{
		case SDLK_END:
			_galaxy.SetPertN(std::max(_galaxy.GetPertN() - 1, 0));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_HOME:
			_galaxy.SetPertN(std::min(_galaxy.GetPertN() + 1, 5));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_PAGEDOWN:
			_galaxy.SetPertAmp(_galaxy.GetPertAmp() - 2);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_PAGEUP:
			_galaxy.SetPertAmp(_galaxy.GetPertAmp() + 2);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_q:
			_galaxy.SetExInner(_galaxy.GetExInner() + 0.05f);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_a:
			_galaxy.SetExInner(std::max(_galaxy.GetExInner() - 0.05f, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_w:
			_galaxy.SetExOuter(_galaxy.GetExOuter() + 0.05f);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_s:
			_galaxy.SetExOuter(std::max(_galaxy.GetExOuter() - 0.05f, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_e:
			_galaxy.SetAngularOffset(_galaxy.GetAngularOffset() + 0.00002f);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_d:
			_galaxy.SetAngularOffset(std::max(_galaxy.GetAngularOffset() - 0.00002f, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_r:
			if (_galaxy.GetRad() > _galaxy.GetCoreRad() + 500)
				_galaxy.SetCoreRad(_galaxy.GetCoreRad() + 500);
			break;

		case SDLK_m:
			_galaxy.ToggleDarkMatter();
			_renderUpdateHint |= ruhSTARS | ruhDUST | ruhCREATE_VELOCITY_CURVE;
			break;

		case SDLK_f:
			_galaxy.SetCoreRad(std::max(_galaxy.GetCoreRad() - 500, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_t:
			_galaxy.SetRad(_galaxy.GetRad() + 1000);
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_g:
			_galaxy.SetRad(std::max(_galaxy.GetRad() - 1000, 0.0f));
			_renderUpdateHint |= ruhDENSITY_WAVES | ruhSTARS | ruhDUST;
			break;

		case SDLK_z:
			_galaxy.SetBaseTemp(std::min(_galaxy.GetBaseTemp() + 100, 10000.0f));
			_renderUpdateHint |= ruhSTARS | ruhDUST;
			break;

		case SDLK_h:
			_galaxy.SetBaseTemp(std::max(_galaxy.GetBaseTemp() - 100, 1000.0f));
			_renderUpdateHint |= ruhSTARS | ruhDUST;
			break;

		case SDLK_b:
			_galaxy.SetDustRenderSize(_galaxy.GetDustRenderSize() - 5);
			break;

		case SDLK_n:
			_galaxy.SetDustRenderSize(_galaxy.GetDustRenderSize() + 5);
			break;

		case  SDLK_F1:
			_showUi = !_showUi;
			break;

		case SDLK_F2:
			_flags ^= (int)DisplayItem::AXIS;
			break;

		case  SDLK_F3:
			_flags ^= (int)DisplayItem::DUST;
			break;

		case  SDLK_F4:
			_flags ^= (int)DisplayItem::H2;
			break;

		case  SDLK_F5:
			_flags ^= (int)DisplayItem::FILAMENTS;
			break;

		case SDLK_F6:
			_flags ^= (int)DisplayItem::DENSITY_WAVES;
			break;

		case SDLK_F7:
			ToggleVideoRecording();
			break;

		case SDLK_F10:
				_bRunning = false;
			break;

		case  SDLK_v:
			_flags ^= (int)DisplayItem::VELOCITY;
			break;

		case  SDLK_PAUSE:
		case  SDLK_SPACE:
			_flags ^= (int)DisplayItem::PAUSE;
			break;

		case SDLK_KP_0:
			ApplyPreset(0);
			break;

		case SDLK_KP_1:
			ApplyPreset(1);
			break;

		case SDLK_KP_2:
			ApplyPreset(2);
			break;
		case SDLK_KP_3:
			ApplyPreset(3);
			break;

		case SDLK_KP_4:
			ApplyPreset(4);
			break;

			// Typ SBb
		case SDLK_KP_5:
			ApplyPreset(5);
			break;

		case SDLK_KP_6:
			ApplyPreset(6);
			break;


		case SDLK_KP_7:
			ApplyPreset(7);
			break;


		case SDLK_KP_8:
			ApplyPreset(8);
			break;

		case SDLK_PLUS:
		case SDLK_KP_PLUS:
			ScaleAxis(0.9f);
			SetCameraOrientation({ 0, 1, 0 });
			_renderUpdateHint |= ruhAXIS | ruhDENSITY_WAVES;  // ruhDENSITY_WAVES only for the labels!
			break;

		case SDLK_MINUS:
		case SDLK_KP_MINUS:
			ScaleAxis(1.1f);
			SetCameraOrientation({ 0, 1, 0 });
			_renderUpdateHint |= ruhAXIS | ruhDENSITY_WAVES;  // ruhDENSITY_WAVES only for the labels!
			break;

		case SDLK_ESCAPE:
			// This is a silly mechanism to disable sdl event polling and work around an issue where the polling stops briefly evers 3 seconds.
			// the only purpose is to be able to screengrab proper animations.
			static int ct = 0;
			ct++;
			if (ct <= 5)
			{
				std::cout << "Stop pressing [ESC] or i will stop event polling!" << std::endl;
			}
			else
			{
				std::cout << "Ok, you wanted this. Enjoy you frozen window..." << std::endl;
				_stopEventPolling = true;
			}
			break;
		} // switch (m_event.key.keysym.sym)
		break;
	} // switch (type) -> Mouse or Keys
}
