/*
 *  Periodic cubic spline interpolation plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_spline.h>
#include "../interpolations.h"

extern "C" int kstinterp_cspline_periodic(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int kstinterp_cspline_periodic(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
  return interpolate( inArrays, inArrayLens, inScalars, outArrays, outArrayLens, outScalars, gsl_interp_cspline_periodic);
}
