#pragma once


/** \brief A class to store relevant constants. */
class MathHelper
{
public:

	/** \brief Convert parsec to kilometre. */
	static const double PC_TO_KM;

	/** \brief Seconds per year. */
	static const double SEC_PER_YEAR;

	/** \brief Deg to radian conversion faktor. */
	static const double DEG_TO_RAD;

	/** \brief Radian to deg conversion faktor. */
	static const double RAD_TO_DEG;

	static inline unsigned int PowerTwoFloor(unsigned int val)
	{
		unsigned int power = 2, nextVal = power * 2;

		while ((nextVal *= 2) <= val)
		{
			power = power << 1; // *= 2;
		}

		return power << 1; // power * 2;
	}
};
