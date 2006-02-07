/*
 *  Phase plugin for KST.
 *  Copyright 2003, The University of British Columbia
 *  Released under the terms of the GPL.
 *
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#define KST_UNUSED(x) if(x){};

extern "C" int noise_addition( 
    const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] );

int noise_addition( 
    const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[] )
{
  KST_UNUSED( outScalars )
   
  const gsl_rng_type* pGeneratorType;
  gsl_rng* pRandomNumberGenerator;
  double* pResult[1];
  int iRetVal = -1;
  int iLength = inArrayLens[0];
  int i;
  
  if( iLength > 0 ) {   
    if( outArrayLens[0] != iLength ) {
      pResult[0] = (double*)realloc( outArrays[0], iLength * sizeof( double ) );
    } else {
      pResult[0] = outArrays[0];
    }
  }
  
  pGeneratorType = gsl_rng_default;
  pRandomNumberGenerator = gsl_rng_alloc( pGeneratorType );
  if( pRandomNumberGenerator != NULL ) {
    if( pResult[0] != NULL ) {	
      outArrays[0] = pResult[0];
      outArrayLens[0] = iLength;
      
      for( i=0; i<iLength; i++ ) {
        outArrays[0][i] = inArrays[0][i] + gsl_ran_gaussian( pRandomNumberGenerator, inScalars[0] );
      }
            
      iRetVal = 0;
    }
    gsl_rng_free( pRandomNumberGenerator );
  }
  
  return iRetVal;
}
