#include "Galaxy.hpp"
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <cmath>
#include <iostream>

#include "MathHelper.hpp"
#include "Star.hpp"
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
	, _numH2(300)
	, _pertN(0)
	, _pertAmp(0)
	, _time(0)
	, _timeStep(0)
	, _hasDarkMatter(true)
	, _baseTemp(4000)
	, _numberByRad()
	, _pos({ 0, 0 })
	, _pStars(nullptr)
	, _pDust(nullptr)
	, _pH2(nullptr)
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
	_numDust = param.numStars / 2;
	_time = 0;
	_dustRenderSize = param.dustRenderSize;
	_hasDarkMatter = param.hasDarkMatter;
	_pertN = param.pertN;
	_pertAmp = param.pertAmp;

	for (int i = 0; i < 100; ++i)
		_numberByRad[i] = 0;

	InitStars();
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

void Galaxy::InitStars()
{
	delete[] _pDust;
	_pDust = new Star[_numDust];

	delete[] _pStars;
	_pStars = new Star[_numStars];

	delete[] _pH2;
	_pH2 = new Star[_numH2 * 2];

	// The first three stars can be used for aligning the
	// camera with the galaxy rotation.

	// First star ist the black hole at the centre
	_pStars[0].a = 0;
	_pStars[0].b = 0;
	_pStars[0].angle = 0;
	_pStars[0].theta = 0;
	_pStars[0].velTheta = 0;
	_pStars[0].center = { 0, 0 };
	_pStars[0].velTheta = GetOrbitalVelocity((_pStars[0].a + _pStars[0].b) / 2.0f);
	_pStars[0].temp = 6000;

	// second star is at the edge of the core area
	_pStars[1].a = _radCore;
	_pStars[1].b = _radCore * GetExcentricity(_radCore);
	_pStars[1].angle = GetAngularOffset(_radCore);
	_pStars[1].theta = 0;
	_pStars[1].center = { 0, 0 };
	_pStars[1].velTheta = GetOrbitalVelocity((_pStars[1].a + _pStars[1].b) / 2.0f);
	_pStars[1].temp = 6000;

	// third star is at the edge of the disk
	_pStars[2].a = _radGalaxy;
	_pStars[2].b = _radGalaxy * GetExcentricity(_radGalaxy);
	_pStars[2].angle = GetAngularOffset(_radGalaxy);
	_pStars[2].theta = 0;
	_pStars[2].center = { 0, 0 };
	_pStars[2].velTheta = GetOrbitalVelocity((_pStars[2].a + _pStars[2].b) / 2.0f);
	_pStars[2].temp = 6000;

	// cell width of the histogramm
	float dh = _radFarField / 100.0f;

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

	for (int i = 3; i < _numStars; ++i)
	{
		float rad = (float)cdf.ValFromProb((float)rand() / (float)RAND_MAX);

		_pStars[i].a = rad;
		_pStars[i].b = rad * GetExcentricity(rad);
		_pStars[i].angle = GetAngularOffset(rad);
		_pStars[i].theta = 360.0f * ((float)rand() / RAND_MAX);
		_pStars[i].velTheta = GetOrbitalVelocity(rad);
		_pStars[i].center = { 0, 0 };
		_pStars[i].temp = 6000 + (4000 * ((float)rand() / RAND_MAX)) - 2000;
		_pStars[i].mag = 0.3f + 0.2f * (float)rand() / (float)RAND_MAX;

		int idx = (int)std::min(1.0f / dh * (_pStars[i].a + _pStars[i].b) / 2.0f, 99.0f);
		_numberByRad[idx]++;
	}

	// Initialise Dust
	float x, y, rad;
	for (int i = 0; i < _numDust; ++i)
	{
		if (i % 4 == 0)
		{
			rad = (float)cdf.ValFromProb((float)rand() / (float)RAND_MAX);
		}
		else
		{
			x = 2 * _radGalaxy * ((float)rand() / (float)RAND_MAX) - _radGalaxy;
			y = 2 * _radGalaxy * ((float)rand() / (float)RAND_MAX) - _radGalaxy;
			rad = sqrt(x * x + y * y);
		}

		_pDust[i].a = rad;
		_pDust[i].b = rad * GetExcentricity(rad);
		_pDust[i].angle = GetAngularOffset(rad);
		_pDust[i].theta = 360.0f * ((float)rand() / (float)RAND_MAX);
		_pDust[i].velTheta = GetOrbitalVelocity((_pDust[i].a + _pDust[i].b) / 2.0f);
		_pDust[i].center = { 0, 0 };

		// I want the outer parts to appear blue, the inner parts yellow. I'm imposing
		// the following temperature distribution (no science here it just looks right)
		_pDust[i].temp = _baseTemp + rad / 4.5f;

		_pDust[i].mag = 0.015f + 0.01f * (float)rand() / (float)RAND_MAX;
		int idx = (int)std::min(1.0f / dh * (_pDust[i].a + _pDust[i].b) / 2.0f, 99.0f);
		_numberByRad[idx]++;
	}

	// Initialise Dust
	for (int i = 0; i < _numH2; ++i)
	{
		x = 2 * (_radGalaxy) * ((float)rand() / (float)RAND_MAX) - (_radGalaxy);
		y = 2 * (_radGalaxy) * ((float)rand() / (float)RAND_MAX) - (_radGalaxy);
		rad = sqrt(x * x + y * y);

		int k1 = 2 * i;
		_pH2[k1].a = rad;
		_pH2[k1].b = rad * GetExcentricity(rad);
		_pH2[k1].angle = GetAngularOffset(rad);
		_pH2[k1].theta = 360.0f * ((float)rand() / (float)RAND_MAX);
		_pH2[k1].velTheta = GetOrbitalVelocity((_pH2[k1].a + _pH2[k1].b) / 2.0f);
		_pH2[k1].center = { 0, 0 };
		_pH2[k1].temp = 6000 + (6000 * ((float)rand() / (float)RAND_MAX)) - 3000;
		_pH2[k1].mag = 0.1f + 0.05f * (float)rand() / (float)RAND_MAX;
		int idx = (int)std::min(1.0f / dh * (_pH2[k1].a + _pH2[k1].b) / 2.0f, 99.0f);
		_numberByRad[idx]++;

		// Create second point 100 pc away from the first one
		int dist = 1000;
		int k2 = 2 * i + 1;
		_pH2[k2].a = (rad + dist);
		_pH2[k2].b = (rad /*+ dist*/)*GetExcentricity(rad /*+ dist*/);
		_pH2[k2].angle = GetAngularOffset(rad);
		_pH2[k2].theta = _pH2[k1].theta;
		_pH2[k2].velTheta = _pH2[k1].velTheta;
		_pH2[k2].center = _pH2[k1].center;
		_pH2[k2].temp = _pH2[k1].temp;
		_pH2[k2].mag = _pH2[k1].mag;
		idx = (int)std::min(1.0 / dh * ((double)_pH2[k2].a + _pH2[k2].b) / 2.0, 99.0);
		_numberByRad[idx]++;
	}
}

float Galaxy::GetBaseTemp() const noexcept
{
	return _baseTemp;
}

void Galaxy::SetBaseTemp(float baseTemp)
{
	_baseTemp = baseTemp;
	InitStars();
}

void Galaxy::SetDustRenderSize(float sz)
{
	_dustRenderSize = std::min(200.0f, std::max(sz, 1.0f));
}

Star* Galaxy::GetStars() const
{
	return _pStars;
}

Star* Galaxy::GetDust() const
{
	return _pDust;
}

Star* Galaxy::GetH2() const
{
	return _pH2;
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
			return (float)rho_so * (float)std::exp(-r / rH) * (r * r) * MathHelper::PI * d;
		}

		static float MH(float r)
		{
			float rho_h0 = 0.15f; // Dichte des Halos im Zentrum
			float rC = 2500;     // typische skalenlänge im Halo
			return (float)rho_h0 * 1 / (float)(1 + std::pow(r / rC, 2)) * (4 * MathHelper::PI * std::pow(r, 3) / 3);
		}

		// Velocity curve with dark matter
		static float v(float r)
		{
			float MZ = 100;
			return 20000.0f * (float)std::sqrt(MathHelper::CONTANT_OF_GRAVITY * (MH(r) + MS(r) + MZ) / r);
		}

		// velocity curve without dark matter
		static float vd(float r)
		{
			float MZ = 100;
			return 20000.0f * (float)std::sqrt(MathHelper::CONTANT_OF_GRAVITY * (MS(r) + MZ) / r);
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
	float u = 2.0f * MathHelper::PI * rad * MathHelper::PC_TO_KM;        // Umfang in km
	float time = u / (vel_kms * MathHelper::SEC_PER_YEAR);  // Umlaufzeit in Jahren

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

float Galaxy::GetTimeStep() const
{
	return _timeStep;
}

float Galaxy::GetTime() const
{
	return _time;
}

void Galaxy::CalcXY(Star &p, int pertN, float pertAmp)
{
	float beta = -p.angle;
	float alpha = p.theta * MathHelper::DEG_TO_RAD;

	// temporaries to save cpu time
	float cosalpha = std::cos(alpha);
	float sinalpha = std::sin(alpha);
	float cosbeta = std::cos(beta);
	float sinbeta = std::sin(beta);

	Vec2D ps = {
		p.center.x + (p.a * cosalpha * cosbeta - p.b * sinalpha * sinbeta),
		p.center.y + (p.a * cosalpha * sinbeta + p.b * sinalpha * cosbeta) };

	// Add small perturbations to create more spiral arms
	if (pertAmp > 0 && pertN > 0)
	{
		ps.x += (p.a / pertAmp) * sin(alpha * 2 * pertN);
		ps.y += (p.a / pertAmp) * cos(alpha * 2 * pertN);
	}

	p.pos = ps;
}

void Galaxy::SingleTimeStep(float time)
{
	_timeStep = time;
	_time += time;

	Vec2D posOld;
	for (int i = 0; i < _numStars; ++i)
	{
		_pStars[i].theta += (_pStars[i].velTheta * time);
		posOld = _pStars[i].pos;
		CalcXY(_pStars[i], _pertN, _pertAmp);

		Vec2D b = {
			_pStars[i].pos.x - posOld.x,
			_pStars[i].pos.y - posOld.y 
		};

		_pStars[i].vel = b;
	}

	for (int i = 0; i < _numDust; ++i)
	{
		_pDust[i].theta += (_pDust[i].velTheta * time);
		posOld = _pDust[i].pos;
		CalcXY(_pDust[i], _pertN, _pertAmp);
	}

	for (int i = 0; i < _numH2 * 2; ++i)
	{
		_pH2[i].theta += (_pH2[i].velTheta * time);
		posOld = _pDust[i].pos;
		CalcXY(_pH2[i], _pertN, _pertAmp);
	}
}

const Vec2D& Galaxy::GetStarPos(int idx)
{
	if (idx >= _numStars)
		throw std::runtime_error("Index out of bounds.");

	return _pStars[idx].pos; 
}

int Galaxy::GetNumH2() const
{
	return _numH2;
}

int Galaxy::GetNumStars() const
{
	return _numStars;
}

int Galaxy::GetNumDust() const
{
	return _numDust;
}