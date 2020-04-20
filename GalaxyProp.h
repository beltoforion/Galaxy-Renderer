#ifndef _GALAXY_PROP_H
#define _GALAXY_PROP_H

//-------------------------------------------------------------------------------------------
/** \brief Encapsulates the data needed to define the galaxies shape. */
class GalaxyProp {
  friend class Galaxy;

  GalaxyProp(double radDisk, double radCore, double deltaAng, double exInner,
             double exOuter, double velInner, double velOuter);

public:
  // Excentricity of the orbital ellipses
  double m_excInner; ///< Excentricity of the innermost ellipse
  double m_excOuter; ///< Excentricity of the outermost ellipse

  // Orbital Velocities
  double m_velOrigin; ///< Velovity at the innermost core in km/s
  double m_velInner;  ///< Velocity at the core edge in km/s
  double m_velOuter;  ///< Velocity at the edge of the disk in km/s

  // Shape of the density wave
  double m_angleOffset;    ///< Angular of the orbits offset per parsec
  double m_angleOffsetInc; ///< Increment of the angular offset
  double m_velAngle;       ///< Angular velocity of the density waves

  // Galaxy radii
  double m_radCore;     ///< Radius of the inner core
  double m_radGalaxy;   ///< Radius of the galaxy
  double m_radFarField; ///< The radius after which all density waves must have
                        ///< circular shape
};

#endif // _GALAXY_PROP_H
