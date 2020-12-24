#pragma once


#include <vector>


class CumulativeDistributionFunction final
{
public:
	CumulativeDistributionFunction();

	double ProbFromVal(double fVal);
	double ValFromProb(double fVal);

	void SetupRealistic(double I0, double k, double a, double RBulge, double min, double max, int nSteps);

private:
	double _fMin;
	double _fMax;
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
