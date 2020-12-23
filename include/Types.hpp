#pragma once

#include <cstdint>

struct Vec2 final
{
	float x, y;
};

struct Vec3 final
{
	float x, y, z;
};

struct Color
{
	float r, g, b, a;
};

struct Star
{
	Vec2 pos;         // current position in kartesian coordinates
	Vec2 vel;         // current velocity (calculated)

	float theta0;     // initial angular position on the ellipse
	float velTheta;   // angular velocity
	float tiltAngle;  // tilt angle of the ellipse
	float a;          // kleine halbachse
	float b;          // große halbachse
	Vec2 center;      // center of the elliptical orbit

	float temp;       // star temperature
	float mag;        // brightness;

	int32_t type;	  // Type 0 for star, 1 for dust	
};
