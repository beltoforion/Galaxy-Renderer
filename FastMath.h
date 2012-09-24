#ifndef _FAST_MATH_H
#define _FAST_MATH_H

//------------------------------------------------------------------------------------
#include <cmath>

//--------------------------------------------------------------------------------
double IntensityDisk(double r, double i0, double a);

//--------------------------------------------------------------------------------
// Intensitätsverteilung im Zentralbereich
double IntensityBulge(double r, double i0, double k);

//--------------------------------------------------------------------------------
// Intensitätsverteilung Scheibe und Zentralbereich
double Intensity(double r, double r_bulge, double i0, double a, double k);


//------------------------------------------------------------------------------------
/** \brief Fast math functions using table lookup or other optimizations. */
class FastMath
{
public:

  static void init(int num = 10000);
  static void release();

  static double nvzz(double m, double s);
  static double sin(double v);
  static double cos(double v);
  static double sqr(double v);

  static void self_test();

private:

  static int s_num;
  static double s_da;
  static double *s_sin;
  static double *s_cos;
};

#endif
