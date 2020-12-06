#pragma once

#include <memory>
#include "Star.hpp"


/** \brief A class to encapsulate the geometric details of a spiral galaxy. */
class Galaxy final
{
public:

	Galaxy(
		double rad = 15000,
		double radCore = 6000,
		double deltaAng = 0.019,
		double ex1 = 0.8,
		double ex2 = 1,
		int numStars = 20000);
	~Galaxy();

	void Reset(
		double rad,
		double radCore,
		double deltaAng,
		double ex1,
		double ex2,
		int numStars,
		bool hasDarkMatter,
		int pertN,
		double pertAmp,
		double dustRenderSize,
		double (*ColorFun)(double) = nullptr);

	void Reset();

	Star* GetStars() const;
	Star* GetDust() const;
	Star* GetH2() const;

	double GetRad() const;
	double GetCoreRad() const;
	double GetFarFieldRad() const;
	double GetSigma() const;

	// Properties depending on the orbital radius

	double GetExcentricity(double rad) const;
	double GetOrbitalVelocity(double rad) const;

	double GetAngularOffset(double rad) const;
	double GetAngularOffset() const;

	double GetExInner() const;
	double GetExOuter() const;
	double GetTimeStep() const;
	double GetTime() const;
	double GetDustRenderSize() const;
	int GetNumStars() const;
	int GetNumDust() const;
	int GetNumH2() const;
	int GetPertN() const;
	double GetPertAmp() const;

	void ToggleDarkMatter();

	void SingleTimeStep(double time);

	const Vec2D& GetStarPos(int idx);

	void SetPertN(int n);
	void SetPertAmp(double amp);
	void SetSigma(double sigma);
	void SetAngularOffset(double offset);
	void SetCoreRad(double rad);
	void SetRad(double rad);
	void SetExInner(double ex);
	void SetExOuter(double ex);
	void SetDustRenderSize(double sz);
	void SetColorFunction(double (*cf)(double)) noexcept(false);

private:

	Galaxy(const Galaxy& obj);
	Galaxy& operator=(const Galaxy& obj);

	void InitStars();

	// Parameters needed for defining the general structure of the galaxy

	double _elEx1;          ///< Excentricity of the innermost ellipse
	double _elEx2;          ///< Excentricity of the outermost ellipse

	double _velOrigin;      ///< Velovity at the innermost core in km/s

	double _angleOffset;    ///< Angular offset per parsec

	double _radCore;        ///< Radius of the inner core
	double _radGalaxy;      ///< Radius of the galaxy
	double _radFarField;    ///< The radius after which all density waves must have circular shape
	double _velAngle;       ///< Angular velocity of the density waves

	double _dustRenderSize;

	int _numStars;          ///< Total number of stars
	int _numDust;           ///< Number of Dust Particles
	int _numH2;             ///< Number of H2 Regions

	int _pertN;
	double _pertAmp;

	double _time;
	double _timeStep;

	bool _hasDarkMatter;

	double (*_colorFun)(double);

public:

	int _numberByRad[100];  ///< Historgramm showing distribution of stars

private:

	Vec2D _pos;             ///< Center of the galaxy
	Star *_pStars;          ///< Pointer to an array of star data
	Star* _pDust;           ///< Pointer to an array of dusty areas
	Star* _pH2;
};
