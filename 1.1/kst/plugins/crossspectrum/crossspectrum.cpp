/***************************************************************************
            crossspectrum.cpp - cross power spectrum plugin for kst
                             -------------------
    begin                : Fri July 30, 2004
    copyright            : (C) 2004 by C. Barth Netterfield
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KSTPSDMAXLEN 27

extern "C" void rdft(int n, int isgn, double *a);

extern "C" int crossspectrum(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[]);

int crossspectrum(const double *const inArrays[], const int inArrayLens[],
		const double inScalars[],
		double *outArrays[], int outArrayLens[],
		double outScalars[])
{
  double SR = inScalars[1]; // sample rate
  double df;
  int i,  xps_len;
  double *a,  *b;
  double mean_a,  mean_b;
  int dv0,  dv1,  v_len;
  int i_subset,  n_subsets;
  int i_samp,  copyLen;
  double norm_factor;

  /* parse fft length */
  xps_len = int( inScalars[0] - 0.99);
  if ( xps_len > KSTPSDMAXLEN ) xps_len = KSTPSDMAXLEN;
  if ( xps_len<2 ) xps_len = 2;
  xps_len = int ( pow( 2,  xps_len ) );

  /* input vector lengths */
  v_len = ( ( inArrayLens[0] < inArrayLens[1] ) ?
            inArrayLens[0] : inArrayLens[1] );
  dv0 = v_len/inArrayLens[0];
  dv1 = v_len/inArrayLens[1];

  while ( xps_len > v_len ) xps_len/=2;

  // allocate the lengths
  if ( outArrayLens[0] != xps_len ) {
    outArrays[0] = (double *) realloc(outArrays[0], xps_len*sizeof(double));
    outArrays[1] = (double *) realloc(outArrays[1], xps_len*sizeof(double));
    outArrays[2] = (double *) realloc(outArrays[2], xps_len*sizeof(double));
    outArrayLens[0] = xps_len;
    outArrayLens[1] = xps_len;
    outArrayLens[2] = xps_len;
  }

  /* Fill the frequency and zero the xps */
  df = SR/( 2.0*double( xps_len-1 ) );
  for ( i=0; i<xps_len; i++ ) {
    outArrays[2][i] = double( i ) * df;
    outArrays[0][i] = 0.0;
    outArrays[1][i] = 0.0;
  }

  /* allocate input arrays */
  int ALen = xps_len * 2;
  a = new double[ALen];
  b = new double[ALen];

  /* do the fft's */
  n_subsets = v_len/xps_len + 1;

  for ( i_subset=0; i_subset<n_subsets; i_subset++ ) {
        /* copy each chunk into a[] and find mean */
    if (i_subset*xps_len + ALen <= v_len) {
      copyLen = ALen;
    } else {
      copyLen = v_len - i_subset*xps_len;
    }
    mean_b = mean_a = 0;
    for (i_samp = 0; i_samp < copyLen; i_samp++) {
      i = ( i_samp + i_subset*xps_len )/dv0;
      mean_a += (
        a[i_samp] = inArrays[0][i]
        );
      i = ( i_samp + i_subset*xps_len )/dv1;
      mean_b += (
        b[i_samp] = inArrays[1][i]
        );
    }
    if (copyLen>1) {
      mean_a/=(double)copyLen;
      mean_b/=(double)copyLen;
    }

    /* Remove Mean and apodize */
    for (i_samp=0; i_samp<copyLen; i_samp++) {
      a[i_samp] -= mean_a;
      b[i_samp] -= mean_b;
    }

    for (;i_samp < ALen; i_samp++) {
      a[i_samp] = 0.0;
      b[i_samp] = 0.0;
    }

    /* fft */
    rdft(ALen, 1, a);
    rdft(ALen, 1, b);

    /* sum each bin into psd[] */
    outArrays[0][0] += ( a[0]*b[0] );
    outArrays[0][xps_len-1] += ( a[1]*b[1] );
    for (i_samp=1; i_samp<xps_len-1; i_samp++) {
      outArrays[0][i_samp]+= ( a[i_samp*2] * b[i_samp*2] -
                                   a[i_samp*2+1] * b[i_samp*2+1] );
      outArrays[1][i_samp]+= ( -a[i_samp*2] * b[i_samp*2+1] +
                                   a[i_samp*2+1] * b[i_samp*2] );
    }// (a+ci)(b+di)* = ab+cd +i(-ad + cb)
  }

  /* renormalize */
  norm_factor = 1.0/((double(SR)*double(xps_len))*double(n_subsets));
  for ( i=0; i<xps_len; i++ ) {
    outArrays[0][i]*=norm_factor;
    outArrays[1][i]*=norm_factor;
  }

  /* free */
  delete[] b;
  delete[] a;
  return 0;
}
