/***************************************************************************
                          syncbin.cpp  -  1D naive map plugin for kst
                             -------------------
    begin                : Jan 9, 2005
    copyright            : (C) 2005 by Barth Netterfield
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

extern "C" int syncbin(const double *const inArrays[], const int inArrayLens[],
                       const double is[],
                       double *outArrays[], int outArrayLens[],
                       double outScalars[]);

// macros to find the top, bottom, and middle of a bin
#define BINMID(x) XMin+(XMax-XMin)*(double(x)+0.5)/double(nbins)

#define BIN( x ) int(double(nbins)*(x-XMin)/(XMax-XMin))

/***************************************************************************/
/*                                                                         */
/*   Synchronously bin vector Y into bins defined by vector X - like a     */
/*   1d binned map.  This is useful for locking in a signal (eg, a         */
/*   bolometer) synchronously with some other signal (eg, azimuth)         */
/*                                                                         */
/***************************************************************************/
int syncbin(const double *const inArrays[], const int inArrayLens[],
            const double is[],
            double *outArrays[], int outArrayLens[],
            double outScalars[]) {

  int nbins=int( is[0] );
  int n_in;
  double XMin = is[1];
  double XMax = is[2];
  double *Xout, *Yout, *Yerr, *N;

  // check for ill defined inputs...
  if ((inArrayLens[0] < 1) ||
      (inArrayLens[1] != inArrayLens[0] ) ||
      (nbins < 2))  {
    return -1;
  }

  //now set the outputs
  outArrayLens[0] = outArrayLens[1] = outArrayLens[2] =
    outArrayLens[3] = nbins;

  //resize the output arrays
  outArrays[0]=(double*)realloc(outArrays[0], outArrayLens[0]*sizeof(double));
  outArrays[1]=(double*)realloc(outArrays[1], outArrayLens[1]*sizeof(double));
  outArrays[2]=(double*)realloc(outArrays[2], outArrayLens[2]*sizeof(double));
  outArrays[3]=(double*)realloc(outArrays[3], outArrayLens[3]*sizeof(double));

  // convenience definitions
  n_in = int( inArrayLens[0] );
  const double *Xin = inArrays[0];
  const double *Yin = inArrays[1];
  Xout = outArrays[0];
  Yout = outArrays[1];
  Yerr = outArrays[2];
  N    = outArrays[3];

  // set/check XMax and XMin
  if ( XMax <= XMin ) { // autobin
    XMax = XMin = Xin[0];
    for (int i=1; i<n_in; i++ ) {
      if ( XMax>Xin[i] ) XMax = Xin[i];
      if ( XMin<Xin[i] ) XMin = Xin[i];
    }
    // make sure end points are included.
    double d = (XMax - XMin)/double(nbins*100.0);
    XMax+=d;
    XMin-=d;
  }

  if ( XMax == XMin ) { // don't want divide by zero...
    XMax +=1;
    XMin -=1;
  }

  // Fill Xout and zero Yout and Yerr
  for ( int i=0; i<nbins; i++ ) {
    Xout[i] = BINMID( i );
    Yout[i] = Yerr[i] = 0.0;
    N[i] = 0.0;
  }

  //bin the data
  int bin, last_bin=-1;
  int last_N=0;
  double last_sY=0;

  for ( int i=0; i<n_in; i++ ) {
    bin = BIN( Xin[i] );
    if (bin == last_bin) {
      last_sY += Yin[i];
      last_N++;
    } else { // new bin
      if (last_N>0) {
	last_sY/=last_N;
	if ( (last_bin>=0) && (last_bin<nbins) ) {
	  Yout[last_bin]+=last_sY;
	  Yerr[last_bin]+=last_sY*last_sY;
	  N[last_bin]++;
	}
      }
      last_sY = Yin[i];
      last_N = 1;
      last_bin = bin;
    }
  }
  if (last_N>0) {
    last_sY/=last_N;
    if ( (last_bin>=0) && (last_bin<nbins) ) {
      Yout[last_bin]+=last_sY;
      Yerr[last_bin]+=last_sY*last_sY;
      N[last_bin]++;
    }
  }

  // normalize the bins
  for ( int i = 0; i<nbins; i++ ) {
    if ( N[i]>0 ) {
      Yerr[i] = sqrt( Yerr[i] - Yout[i]*Yout[i]/N[i] )/N[i];
      Yout[i]/=N[i];
    }
  }

  return 0;
}
