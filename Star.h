#ifndef STAR_H
#define STAR_H

#include "Vector.h"

/*
class OrbitCalculator
{
    static Vec2D Compute(double angle, double a, double b, double theta, const
Vec2D &center, double pertN, double pertAmp);
};
*/
//------------------------------------------------------------------------
class Star {
public:
  Star();
  const Vec2D &CalcXY(int pertN, double pertAmp);

  double m_theta;    // position auf der ellipse
  double m_velTheta; // angular velocity
  double m_angle;    // Schräglage der Ellipse
  double m_a;        // kleine halbachse
  double m_b;        // große halbachse
  double m_temp;     // star temperature
  double m_mag;      // brigtness;
  Vec2D m_center;    // center of the elliptical orbit
  Vec2D m_vel;       // Current velocity (calculated)
  Vec2D m_pos;       // current position in kartesion koordinates
};

#endif