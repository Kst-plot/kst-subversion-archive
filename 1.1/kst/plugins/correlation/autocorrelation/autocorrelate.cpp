/*
 *  Autocorrelation plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>

extern "C" int autocorrelate(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int autocorrelate(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
  double* pdArrayOne;
  double* pdResult[2];
  double  dReal;
  double  dImag;
  int i = 0;
  int iLength;
  int iLengthNew;
  int iReturn = -1;

  if( inArrayLens[0] ) 
  {
    //
    // zero-pad the array...
    //
    iLength  = inArrayLens[0];
    iLength *= 2;
    
    //
    // round iLength up to the nearest power of two...
    //
    iLengthNew = 64;
    while( iLengthNew < iLength && iLengthNew > 0 )
    {
      iLengthNew *= 2;
    }
    iLength = iLengthNew;

    if( iLength > 0 )
    {
      pdArrayOne = new double[iLength];
      if( pdArrayOne != NULL )
      {
        //
        // zero-pad the two arrays...
        //
        memset( pdArrayOne, 0, iLength * sizeof( double ) );
        memcpy( pdArrayOne, inArrays[0], inArrayLens[0] * sizeof( double ) );

        //
        // calculate the FFTs of the two functions...
        //
        if( gsl_fft_real_radix2_transform( pdArrayOne, 1, iLength ) == 0 )
        {
          //
          // multiply the FFT by its complex conjugate...
          //
          for( i=0; i<iLength/2; i++ )
          {
            if( i==0 || i==(iLength/2)-1 )
            {
              pdArrayOne[i] *= pdArrayOne[i];
            }
            else
            {
              dReal = pdArrayOne[i] * pdArrayOne[i] + pdArrayOne[iLength-i] * pdArrayOne[iLength-i];
              dImag = pdArrayOne[i] * pdArrayOne[iLength-i] - pdArrayOne[iLength-i] * pdArrayOne[i];

              pdArrayOne[i] = dReal;
              pdArrayOne[iLength-i] = dImag;
            }
          }

          //
          // do the inverse FFT...
          //
          if( gsl_fft_halfcomplex_radix2_inverse( pdArrayOne, 1, iLength ) == 0 )
          {
            if( outArrayLens[0] != inArrayLens[0] )
            {
              pdResult[0] = (double*)realloc( outArrays[0], inArrayLens[0] * sizeof( double ) );
            }
            else
            {
              pdResult[0] = outArrays[0];
            }

            if( outArrayLens[1] != inArrayLens[1] )
            {
              pdResult[1] = (double*)realloc( outArrays[1], inArrayLens[1] * sizeof( double ) );
            }
            else
            {
              pdResult[1] = outArrays[1];
            }

            if( pdResult[0] != NULL && pdResult[1] != NULL )
            {
              outArrays[0] = pdResult[0];
              outArrayLens[0] = inArrayLens[0];

              outArrays[1] = pdResult[1];
              outArrayLens[1] = inArrayLens[1];

              for( i=0; i<inArrayLens[0]; i++ )
              {
                  outArrays[0][i] = (double)( i - ( inArrayLens[0] / 2 ) );
              }

              memcpy( &(outArrays[1][inArrayLens[0] / 2]),
                      &(pdArrayOne[0]),
                      ( ( inArrayLens[0] + 1 ) / 2 ) * sizeof( double ) );

              memcpy( &(outArrays[1][0]),
                      &(pdArrayOne[iLength - (inArrayLens[0] / 2)]),
                      ( inArrayLens[0] / 2 ) * sizeof( double ) );

              iReturn = 0;
            }
          }
        }
        
        delete[] pdArrayOne;
      }
    }
  }
  
  return iReturn;
}
