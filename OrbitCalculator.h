#pragma once

#include "Star.h"

class OrbitCalculator
{
public:
    
    static Vec2D Compute(double angle, double a, double b, double theta, const Vec2D &p, int pertN, double pertAmp);
};
