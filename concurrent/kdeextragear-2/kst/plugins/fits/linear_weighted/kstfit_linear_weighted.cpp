/*
 *  Linear weighted fitting plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_fit.h>

#define X 0
#define Y 1
#define W 2

extern "C" int kstfit_linear_weighted(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int kstfit_linear_weighted(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
  int i = 0;
  int	iLength;
  int iReturn = -1;
  double* pResult[4];
  double c0 = 0.0;
  double c1 = 0.0;
  double cov00 = 0.0;
  double cov01 = 0.0;
  double cov11 = 0.0;
  double dSumSq = 0.0;
  double y;
  double yErr;
  
  if (inArrayLens[Y] >= 2 && inArrayLens[X] >= 2) {
    iLength = inArrayLens[Y];
    if( inArrayLens[X] < iLength ) {
      iLength = inArrayLens[X];
    }
    if( inArrayLens[W] < iLength ) {
      iLength = inArrayLens[W];
    }
    
    for( i=0; i<4; i++ ) {
      if( outArrayLens[0] != iLength ) {
        pResult[i] = (double*)realloc( outArrays[i], iLength * sizeof( double ) );
      } else {
        pResult[i] = outArrays[i];
      }
    }
    
    if( pResult[0] != NULL && 
        pResult[1] != NULL && 
        pResult[2] != NULL && 
        pResult[3] != NULL )
    {
      for( i=0; i<4; i++ ) {
        outArrays[i] 		= pResult[i];
        outArrayLens[i] = iLength;
      }
            
      if( !gsl_fit_wlinear( inArrays[X], 1, inArrays[W], 1, inArrays[Y], 1, iLength, &c0, &c1, &cov00, &cov01, &cov11, &dSumSq ) ) {
        
        for( i=0; i<iLength; i++ ) {
          gsl_fit_linear_est( inArrays[X][i], c0, c1, cov00, cov01, cov11, &y, &yErr );
          outArrays[0][i] = y;
          outArrays[1][i] = y - yErr;
          outArrays[2][i] = y + yErr;
          outArrays[3][i] = inArrays[Y][i] - y;
        }
        
        outScalars[0] = c0;
        outScalars[1] = c1;
        outScalars[2] = cov00;
        outScalars[3] = cov01;
        outScalars[4] = cov11;
        outScalars[5] = dSumSq;
        
        iReturn = 0;
      }
    } 
  }
  
  return iReturn;
}
