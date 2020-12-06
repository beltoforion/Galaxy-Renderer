#pragma once

/* A colour system is defined by the CIE x and y coordinates of
   its three primary illuminants and the x and y coordinates of
   the white point. */
struct colourSystem
{
	const char* name;		// Colour system name
	double xRed, yRed;		// Red x, y
	double xGreen, yGreen;	// Green x, y
	double 	xBlue, yBlue;	// Blue x, y
	double xWhite, yWhite;	// White point x, y
	double gamma;			// Gamma correction for system
};

extern colourSystem NTSCsystem;
extern colourSystem EBUsystem;
extern colourSystem SMPTEsystem;
extern colourSystem CIEsystem;
extern colourSystem Rec709system;
extern double bbTemp;                 /* Hidden temperature argument */

void spectrum_to_xyz(double (*spec_intens)(double wavelength), double* x, double* y, double* z);
extern void xyz_to_rgb(struct colourSystem* cs, double xc, double yc, double zc, double* r, double* g, double* b);
extern double bb_spectrum(double wavelength);
extern void norm_rgb(double* r, double* g, double* b);
extern double bb_spectrum(double wavelength);
