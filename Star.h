#pragma once


struct Vec2D final
{
public:

	Vec2D(double a_x = 0, double a_y = 0)
		:x(a_x)
		, y(a_y)
	{}

	double x;
	double y;
};


struct Vec3D final
{
public:

	Vec3D(double a_x = 0, double a_y = 0, double a_z = 0)
		: x(a_x)
		, y(a_y)
		, z(a_z)
	{}

	double x;
	double y;
	double z;
};


class Star final
{
public:

	Star();
	void CalcXY(int pertN, double pertAmp);

	double m_theta;    // position auf der ellipse
	double m_velTheta; // angular velocity
	double m_angle;    // Schräglage der Ellipse
	double m_a;        // kleine halbachse
	double m_b;        // große halbachse
	double m_temp;     // star temperature
	double m_mag;      // brigtness;
	Vec2D  m_center;   // center of the elliptical orbit
	Vec2D  m_vel;      // Current velocity (calculated)
	Vec2D  m_pos;      // current position in kartesion koordinates
};
