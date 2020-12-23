#include "Galaxy.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <execution>
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
	, _velOrigin(30)
	, _angleOffset(deltaAng)
	, _radCore(radCore)
	, _radGalaxy(rad)
	, _radFarField(_radGalaxy * 2)
	, _velAngle(0.000001f)
	, _numStars(numStars)
	, _numDust(numStars)
	, _numH2(500)
	, _pertN(0)
	, _pertAmp(0)
	, _time(0)
	, _hasDarkMatter(true)
	, _baseTemp(4000)
	, _stars()
	, _dust()
	, _h2()
	, _dustRenderSize(70)
{}

Galaxy::~Galaxy()
{}

void Galaxy::Reset()
{
	Reset({
		_radGalaxy,
		_radCore,
		_angleOffset,
		_elEx1,
		_elEx2,
		_numStars,
		_hasDarkMatter,
		_pertN,
		_pertAmp,
		_dustRenderSize,
		_baseTemp });
}

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
	_numDust = param.numStars;
	_time = 0;
	_dustRenderSize = param.dustRenderSize;
	_hasDarkMatter = param.hasDarkMatter;
	_pertN = param.pertN;
	_pertAmp = param.pertAmp;

	InitStarsAndDust();
	InitH2AndFilaments();
}

bool Galaxy::HasDarkMatter() const noexcept
{
	return _hasDarkMatter;
}

void Galaxy::ToggleDarkMatter()
{
	_hasDarkMatter ^= true;
	Reset();
}

void Galaxy::InitH2AndFilaments()
{
	_dust = std::vector<Star>();
	_dust.reserve(_numStars);

	_h2 = std::vector((size_t)(2 * _numH2), Star());

	// Initialize the stars
	CumulativeDistributionFunction cdf;
	cdf.SetupRealistic(
		1.0,				// maximum intensity
		0.02,				// k (bulge)
		_radGalaxy / 3.0f,	// disc scale length
		_radCore,			// bulge radius
		0,					// start  of the intnesity curve
		_radFarField,		// end of the intensity curve
		1000);				// number of supporting points

	// Initialize Filaments
	float x, y, rad;
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
			dustParticle.center = { 0, 0 };

			// I want the outer parts to appear blue, the inner parts yellow. I'm imposing
			// the following temperature distribution (no science here it just looks right)
			dustParticle.temp = _baseTemp + rad / 4.5f - 1000;;
			dustParticle.mag = mag + 0.025f * Helper::rnum();
			dustParticle.type = 2;
			_dust.push_back(dustParticle);
		}
	}

	// Initialise H2 regions
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
		particleH2.center = { 0, 0 };
		particleH2.temp = 6000 + (6000 * Helper::rnum()) - 3000;
		particleH2.mag = 0.1f + 0.05f * Helper::rnum();
		particleH2.type = 3;

		_dust.push_back(particleH2);
		
		// Push particle again with type 4 (bright red core of an h2 region)
		particleH2.type = 4;
		_dust.push_back(particleH2);
	}


	// Initialise H2 regions
	for (int i = 0; i < _numH2; ++i)
	{
		x = 2 * _radGalaxy * Helper::rnum() - _radGalaxy;
		y = 2 * _radGalaxy * Helper::rnum() - _radGalaxy;
		rad = sqrt(x * x + y * y);

		int k1 = 2 * i;
		_h2[k1].a = rad;
		_h2[k1].b = rad * GetExcentricity(rad);
		_h2[k1].tiltAngle = GetAngularOffset(rad);
		_h2[k1].theta0 = 360.0f * Helper::rnum();
		_h2[k1].velTheta = GetOrbitalVelocity((_h2[k1].a + _h2[k1].b) / 2.0f);
		_h2[k1].center = { 0, 0 };
		_h2[k1].temp = 6000 + (6000 * Helper::rnum()) - 3000;
		_h2[k1].mag = 0.1f + 0.05f * Helper::rnum();

		// Create second point 100 pc away from the first one
		int dist = 1000;
		int k2 = 2 * i + 1;
		_h2[k2].a = (rad + dist);
		_h2[k2].b = (rad)*GetExcentricity(rad);
		_h2[k2].tiltAngle = GetAngularOffset(rad);
		_h2[k2].theta0 = _h2[k1].theta0;
		_h2[k2].velTheta = _h2[k1].velTheta;
		_h2[k2].center = _h2[k1].center;
		_h2[k2].temp = _h2[k1].temp;
		_h2[k2].mag = _h2[k1].mag;
	}
}

void Galaxy::InitStarsAndDust()
{
	_stars = std::vector<Star>();
	_stars.reserve(_numStars);

	// First star ist the black hole at the centre
	auto star = Star();
	star.a = 0;
	star.b = 0;
	star.tiltAngle = 0;
	star.theta0 = 0;
	star.velTheta = 0;
	star.center = { 0, 0 };
	star.velTheta = GetOrbitalVelocity((star.a + star.b) / 2.0f);
	star.type = 0;
	star.temp = 6000;
	_stars.push_back(star);

	// Initialize the stars
	CumulativeDistributionFunction cdf;
	cdf.SetupRealistic(
		1.0,				// maximum intensity
		0.02,				// k (bulge)
		_radGalaxy / 3.0f,	// disc scale length
		_radCore,			// bulge radius
		0,					// start  of the intnesity curve
		_radFarField,		// end of the intensity curve
		1000);				// number of supporting points

	for (int i = 1; i < _numStars; ++i)
	{
		float rad = (float)cdf.ValFromProb(Helper::rnum());
		auto star = Star();
		star.a = rad;
		star.b = rad * GetExcentricity(rad);
		star.tiltAngle = GetAngularOffset(rad);
		star.theta0 = 360.0f * Helper::rnum();
		star.velTheta = GetOrbitalVelocity(rad);
		star.center = { 0, 0 };
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

	// Initialise Dust:
	// The galaxy gets as many dust clouds as stars
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
		dustParticle.center = { 0, 0 };
		dustParticle.type = 1;

		// I want the outer parts to appear blue, the inner parts yellow. I'm imposing
		// the following temperature distribution (no science here it just looks right)
		dustParticle.temp = _baseTemp + rad / 4.5f;
		dustParticle.mag = 0.02f + 0.15f * Helper::rnum();
		_stars.push_back(dustParticle);
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
	InitH2AndFilaments();
}

void Galaxy::SetDustRenderSize(float sz)
{
	_dustRenderSize = std::min(200.0f, std::max(sz, 1.0f));
}

const std::vector<Star>& Galaxy::GetStars() const
{
	return _stars;
}

const std::vector<Star>& Galaxy::GetDust() const
{
	return _dust;
}

const std::vector<Star>& Galaxy::GetH2() const
{
	return _h2;
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
	Reset();
}

/** \brief Returns the orbital velocity in degrees per year.
	\param rad Radius in parsec
*/
float Galaxy::GetOrbitalVelocity(float rad) const
{
	float vel_kms(0);  // velovity in kilometer per seconds

	// Realistically looking velocity curves for the Wikipedia models.
	struct VelocityCurve
	{
		static double MS(double r)
		{
			float d = 2000;  // Dicke der Scheibe
			float rho_so = 1;  // Dichte im Mittelpunkt
			float rH = 2000; // Radius auf dem die Dichte um die Hälfte gefallen ist
			return (float)rho_so * (float)std::exp(-r / rH) * (r * r) * Helper::PI * d;
		}

		static float MH(float r)
		{
			float rho_h0 = 0.15f; // Dichte des Halos im Zentrum
			float rC = 2500;     // typische skalenlänge im Halo
			return (float)rho_h0 * 1 / (float)(1 + std::pow(r / rC, 2)) * (float)(4 * Helper::PI * std::pow(r, 3) / 3);
		}

		// Velocity curve with dark matter
		static float v(float r)
		{
			float MZ = 100;
			return 20000.0f * (float)std::sqrt(Helper::CONTANT_OF_GRAVITY * (MH(r) + MS(r) + MZ) / r);
		}

		// velocity curve without dark matter
		static float vd(float r)
		{
			float MZ = 100;
			return 20000.0f * (float)std::sqrt(Helper::CONTANT_OF_GRAVITY * (MS(r) + MZ) / r);
		}
	};

	if (_hasDarkMatter)
	{
		//  with dark matter
		vel_kms = VelocityCurve::v(rad);
	}
	else
	{
		// without dark matter:
		vel_kms = VelocityCurve::vd(rad);
	}

	// Calculate velocity in degree per year
	float u = 2.0f * Helper::PI * rad * Helper::PC_TO_KM;        // Umfang in km
	float time = u / (vel_kms * Helper::SEC_PER_YEAR);  // Umlaufzeit in Jahren

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
	Reset();
}

void Galaxy::SetCoreRad(float rad)
{
	_radCore = rad;
	Reset();
}

void Galaxy::SetExInner(float ex)
{
	_elEx1 = ex;
	Reset();
}

void Galaxy::SetExOuter(float ex)
{
	_elEx2 = ex;
	Reset();
}

float Galaxy::GetTime() const
{
	return _time;
}

void Galaxy::CalcXy(Star& p, float time, int pertN, float pertAmp)
{
	auto thetaActual = p.theta0 + p.velTheta * time;

	float beta = -p.tiltAngle;
	float alpha = thetaActual * Helper::DEG_TO_RAD;

	// temporaries to save cpu time
	float cosalpha = std::cos(alpha);
	float sinalpha = std::sin(alpha);
	float cosbeta = std::cos(beta);
	float sinbeta = std::sin(beta);

	Vec2 ps =
	{
		p.center.x + (p.a * cosalpha * cosbeta - p.b * sinalpha * sinbeta),
		p.center.y + (p.a * cosalpha * sinbeta + p.b * sinalpha * cosbeta)
	};

	// Add small perturbations to create more spiral arms
	if (pertAmp > 0 && pertN > 0)
	{
		ps.x += (p.a / pertAmp) * sin(alpha * 2 * pertN);
		ps.y += (p.a / pertAmp) * cos(alpha * 2 * pertN);
	}

	p.pos = ps;
}

void Galaxy::SingleTimeStep(float timeStepSize)
{
	_time += timeStepSize;

	auto pertN = _pertN;
	auto pertAmp = _pertAmp;
	auto time = _time;

	std::for_each(
		std::execution::par_unseq,
		_stars.begin(),
		_stars.end(),
		[pertN, pertAmp, time](auto&& pt)
		{
			Vec2 posOld = pt.pos;
			Galaxy::CalcXy(pt, time, pertN, pertAmp);

			Vec2 b = {
				pt.pos.x - posOld.x,
				pt.pos.y - posOld.y
			};

			pt.vel = b;
		});

	std::for_each(
		std::execution::par_unseq,
		_dust.begin(),
		_dust.end(),
		[pertN, pertAmp, time](auto&& pt)
		{
			Galaxy::CalcXy(pt, time, pertN, pertAmp);
		});

	std::for_each(
		std::execution::par_unseq,
		_h2.begin(),
		_h2.end(),
		[pertN, pertAmp, time](auto&& pt)
		{
			Galaxy::CalcXy(pt, time, pertN, pertAmp);
		});
}