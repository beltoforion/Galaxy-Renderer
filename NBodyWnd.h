/*
 * File:   NBodyWnd.h
 * Author: user
 *
 * Created on 8. Juli 2009, 21:54
 */

#ifndef _NBODYWND_H
#define _NBODYWND_H

#include <fstream>
#include <stdint.h>

//--- Framework ----------------------------------------------------------------
#include "Galaxy.h"
#include "SDLWnd.h"

//------------------------------------------------------------------------------
/** \brief Main window of th n-body simulation. */
class NBodyWnd : public SDLWindow {
public:
  NBodyWnd(const int sz_w, const int sz_h, const std::string caption);
  virtual ~NBodyWnd();

  virtual void Render();
  virtual void OnProcessEvents(uint8_t type);

  void Init(int num);

private:
  enum EDisp {
    dspNONE = 0,
    dspAXIS = 1 << 0,
    dspSTARS = 1 << 1,
    dspSTAT = 1 << 2,
    dspPAUSE = 1 << 3,
    dspHELP = 1 << 4,
    dspROI = 1 << 5,
    dspDENSITY_WAVES = 1 << 6,
    dspRADII = 1 << 7,
    dspVELOCITY = 1 << 8,
    dspDUST = 1 << 9,
    dspH2 = 1 << 10
  };

  struct Color {
    double r;
    double g;
    double b;
  };

  NBodyWnd(const NBodyWnd &orig);

  void DrawStars();
  void DrawDust();
  void DrawH2();
  void DrawStat();
  void DrawHelp();
  void DrawGalaxyRadii();
  void DrawCenterOfMass();
  void DrawDensityWaves(int num, double rad);
  void DrawVelocity();
  void DrawEllipsis(double a, double b, double angle);
  Color ColorFromTemperature(double temp) const;

  int m_camOrient; ///< Index of the camera orientation to use
  int m_starRenderType;
  double m_roi;     ///< Radius of the region of interest
  uint32_t m_flags; ///< The display flags
  bool m_bDumpImage;

  Galaxy m_galaxy;

  // Star color management
  int m_colNum;
  double m_t0, m_t1, m_dt;
  Color m_col[200];
};

#endif /* _NBODYWND_H */
