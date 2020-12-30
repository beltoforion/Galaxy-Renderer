#include "Galaxy.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <algorithm>

#include "Helper.hpp"
#include "Types.hpp"
#include "CumulativeDistributionFunction.hpp"


Galaxy::Galaxy(
	float rad,
	float radCore,
	float deltaAng,
	float ex1,
	float ex2,
	int numStars)
	: _elEx1(ex1)
	, _elEx2(ex2)
	, _angleOffset(deltaAng)
	, _radCore(radCore)
	, _radGalaxy(rad)
	, _radFarField(_radGalaxy * 2)
	, _numStars(numStars)
	, _numH2(400)
	, _pertN(0)
	, _pertAmp(0)
	, _hasDarkMatter(true)
	, _baseTemp(4000)
	, _stars()
	, _dustRenderSize(70)
{}

Galaxy::~Galaxy()
{}

void Galaxy::Reset(GalaxyParam param)
{
	_baseTemp = param.baseTemp;
	_elEx1 = param.ex1;
	_elEx2 = param.ex2;
	_elEx2 = param.ex2;
	_angleOffset = param.deltaAng;
	_radCore = param.radCore;
	_radGalaxy = param.rad;
	_radFarField = _radGalaxy * 2;  // there is no science behind this threshold it just looks nice
	_numStars = param.numStars;
	_dustRenderSize = param.dustRenderSize;
	_hasDarkMatter = param.hasDarkMatter;
	_pertN = param.pertN;
	_pertAmp = param.pertAmp;

	InitStarsAndDust();
}

bool Galaxy::HasDarkMatter() const noexcept
{
	return _hasDarkMatter;
}

void Galaxy::ToggleDarkMatter()
{
	_hasDarkMatter ^= true;
	InitStarsAndDust();
}

void Galaxy::InitStarsAndDust()
{
	_stars = std::vector<Star>();
	_stars.reserve(_numStars);

	//
	// 1.) Initialize the stars
	//

	CumulativeDistributionFunction cdf;
	cdf.SetupRealistic(
		1.0,				// maximum intensity
		0.02,				// k (bulge)
		_radGalaxy / 3.0f,	// disc scale length
		_radCore,			// bulge radius
		0,					// start  of the intnesity curve
		_radFarField,		// end of the intensity curve
		1000);				// number of supporting points

	for (int i = 0; i < _numStars; ++i)
	{
		float rad = (float)cdf.ValFromProb(Helper::rnum());
		auto star = Star();
		star.a = rad;
		star.b = rad * GetExcentricity(rad);
		star.tiltAngle = GetAngularOffset(rad);
		star.theta0 = 360.0f * Helper::rnum();
		star.velTheta = GetOrbitalVelocity(rad);
		star.temp = 6000 + (4000 * Helper::rnum() - 2000);
		star.mag = 0.1f + 0.4f * Helper::rnum();
		star.type = 0;

		// Make a small portion of the stars brighter
		if (i < _numStars / 60)
		{
			star.mag = std::min(star.mag + 0.1f + Helper::rnum() * 0.4f, 1.0f);
		}

		_stars.push_back(star);
	}

	//
	// 2.) Initialise Dust:
	//
	//	The galaxy gets as many dust clouds as stars

	float x, y, rad;
	for (int i = 0; i < _numStars; ++i)
	{
		if (i % 2 == 0)
		{
			rad = (float)cdf.ValFromProb(Helper::rnum());
		}
		else
		{
			x = 2 * _radGalaxy * Helper::rnum() - _radGalaxy;
			y = 2 * _radGalaxy * Helper::rnum() - _radGalaxy;
			rad = sqrt(x * x + y * y);
		}

		auto dustParticle = Star();
		dustParticle.a = rad;
		dustParticle.b = rad * GetExcentricity(rad);
		dustParticle.tiltAngle = GetAngularOffset(rad);
		dustParticle.theta0 = 360.0f * Helper::rnum();
		dustParticle.velTheta = GetOrbitalVelocity((dustParticle.a + dustParticle.b) / 2.0f);
		dustParticle.type = 1;

		// I want the outer parts to appear blue, the inner parts yellow. I'm imposing
		// the following temperature distribution (no science here it just looks right)
		dustParticle.temp = _baseTemp + rad / 4.5f;
		dustParticle.mag = 0.02f + 0.15f * Helper::rnum();
		_stars.push_back(dustParticle);
	}

	//
	// 3.) Initialize additional dust filaments
	//

	for (int i = 0; i < _numStars / 100; ++i)
	{
		rad = (float)cdf.ValFromProb(Helper::rnum());

		x = 2 * _radGalaxy * Helper::rnum() - _radGalaxy;
		y = 2 * _radGalaxy * Helper::rnum() - _radGalaxy;
		rad = sqrt(x * x + y * y);

		auto theta = 360.0f * Helper::rnum();
		auto mag = 0.1f + 0.05f * Helper::rnum();
		auto a = rad;
		auto b = rad * GetExcentricity(rad);
		auto num = (int)(100 * Helper::rnum());
		auto temp = _baseTemp + rad / 4.5f - 2000;
		for (int i = 0; i < num; ++i)
		{
			rad = rad + 200 - 400 * Helper::rnum();
			auto dustParticle = Star();
			dustParticle.a = rad;
			dustParticle.b = rad * GetExcentricity(rad);
			dustParticle.tiltAngle = GetAngularOffset(rad);
			dustParticle.theta0 = theta + 10 - 20 * Helper::rnum();
			dustParticle.velTheta = GetOrbitalVelocity((dustParticle.a + dustParticle.b) / 2.0f);

			// I want the outer parts to appear blue, the inner parts yellow. I'm imposing
			// the following temperature distribution (no science here it just looks right)
			dustParticle.temp = _baseTemp + rad / 4.5f - 1000;;
			dustParticle.mag = mag + 0.025f * Helper::rnum();
			dustParticle.type = 2;
			_stars.push_back(dustParticle);
		}
	}
	
	//
	// 4.) Initialise H2 regions
	// 

	for (int i = 0; i < _numH2; ++i)
	{
		x = 2 * _radGalaxy * Helper::rnum() - _radGalaxy;
		y = 2 * _radGalaxy * Helper::rnum() - _radGalaxy;
		rad = sqrt(x * x + y * y);

		auto particleH2 = Star();
		particleH2.a = rad;
		particleH2.b = rad * GetExcentricity(rad);
		particleH2.tiltAngle = GetAngularOffset(rad);
		particleH2.theta0 = 360.0f * Helper::rnum();
		particleH2.velTheta = GetOrbitalVelocity((particleH2.a + particleH2.b) / 2.0f);
		particleH2.temp = 6000 + (6000 * Helper::rnum()) - 3000;
		particleH2.mag = 0.1f + 0.05f * Helper::rnum();
		particleH2.type = 3;

		_stars.push_back(particleH2);

		// Push particle again with type 4 (bright red core of an h2 region)
		particleH2.type = 4;
		_stars.push_back(particleH2);
	}
}

float Galaxy::GetBaseTemp() const noexcept
{
	return _baseTemp;
}

void Galaxy::SetBaseTemp(float baseTemp)
{
	_baseTemp = baseTemp;
	InitStarsAndDust();
}

void Galaxy::SetDustRenderSize(float sz)
{
	_dustRenderSize = std::min(200.0f, std::max(sz, 1.0f));
}

const std::vector<Star>& Galaxy::GetStars() const
{
	return _stars;
}

float Galaxy::GetDustRenderSize() const
{
	return _dustRenderSize;
}

float Galaxy::GetRad() const
{
	return _radGalaxy;
}

float Galaxy::GetCoreRad() const
{
	return _radCore;
}

float Galaxy::GetFarFieldRad() const
{
	return _radFarField;
}

void Galaxy::SetAngularOffset(float offset)
{
	_angleOffset = offset;
	InitStarsAndDust();
}

/** \brief Returns the orbital velocity in degrees per year.
	\param rad Radius in parsec
*/
float Galaxy::GetOrbitalVelocity(float rad) const
{
	float vel_kms = 0;  // velovity in kilometer per seconds
	if (_hasDarkMatter)
	{
		vel_kms = Helper::VelocityWithDarkMatter(rad);
	}
	else
	{
		vel_kms = Helper::VelocityWithoutDarkMatter(rad);
	}

	// Calculate velocity in degree per year
	float u = 2.0f * Helper::PI * rad * Helper::PC_TO_KM;   // Umfang in km
	float time = u / (vel_kms * Helper::SEC_PER_YEAR);		// Umlaufzeit in Jahren

	return 360.0f / time;                                   // Grad pro Jahr
}

float Galaxy::GetExcentricity(float r) const
{
	if (r < _radCore)
	{
		// Core region of the galaxy. Innermost part is round
		// excentricity increasing linear to the border of the core.
		return 1 + (r / _radCore) * (_elEx1 - 1);
	}
	else if (r > _radCore && r <= _radGalaxy)
	{
		return _elEx1 + (r - _radCore) / (_radGalaxy - _radCore) * (_elEx2 - _elEx1);
	}
	else if (r > _radGalaxy && r < _radFarField)
	{
		// excentricity is slowly reduced to 1.
		return _elEx2 + (r - _radGalaxy) / (_radFarField - _radGalaxy) * (1 - _elEx2);
	}
	else
		return 1;
}

float Galaxy::GetAngularOffset(float rad) const
{
	return rad * _angleOffset;
}

float Galaxy::GetAngularOffset() const
{
	return _angleOffset;
}

float Galaxy::GetExInner() const
{
	return _elEx1;
}

float Galaxy::GetExOuter() const
{
	return _elEx2;
}

int Galaxy::GetPertN() const
{
	return _pertN;
}

float Galaxy::GetPertAmp() const
{
	return _pertAmp;
}

void Galaxy::SetPertN(int n)
{
	if (n < 0 || n>5)
		throw std::runtime_error("pertN must be greater than 0 and less than 6!");

	_pertN = n;
}

void Galaxy::SetPertAmp(float amp)
{
	_pertAmp = std::max(0.0f, amp);
}

void Galaxy::SetRad(float rad)
{
	_radGalaxy = rad;
	InitStarsAndDust();
}

void Galaxy::SetCoreRad(float rad)
{
	_radCore = rad;
	InitStarsAndDust();
}

void Galaxy::SetExInner(float ex)
{
	_elEx1 = ex;
	InitStarsAndDust();
}

void Galaxy::SetExOuter(float ex)
{
	_elEx2 = ex;
	InitStarsAndDust();
}
