#include <cmath>
#include "Intensity.h"

using namespace std;


double IntensityBulge(double R, double I0, double k)
{
  return I0 * exp(-k*pow(R, 0.25));
}

double IntensityDisc(double R, double I0, double a)
{
  return I0 * exp(-R/a);
}

double Intensity(double x)
{
  double I0 = 1;
  double k=0.2;
  double RBulge = 3;
  return (x<RBulge) ? IntensityBulge(x, I0, k) : IntensityDisc(x-RBulge, IntensityBulge(RBulge, I0, k), 5);
//  return sin(x);
}
