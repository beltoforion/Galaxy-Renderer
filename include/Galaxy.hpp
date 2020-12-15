#pragma once

#include <memory>
#include "Star.hpp"


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
	};

	Galaxy(
		float rad = 15000,
		float radCore = 6000,
		float deltaAng = 0.019,
		float ex1 = 0.8,
		float ex2 = 1,
		int numStars = 40000);
	~Galaxy();

	void Reset(GalaxyParam param);
	void Reset();

	Star* GetStars() const;
	Star* GetDust() const;
	Star* GetH2() const;

	float GetRad() const;
	float GetCoreRad() const;
	float GetFarFieldRad() const;

	static void CalcXY(Star &pStar, int pertN, float pertAmp);

	// Properties depending on the orbital radius

	float GetExcentricity(float rad) const;
	float GetOrbitalVelocity(float rad) const;

	float GetAngularOffset(float rad) const;
	float GetAngularOffset() const;

	float GetExInner() const;
	float GetExOuter() const;
	float GetTimeStep() const;
	float GetTime() const;
	float GetDustRenderSize() const;
	int GetNumStars() const;
	int GetNumDust() const;
	int GetNumH2() const;
	int GetPertN() const;
	float GetPertAmp() const;
	float GetBaseTemp() const noexcept;

	void ToggleDarkMatter();

	void SingleTimeStep(float time);

	const Vec2D& GetStarPos(int idx);

	void SetPertN(int n);
	void SetPertAmp(float amp);
	void SetAngularOffset(float offset);
	void SetCoreRad(float rad);
	void SetRad(float rad);
	void SetExInner(float ex);
	void SetExOuter(float ex);
	void SetDustRenderSize(float sz);
	void SetBaseTemp(float temp);

	bool HasDarkMatter() const noexcept;

private:

	Galaxy(const Galaxy& obj);
	Galaxy& operator=(const Galaxy& obj);

	void InitStars();

	// Parameters needed for defining the general structure of the galaxy

	float _elEx1;          ///< Excentricity of the innermost ellipse
	float _elEx2;          ///< Excentricity of the outermost ellipse

	float _velOrigin;      ///< Velovity at the innermost core in km/s

	float _angleOffset;    ///< Angular offset per parsec

	float _radCore;        ///< Radius of the inner core
	float _radGalaxy;      ///< Radius of the galaxy
	float _radFarField;    ///< The radius after which all density waves must have circular shape
	float _velAngle;       ///< Angular velocity of the density waves

	float _dustRenderSize;

	int _numStars;          ///< Total number of stars
	int _numDust;           ///< Number of Dust Particles
	int _numH2;             ///< Number of H2 Regions

	int _pertN;
	float _pertAmp;

	float _time;
	float _timeStep;

	bool _hasDarkMatter;

	float _baseTemp;

public:
	int _numberByRad[100];  ///< Historgramm showing distribution of stars

private:
	Vec2D _pos;             ///< Center of the galaxy
	Star *_pStars;          ///< Pointer to an array of star data
	Star *_pDust;           ///< Pointer to an array of dusty areas
	Star *_pH2;
};
