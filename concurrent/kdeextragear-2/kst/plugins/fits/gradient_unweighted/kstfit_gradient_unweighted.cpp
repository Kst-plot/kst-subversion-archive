/*
 *  Gradient unweighted fitting plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_fit.h>

#define X 0
#define Y 1

extern "C" int kstfit_gradient_unweighted(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int kstfit_gradient_unweighted(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
  int i = 0;
  int	iLength;
  int iReturn = -1;
  double* pResult[4];
  double c1 = 0.0;
  double cov11 = 0.0;
  double dSumSq = 0.0;
  double y;
  double yErr;
  
  if (inArrayLens[Y] >= 2 && inArrayLens[X] >= 2) {
    iLength = inArrayLens[Y];
    if( inArrayLens[X] < iLength ) {
      iLength = inArrayLens[X];
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
            
      if( !gsl_fit_mul( inArrays[X], 1, inArrays[Y], 1, iLength, &c1, &cov11, &dSumSq ) ) {
        for( i=0; i<iLength; i++ ) {
          gsl_fit_mul_est( inArrays[X][i], c1, cov11, &y, &yErr );
          outArrays[0][i] = y;
          outArrays[1][i] = y - yErr;
          outArrays[2][i] = y + yErr;
          outArrays[3][i] = inArrays[Y][i] - y;
        }
        
        outScalars[0] = c1;
        outScalars[1] = cov11;
        outScalars[2] = dSumSq;
        
        iReturn = 0;
      }
    } 
  }
  
  return iReturn;
}
