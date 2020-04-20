/*
 * File:   main.cpp
 * Author: user
 *
 * Created on 5. Juli 2009, 22:15
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>

//------------------------------------------------------------------------------
#include "NBodyWnd.h"

using namespace std;

void ShowHelp() {
  printf("Keyboard commands\r\n");
  printf("Camera\r\n");
  printf("  1     - centric; fixed\r\n");
  //	printf("  2     - centric; rotating with core speed\r\n");
  //	printf("  3     - centric; rotating with speed of outer disc\r\n");
  printf("Galaxy geometry\r\n");
  printf("  q     - increase inner excentricity\r\n");
  printf("  a     - decrease inner excentricity\r\n");
  printf("  w     - increase outer excentricity\r\n");
  printf("  s     - decrease outer excentricity\r\n");
  printf("  e     - increase angular shift of orbits\r\n");
  printf("  d     - decrease angular shift of orbits\r\n");
  printf("  r     - increase core size\r\n");
  printf("  f     - decrease core size\r\n");
  printf("  t     - increase galaxy size\r\n");
  printf("  g     - decrease galaxy size\r\n");
  printf("Display features\r\n");
  printf("  F1    - Help screen\r\n");
  printf("  F2    - Galaxy data\r\n");
  printf("  F3    - Stars (on/off)\r\n");
  printf("  F4    - Dust (on/off)\r\n");
  printf("  F5    - H2 Regions (on/off)\r\n");
  printf("  F6    - Density waves (Star orbits)\r\n");
  printf("  F7    - Axis\r\n");
  printf("  F8    - Radii\r\n");
  printf("  +     - Zoom in\r\n");
  printf("  -     - Zoom out\r\n");
  printf("  b     - Decrease Dust Render Size\r\n");
  printf("  n     - Increase Dust Render Size\r\n");
  printf("  m     - Toggle dark matter on/off\r\n");
  printf("Misc\r\n");
  printf("  pause - halt simulation\r\n");
  printf("  F12   - Write frames to TGA\r\n");
  printf("Predefined Galaxies\r\n");
  printf("  Keypad 0 - 4\r\n");
}

int main(int argc, char **argv) {
  try {
    ShowHelp();
    NBodyWnd wndMain(3000, 2000, "Density wave simulation");

    // Define simulation size
    wndMain.Init(10000);
    wndMain.MainLoop();
  } catch (std::exception &exc) {
    std::cout << "Fatal error: " << exc.what() << std::endl;
  } catch (...) {
    std::cout << "Fatal error: unexpected exception" << std::endl;
  }

  return (EXIT_SUCCESS);
}
