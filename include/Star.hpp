#pragma once


struct Vec2D final
{
	float x;
	float y;
};


struct Vec3D final
{
	float x;
	float y;
	float z;
};


struct Star final
{
	Star()
		: theta(0)
		, velTheta(0)
		, angle(0)
		, a(0)
		, b(0)
		, temp(0)
		, mag(0)
		, center({ 0, 0 })
		, vel({ 0, 0 })
		, pos({ 0, 0 })
	{}

	float theta;    // position auf der ellipse
	float velTheta; // angular velocity
	float angle;    // tilt of the ellipse
	float a;        // kleine halbachse
	float b;        // große halbachse
	float temp;     // star temperature
	float mag;      // brightness;

	Vec2D center;   // center of the elliptical orbit
	Vec2D vel;      // Current velocity (calculated)
	Vec2D pos;      // current position in kartesion koordinates
};
