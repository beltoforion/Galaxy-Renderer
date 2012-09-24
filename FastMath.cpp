#include "FastMath.h"
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>


//--------------------------------------------------------------------------------
int FastMath::s_num = 0;
double *FastMath::s_sin = NULL;
double *FastMath::s_cos = NULL;
double FastMath::s_da = 0;

//--------------------------------------------------------------------------------
void FastMath::init(int num)
{
  s_num = num;
  s_sin = new double [num];
  s_cos = new double [num];
  s_da = 2*M_PI / (double)s_num;

  for (int i=0; i<s_num; ++i)
  {
    FastMath::s_sin[i] = ::sin(s_da * i);
    FastMath::s_cos[i] = ::cos(s_da * i);
  }
}

//--------------------------------------------------------------------------------
void FastMath::self_test()
{
  for (double v = -3 * M_PI; v<3*M_PI; v+=0.04)
  {
    double v0, v1, diff;

    v0 = ::sin(v),
    v1 = FastMath::sin(v),
    diff = v1-v0;
    std::cout << "sin(" << v << ") = " << v0 << ", approx: " << v1 << ", diff: " << diff << "\n";

    v0 = ::cos(v),
    v1 = FastMath::cos(v),
    diff = v1-v0;
    std::cout << "cos(" << v << ") = " << v0 << ", approx: " << v1 << ", diff: " << diff << "\n";
  }
}

//--------------------------------------------------------------------------------
void FastMath::release()
{
  delete [] s_sin;
  delete [] s_cos;
  s_num = 0;
  s_da = 0;
}

//--------------------------------------------------------------------------------
double FastMath::sin(double v)
{
  int idx = (int)(v/s_da) % s_num;
  return (idx<0) ? -s_sin[-idx] : s_sin[idx];
}

//--------------------------------------------------------------------------------
double FastMath::cos(double v)
{
  int idx = (int)(v/s_da) % s_num;
  return (idx<0) ? s_cos[-idx] : s_cos[idx];
}

//--------------------------------------------------------------------------------
double FastMath::sqr(double v)
{
  return v*v;
}

//--------------------------------------------------------------------------------
double FastMath::nvzz(double m, double s)
{
  double sum = -6;
  for (int i=0; i<12; ++i)
  {
    sum += (double)rand()/(double)RAND_MAX;
  }

  return s*sum + m;
}

//--------------------------------------------------------------------------------
// Helligkeitsverteilung einer elliptischen Galaxie als Funktion des Radius
// nach Freeman
double IntensityDisk(double r, double i0, double a)
{
  return i0*exp(-r/a);
}


//--------------------------------------------------------------------------------
// Intensitätsverteilung im Zentralbereich
double IntensityBulge(double r, double i0, double k)
{
  return i0*exp(-k* pow(r, 0.25));
}

//--------------------------------------------------------------------------------
// Intensitätsverteilung Scheibe und Zentralbereich
double Intensity(double r, double r_bulge, double i0, double a, double k)
{
  return (r<r_bulge) ? IntensityBulge(r, i0, k) : IntensityDisk(r-r_bulge, IntensityBulge(r_bulge, i0, k), a);
}
