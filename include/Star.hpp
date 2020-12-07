#pragma once


struct Vec2D final
{
	Vec2D(float a_x = 0, float a_y = 0)
		: x(a_x)
		, y(a_y)
	{}

	float x;
	float y;
};


struct Vec3D final
{
	Vec3D(float a_x = 0, float a_y = 0, float a_z = 0)
		: x(a_x)
		, y(a_y)
		, z(a_z)
	{}

	float x;
	float y;
	float z;
};


struct Star final
{
	Star();
//	Star* Clone() const;

	void CalcXY(int pertN, float pertAmp);

	float m_theta;    // position auf der ellipse
	float m_velTheta; // angular velocity
	float m_angle;    // tilt of the ellipse
	float m_a;        // kleine halbachse
	float m_b;        // große halbachse
	float m_temp;     // star temperature
	float m_mag;      // brigtness;
	Vec2D  m_center;   // center of the elliptical orbit
	Vec2D  m_vel;      // Current velocity (calculated)
	Vec2D  m_pos;      // current position in kartesion koordinates
};
