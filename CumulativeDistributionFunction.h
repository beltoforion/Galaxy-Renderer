#ifndef _CUMULATIVE_DISTRIBUTION_FUNCTION_H
#define _CUMULATIVE_DISTRIBUTION_FUNCTION_H

//-------------------------------------------------------------------------------------------------
#include <vector>


class CumulativeDistributionFunction
{
  public:
    typedef double (CumulativeDistributionFunction::*dist_fun_t)(double x);

    CumulativeDistributionFunction();
    virtual ~CumulativeDistributionFunction();

    double PropFromVal(double fVal);
    double ValFromProp(double fVal);

    void SetupRealistic(double I0, double k, double a, double RBulge, double min, double max, int nSteps);



  private:
    dist_fun_t m_pDistFun;
    double m_fMin;
    double m_fMax;
    double m_fWidth;
    int m_nSteps;

    // parameters for realistic star distribution
    double m_I0;
    double m_k;
    double m_a;
    double m_RBulge;

    std::vector<double> m_vM1;
    std::vector<double> m_vY1;
    std::vector<double> m_vX1;

    std::vector<double> m_vM2;
    std::vector<double> m_vY2;
    std::vector<double> m_vX2;

    void BuildCDF(int nSteps);


    double IntensityBulge(double R, double I0, double k);
    double IntensityDisc(double R, double I0, double a);
    double Intensity(double x);
};

#endif // CUMULATIVEDISTRIBUTIONFUNCTION_H
