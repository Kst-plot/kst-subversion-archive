/*
 *  Butterworth low pass filter plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#include <stdlib.h>
#include <math.h>
#include "../filters.h"

extern "C" int butterworth_bandstop(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int butterworth_bandstop(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
  int iReturn;
  
  iReturn = kst_pass_filter( inArrays, 
                             inArrayLens, 
                             inScalars, 
                             outArrays, 
                             outArrayLens,
                             outScalars );
  
  return iReturn;
}

double filter_calculate( double dFreqValue, const double inScalars[] ) {
  double dValue;
  
  dValue  = dFreqValue * inScalars[2];
  dValue /= ( (dFreqValue * dFreqValue) - (inScalars[1] * inScalars[1]) );
  dValue  = 1.0 / ( 1.0 + pow( dValue, 2.0 * (double)inScalars[0] ) ); 
  
  return dValue;
}
