#include "Star.hpp"

#include "MathHelper.hpp"
#include <cmath>


Star::Star()
	: m_theta(0)
	, m_velTheta(0)
	, m_angle(0)
	, m_a(0)
	, m_b(0)
	, m_temp(0)
	, m_mag(0)
	, m_center(0, 0)
	, m_vel(0, 0)
	, m_pos(0, 0)
{}

//Star* Star::Clone() const
//{
//	Star *newStar = new Star(
//	{
//		m_theta,
//		m_velTheta,
//		double m_angle,
//		double m_a,
//		double m_b,
//		double m_temp,
//		double m_mag,
//		Vec2D  m_center,
//		Vec2D  m_vel,
//		Vec2D  m_pos
//	});
//}

void Star::CalcXY(int pertN, float pertAmp)
{
	float beta = -m_angle;
	float alpha = m_theta * MathHelper::DEG_TO_RAD;

	// temporaries to save cpu time
	float cosalpha = std::cos(alpha);
	float sinalpha = std::sin(alpha);
	float cosbeta = std::cos(beta);
	float sinbeta = std::sin(beta);

	Vec2D pos = Vec2D(
		m_center.x + (m_a * cosalpha * cosbeta - m_b * sinalpha * sinbeta),
		m_center.y + (m_a * cosalpha * sinbeta + m_b * sinalpha * cosbeta));

	// Add small perturbations to create more spiral arms
	if (pertAmp > 0 && pertN > 0)
	{
		pos.x += (m_a / pertAmp) * sin(alpha * 2 * pertN);
		pos.y += (m_a / pertAmp) * cos(alpha * 2 * pertN);
	}

	m_pos = pos;
}