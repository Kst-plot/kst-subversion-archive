/*
 *  Generic pass filter plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>

#define KST_UNUSED(x) if(x){};

double filter_calculate( double dFreqValue, const double inScalars[] );

int kst_pass_filter(
  const double *const inArrays[], 
  const int inArrayLens[],
  const double inScalars[],
  double *outArrays[], 
  int outArrayLens[],
	double outScalars[] );

int kst_pass_filter(
  const double *const inArrays[], 
  const int inArrayLens[],
  const double inScalars[],
  double *outArrays[], 
  int outArrayLens[],
	double outScalars[] ) {
  
  KST_UNUSED( outScalars )
  
  gsl_fft_real_wavetable* real;
  gsl_fft_real_workspace* work;
  gsl_fft_halfcomplex_wavetable* hc;
  double* pResult[1];
  double dFreqValue;
  int iLengthData;
  int iReturn = -1;
  int iStatus;
  int i;
  
  if( inScalars[1] > 0.0 ) {
    iLengthData = inArrayLens[0];
    
    if( iLengthData > 0 ) {
      if( outArrayLens[0] != iLengthData ) {
        pResult[0] = (double*)realloc( outArrays[0], iLengthData * sizeof( double ) );
      } else {
        pResult[0] = outArrays[0];
      }
      
      if( pResult[0] != NULL ) {
        outArrays[0] = pResult[0];
        outArrayLens[0] = iLengthData;
        
        real = gsl_fft_real_wavetable_alloc( iLengthData );
        if( real != NULL ) {
          work = gsl_fft_real_workspace_alloc( iLengthData );
          if( work != NULL ) {
            memcpy( pResult[0], inArrays[0], iLengthData * sizeof( double ) );
            
            //
            // calculate the FFT...
            //
            iStatus = gsl_fft_real_transform( pResult[0], 1, iLengthData, real, work );
            
            if( !iStatus ) {
              //
              // apply the filter...
              //
              for( i=0; i<iLengthData; i++ ) {
                dFreqValue = 0.5 * (double)i / (double)iLengthData;
                pResult[0][i] *= filter_calculate( dFreqValue, inScalars );
              }

              hc = gsl_fft_halfcomplex_wavetable_alloc( iLengthData );
              if( hc != NULL ) {
                //
                // calculate the inverse FFT...
                //
                iStatus = gsl_fft_halfcomplex_inverse( pResult[0], 1, iLengthData, hc, work );
                if( !iStatus ) {            
                  iReturn = 0;
                }
                gsl_fft_halfcomplex_wavetable_free( hc );
              }
            }

            gsl_fft_real_workspace_free( work );
          }
          gsl_fft_real_wavetable_free( real );
        }
        iReturn = 0;
      }
    }
  }
  
  return iReturn;
}
