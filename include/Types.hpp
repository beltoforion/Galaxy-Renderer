#pragma once

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
	float theta0;     // initial angular position on the ellipse
	float velTheta;   // angular velocity
	float tiltAngle;  // tilt angle of the ellipse
	float a;          // kleine halbachse
	float b;          // große halbachse
	float temp;       // star temperature
	float mag;        // brightness;
	int32_t type;	  // Type 0:star, 1:dust, 2 and 3: h2 regions	
};
