#pragma once


/** \brief A class to store relevant constants. */
class MathHelper
{
public:

	/** \brief Convert parsec to kilometre. */
	static const float PC_TO_KM;

	/** \brief Seconds per year. */
	static const float SEC_PER_YEAR;

	/** \brief Deg to radian conversion faktor. */
	static const float DEG_TO_RAD;

	/** \brief Radian to deg conversion faktor. */
	static const float RAD_TO_DEG;

	/** \brief Constant of gravity. */
	static const float CONTANT_OF_GRAVITY;

	static const float PI;

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
