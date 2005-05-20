/*
 *  Convolution plugin for KST.
 *  Copyright 2004, The University of British Columbia
 *  Released under the terms of the GPL.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_fft_real.h>
#include <gsl/gsl_fft_halfcomplex.h>

extern "C" int convolve(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int convolve(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
  double* pdResponse;
  double* pdConvolve;
  double* pdResult;
  double  dReal;
  double  dImag;
  int i = 0;
  int iLength;
  int iLengthNew;
  int iReturn = -1;
  int iResponseMidpoint;
  int iResponseIndex = 1;
  int iConvolveIndex = 0;

  if( inArrayLens[0] > 0 && inArrayLens[1] > 0 )
  {
    //
    // determine which is the response function:
    //  i.e. which is shorter...
    //
    iLength = inArrayLens[0];
    if( inArrayLens[1] > iLength )
    {
      iResponseIndex = 0;
      iConvolveIndex = 1;
    }

    iResponseMidpoint = inArrayLens[iResponseIndex] / 2;
    iLength = inArrayLens[iConvolveIndex] + iResponseMidpoint;

    //
    // round iLength up to the nearest factor of two...
    //
    iLengthNew = 64;
    while( iLengthNew < iLength && iLengthNew > 0 )
    {
      iLengthNew *= 2;
    }
    iLength = iLengthNew;

    if( iLength > 0 )
    {
      pdResponse = new double[iLength];
      pdConvolve = new double[iLength];
      if( pdResponse != NULL && pdConvolve != NULL )
      {
        //
        // sort the response function into wrap-around order...
        //
        memset( pdResponse, 0, iLength * sizeof( double ) );

        for( i=0; i<iResponseMidpoint; i++ )
        {
          pdResponse[i]                           = inArrays[iResponseIndex][iResponseMidpoint+i];
          pdResponse[iLength-iResponseMidpoint+i] = inArrays[iResponseIndex][i];
        }

        //
        // handle the case where the response function has an odd number of points...
        //
        if( iResponseMidpoint % 2 == 1 )
        {
          pdResponse[iResponseMidpoint]           = inArrays[iResponseIndex][inArrayLens[iResponseIndex]];
        }

        //
        // zero-pad the convolve array...
        //
        memset( pdConvolve, 0, iLength * sizeof( double ) );
        memcpy( pdConvolve, inArrays[iConvolveIndex], inArrayLens[iConvolveIndex] * sizeof( double ) );

        //
        // calculate the FFTs of the two functions...
        //
        if( gsl_fft_real_radix2_transform( pdResponse, 1, iLength ) == 0 )
        {
          if( gsl_fft_real_radix2_transform( pdConvolve, 1, iLength ) == 0 )
          {
            //
            // multiply the FFTs together...
            //
            for( i=0; i<iLength/2; i++ )
            {
              if( i==0 || i==(iLength/2)-1 )
              {
                pdResponse[i] = pdResponse[i] * pdConvolve[i];
              }
              else
              {              
                dReal = pdResponse[i] * pdConvolve[i] - pdResponse[iLength-i] * pdConvolve[iLength-i];
                dImag = pdResponse[i] * pdConvolve[iLength-i] + pdResponse[iLength-i] * pdConvolve[i];

                pdResponse[i]         = dReal;
                pdResponse[iLength-i] = dImag;
              }
            }

            //
            // do the inverse FFT...
            //
            if( gsl_fft_halfcomplex_radix2_inverse( pdResponse, 1, iLength ) == 0 )
            {              
              if( outArrayLens[0] != inArrayLens[iConvolveIndex] )
              {
                pdResult = (double*)realloc( outArrays[0], inArrayLens[iConvolveIndex] * sizeof( double ) );
              }
              else
              {
                pdResult = outArrays[0];
              }

              if( pdResult != NULL )
              {
                outArrays[0] = pdResult;
                outArrayLens[0] = inArrayLens[iConvolveIndex];

                memcpy( pdResult, pdResponse, inArrayLens[iConvolveIndex] * sizeof( double ) );

                iReturn = 0;
              }
            }
          }
        }
        
        delete[] pdResponse;
        delete[] pdConvolve;
      }
    }
  }
  
  return iReturn;
}
