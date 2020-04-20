#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#include <cmath>

//--------------------------------------------------------------------------
/** \brief A class to store relevant constants. */
class Constant {
public:
  /** \brief Convert parsec to kilometre. */
  static constexpr double PC_TO_KM = 3.08567758129e13;

  /** \brief Seconds per year. */
  static constexpr double SEC_PER_YEAR = 365.25 * 86400;

  /** \brief Deg to radian conversion faktor. */
  static constexpr double DEG_TO_RAD = M_PI / 180.0;

  /** \brief Radian to deg conversion faktor. */
  static constexpr double RAD_TO_DEG = 180.0 / M_PI;
};

#endif // _CONSTANTS_H
