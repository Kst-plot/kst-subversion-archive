/***************************************************************************
          effective_bandwidth.cpp  -  Calculate Effective Bandwidth
                             -------------------
    begin                : Jun 6, 2006
    copyright            : (C) 2006 by Duncan Hanson
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>

extern "C" int effective_bandwidth(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int effective_bandwidth(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
  if ((inArrayLens[1] != inArrayLens[0] ) || (inArrayLens[0] < 1))  {
    return -2;
  }

  double minWhiteNoiseFreq, samplingFrequency, radiometerConstantK;

  minWhiteNoiseFreq = inScalars[0];
  samplingFrequency = inScalars[1];
  radiometerConstantK = inScalars[2];

  int minWhiteNoiseIndex;

  //fast calculation of index for minWhiteNoiseFreq
  int i_bot, i_top;
  i_bot = 0;
  i_top = inArrayLens[0] - 1;

  while (i_bot + 1 < i_top) {
    int i0 = (i_top + i_bot)/2;
    if (minWhiteNoiseFreq < inArrays[0][i0]) {
      i_top = i0;
    } else {
      i_bot = i0;
    }
  }
  minWhiteNoiseIndex = i_top;

  //verify calculated indices.
  if ( !(minWhiteNoiseIndex>0) || !(minWhiteNoiseIndex<(inArrayLens[0]-1)) ) {
    return -2;
  }

  // calculate white noise limit
  double sumY, sumY2;
  sumY = sumY2 = 0;

  int i;
  double yi;

  for (i = minWhiteNoiseIndex; i < inArrayLens[0]; i++) {
    yi = inArrays[1][i];
    sumY    +=  yi;
    sumY2   +=  pow(yi,2);
  }

  double ybar, ysigma;
  ybar = sumY/(inArrayLens[0] - minWhiteNoiseIndex);
  ysigma = sqrt((sumY2 - 2*ybar*sumY + pow(ybar,2)*(inArrayLens[0] - minWhiteNoiseIndex))/(inArrayLens[0] - minWhiteNoiseIndex));
  // end calculate white noise limit

  double effectiveBandwidth = 2*samplingFrequency*pow(radiometerConstantK*inArrays[1][0]/ysigma,2);

  // output fit data
  outScalars[0] = ybar;
  outScalars[1] = ysigma;
  outScalars[2] = effectiveBandwidth;

  return 0;
}
