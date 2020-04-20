#include "OrbitCalculator.h"
#include "Constants.h"
#include <cmath>

using namespace std;

Vec2D OrbitCalculator::Compute(double angle, double a, double b, double theta,
                               const Vec2D &p, int pertN, double pertAmp) {
  double beta = -angle, alpha = theta * Constant::DEG_TO_RAD;

  // temporaries to save cpu time
  double cosalpha = cos(alpha), sinalpha = sin(alpha), cosbeta = cos(beta),
         sinbeta = sin(beta);

  Vec2D pos = Vec2D(p.x + (a * cosalpha * cosbeta - b * sinalpha * sinbeta),
                    p.y + (a * cosalpha * sinbeta + b * sinalpha * cosbeta));

  // Add small perturbations to create more spiral arms
  if (pertAmp > 0 && pertN > 0) {
    pos.x += (a / pertAmp) * sin(alpha * 2 * pertN);
    pos.y += (a / pertAmp) * cos(alpha * 2 * pertN);
  }

  return pos;
}