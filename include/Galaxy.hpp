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

	void SetPertN(int n);
	void SetPertAmp(float amp);
	void SetAngularOffset(float offset);
	void SetCoreRad(float rad);
	void SetRad(float rad);
	void SetExInner(float ex);
	void SetExOuter(float ex);
	void SetDustRenderSize(float sz);
	void SetBaseTemp(float temp);

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
	int _numH2;             ///< Number of H2 Regions

	int _pertN;
	float _pertAmp;

	bool _hasDarkMatter;
	float _baseTemp;

private:
	std::vector<Star> _stars;  ///< Pointer to an array of star and dust data
};
