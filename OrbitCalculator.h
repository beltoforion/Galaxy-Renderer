#ifndef ORBIT_CALCULATOR_H
#define ORBIT_CALCULATOR_H

#include "Vector.h"

class OrbitCalculator {
public:
  static Vec2D Compute(double angle, double a, double b, double theta,
                       const Vec2D &p, int pertN, double pertAmp);
};

#endif