#include "Star.h"

#include "Constants.h"
#include "OrbitCalculator.h"
#include <cmath>

using namespace std;

//------------------------------------------------------------------------
Star::Star() : m_theta(0), m_a(0), m_b(0), m_center(0, 0) {}

//-----------------------------------------------------------------------
const Vec2D &Star::CalcXY(int pertN, double pertAmp) {
  m_pos = OrbitCalculator::Compute(m_angle, m_a, m_b, m_theta, m_center, pertN,
                                   pertAmp);
  return m_pos;
}