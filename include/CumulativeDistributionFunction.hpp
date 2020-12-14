#pragma once


#include <vector>


class CumulativeDistributionFunction final
{
public:
	typedef double (CumulativeDistributionFunction::* dist_fun_t)(double x);

	CumulativeDistributionFunction();

	double ProbFromVal(double fVal);
	double ValFromProb(double fVal);

	void SetupRealistic(double I0, double k, double a, double RBulge, double min, double max, int nSteps);

private:
	dist_fun_t _pDistFun;
	double _fMin;
	double _fMax;
	double _fWidth;
	int _nSteps;

	// parameters for realistic star distribution
	double _I0;
	double _k;
	double _a;
	double _RBulge;

	std::vector<double> _vM1;
	std::vector<double> _vY1;
	std::vector<double> _vX1;

	std::vector<double> _vM2;
	std::vector<double> _vY2;
	std::vector<double> _vX2;

	void BuildCDF(int nSteps);

	double IntensityBulge(double R, double I0, double k);
	double IntensityDisc(double R, double I0, double a);
	double Intensity(double x);
};
