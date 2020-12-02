#include "Star.h"

#include "MathHelper.h"
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


void Star::CalcXY(int pertN, double pertAmp)
{
	double beta = -m_angle;
	double alpha = m_theta * MathHelper::DEG_TO_RAD;

	// temporaries to save cpu time
	double cosalpha = std::cos(alpha);
	double sinalpha = std::sin(alpha);
	double cosbeta = std::cos(beta);
	double sinbeta = std::sin(beta);

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