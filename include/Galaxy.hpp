#pragma once

#include <vector>
#include "Types.hpp"


/** \brief A class to encapsulate the geometric details of a spiral galaxy. */
class Galaxy final
{
public:

	struct GalaxyParam {
		float rad;
		float radCore;
		float deltaAng;
		float ex1;
		float ex2;
		int numStars;
		bool hasDarkMatter;
		int pertN;
		float pertAmp;
		float dustRenderSize;
		float baseTemp;
		int numDust = -1;   ///< number of dust clouds; -1 = same as numStars
		int numH2 = 400;    ///< number of H2 regions

		// Central bar (off by default: presets from older versions keep
		// their look)
		bool hasBar = false;
		float barRadius = 3000.0f;    ///< length of the bar semi-major axis (pc)
		float barEx = 0.55f;          ///< axis ratio b/a of the bar orbits
	};

	Galaxy(
		float rad = 15000,
		float radCore = 6000,
		float deltaAng = 0.019,
		float ex1 = 0.8,
		float ex2 = 1,
		int numStars = 60000);
	~Galaxy();

	void Reset(GalaxyParam param);

	const std::vector<Star>& GetStars() const;

	float GetRad() const;
	float GetCoreRad() const;
	float GetFarFieldRad() const;
	float GetExcentricity(float rad) const;
	float GetOrbitalVelocity(float rad) const;
	float GetAngularOffset(float rad) const;
	float GetAngularOffset() const;
	float GetExInner() const;
	float GetExOuter() const;
	float GetDustRenderSize() const;
	int GetPertN() const;
	float GetPertAmp() const;
	float GetBaseTemp() const noexcept;
	int GetNumStars() const;
	int GetNumDust() const;
	int GetNumH2() const;
	bool HasBar() const noexcept;
	float GetBarRadius() const noexcept;
	float GetBarEx() const noexcept;

	void SetPertN(int n);
	void SetPertAmp(float amp);
	void SetAngularOffset(float offset);
	void SetCoreRad(float rad);
	void SetRad(float rad);
	void SetExInner(float ex);
	void SetExOuter(float ex);
	void SetDustRenderSize(float sz);
	void SetBaseTemp(float temp);
	void SetNumStars(int n);
	void SetNumDust(int n);
	void SetNumH2(int n);
	void SetBarEnabled(bool on);
	void SetBarRadius(float rad);
	void SetBarEx(float ex);

	void ToggleDarkMatter();
	bool HasDarkMatter() const noexcept;

private:

	Galaxy(const Galaxy& obj);
	Galaxy& operator=(const Galaxy& obj);

	void InitStarsAndDust();

	float _elEx1;          ///< Excentricity of the innermost ellipse
	float _elEx2;          ///< Excentricity of the outermost ellipse

	float _angleOffset;    ///< Angular offset per parsec

	float _radCore;        ///< Radius of the inner core
	float _radGalaxy;      ///< Radius of the galaxy
	float _radFarField;    ///< The radius after which all density waves must have circular shape
	float _dustRenderSize;

	int _numStars;          ///< Total number of stars
	int _numDust;           ///< Number of dust clouds
	int _numH2;             ///< Number of H2 Regions

	unsigned int _seed;     ///< RNG seed so parameter tweaks rebuild the same star population

	int _pertN;
	float _pertAmp;

	bool _hasDarkMatter;
	float _baseTemp;

	bool _hasBar;
	float _barRadius;
	float _barEx;

private:
	std::vector<Star> _stars;  ///< Pointer to an array of star and dust data
};
