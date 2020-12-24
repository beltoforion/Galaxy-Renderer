#include "CumulativeDistributionFunction.hpp"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <stdexcept>


CumulativeDistributionFunction::CumulativeDistributionFunction()
	: _vM1()
	, _vY1()
	, _vX1()
	, _vM2()
	, _vY2()
	, _vX2()
	, _fMin()
	, _fMax()
	, _nSteps()
	, _I0()
	, _k()
	, _a()
	, _RBulge()
{}


void CumulativeDistributionFunction::SetupRealistic(double I0, double k, double a, double RBulge, double min, double max, int nSteps)
{
	_fMin = min;
	_fMax = max;
	_nSteps = nSteps;

	_I0 = I0;
	_k = k;
	_a = a;
	_RBulge = RBulge;

	BuildCDF(_nSteps);
}

void CumulativeDistributionFunction::BuildCDF(int nSteps)
{
	double h = (_fMax - _fMin) / nSteps;
	double x = 0, y = 0;

	_vX1.clear();
	_vY1.clear();
	_vX2.clear();
	_vY2.clear();
	_vM1.clear();
	_vM2.clear();

	// Simpson rule for integration of the distribution function
	_vY1.push_back(0.0);
	_vX1.push_back(0.0);
	for (int i = 0; i < nSteps; i += 2)
	{
		x = h * (i + 2);
		y += h / 3 * (Intensity(_fMin + i * h) + 4 * Intensity(_fMin + (i + 1) * h) + Intensity(_fMin + (i + 2) * h));

		_vM1.push_back((y - _vY1.back()) / (2 * h));
		_vX1.push_back(x);
		_vY1.push_back(y);

		//    printf("%2.2f, %2.2f, %2.2f\n", m_fMin + (i+2) * h, v, h);
	}
	_vM1.push_back(0.0);

	// all arrays must have the same length
	if (_vM1.size() != _vX1.size() || _vM1.size() != _vY1.size())
		throw std::runtime_error("CumulativeDistributionFunction::BuildCDF: array size mismatch (1)!");

	// normieren
	for (std::size_t i = 0; i < _vY1.size(); ++i)
	{
		_vY1[i] /= _vY1.back();
		_vM1[i] /= _vY1.back();
	}

	_vX2.push_back(0.0);
	_vY2.push_back(0.0);

	double p = 0;
	h = 1.0 / nSteps;
	for (int i = 1, k = 0; i < nSteps; ++i)
	{
		p = (double)i * h;

		for (; _vY1[k + 1] <= p; ++k)
		{
		}


		y = _vX1[k] + (p - _vY1[k]) / _vM1[k];

		//    printf("%2.4f, %2.4f, k=%d, %2.4f, %2.4f\n", p, y, k, m_vY1[k], m_vM1[k]);

		_vM2.push_back((y - _vY2.back()) / h);
		_vX2.push_back(p);
		_vY2.push_back(y);
	}
	_vM2.push_back(0.0);

	// all arrays must have the same length
	if (_vM2.size() != _vX2.size() || _vM2.size() != _vY2.size())
		throw std::runtime_error("CumulativeDistributionFunction::BuildCDF: array size mismatch (1)!");

}


double CumulativeDistributionFunction::ProbFromVal(double fVal)
{
	if (fVal<_fMin || fVal>_fMax)
		throw std::runtime_error("out of range");

	double h = 2 * ((_fMax - _fMin) / _nSteps);
	int i = (int)((fVal - _fMin) / h);
	double remainder = fVal - i * h;

	//  printf("fVal=%2.2f; h=%2.2f; i=%d; m_vVal[i]=%2.2f; m_vAsc[i]=%2.2f;\n", fVal, h, i, m_vVal[i], m_vAsc[i]);

	assert(i >= 0 && i < (int)_vM1.size());
	return (_vY1[i] + _vM1[i] * remainder) /* / m_vVal.back()*/;
}


double CumulativeDistributionFunction::ValFromProb(double fVal)
{
	if (fVal < 0 || fVal>1)
		throw std::runtime_error("out of range");

	double h = 1.0 / (_vY2.size() - 1);

	int i = (int)(fVal / h);
	double remainder = fVal - i * h;

	assert(i >= 0 && i < (int)_vM2.size());
	return (_vY2[i] + _vM2[i] * remainder) /* / m_vVal.back()*/;
}


double CumulativeDistributionFunction::IntensityBulge(double R, double I0, double k)
{
	return I0 * exp(-k * pow(R, 0.25));
}


double CumulativeDistributionFunction::IntensityDisc(double R, double I0, double a)
{
	return I0 * exp(-R / a);
}


double CumulativeDistributionFunction::Intensity(double x)
{
	return (x < _RBulge) ? IntensityBulge(x, _I0, _k) : IntensityDisc(x - _RBulge, IntensityBulge(_RBulge, _I0, _k), _a);
}
