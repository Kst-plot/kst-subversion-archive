/***************************************************************************
                          De-spiking filter for kst
                             -------------------
    begin                : Jan 2005
    copyright            : (C) 2005 by cbn
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                        2                                                 *
 ***************************************************************************/


#include <stdlib.h>
#include <math.h>

extern "C" int despike(const double *const inArrays[], const int inArrayLens[],
		       const double inScalars[],
		       double *outArrays[], int outArrayLens[],
		       double outScalars[]);

int despike(const double *const inArrays[], const int inArrayLens[],
	    const double inScalars[],
	    double *outArrays[], int outArrayLens[],
	    double outScalars[]) {
  int N = inArrayLens[0];
  const double *X = inArrays[0];
  double *Y;
  double lastGood;
  double cut = inScalars[1];
  double mdev = 0.0;
  int dx = int(inScalars[0]);
  int i;
  int spikeStart = -1;
  int border = dx*2;

  if (N<1 || cut<=0 || dx<1 || dx>N/2) {
    return -1;
  }

  // get mean deviation of 3 pt difference
  for (i=dx; i<N-dx; i++) {
    mdev += fabs(X[i]-(X[i-dx]+X[i+dx])*0.5);
  }
  mdev /= double(N);

  cut *= mdev;

  // resize the output array
  outArrayLens[0] = N;
  Y = outArrays[0] = (double*)realloc(outArrays[0], N*sizeof(double));

  // do a 2 point difference for first dx points
  lastGood = X[0];
  for (i=0; i<dx; i++) {
    if (fabs(X[i] - X[i+dx]) > cut) {
      if (spikeStart < 0) {
        spikeStart = i-border;
        if (spikeStart < 0) {
          spikeStart = 0;
        }
      }
    } else {
      if (spikeStart >= 0) {
        i += 4*border-1;
        if (i >= N) {
          i=N-1;
        }
        for (int j=spikeStart; j<=i; j++) {
          Y[j] = lastGood;
        }
        spikeStart = -1;
      }
      lastGood = Y[i] = X[i];
    }
  }

  // do a 3 point difference where it is possible
  for (i=dx; i<N-dx; i++) {
    if (fabs(X[i] - (X[i-dx]+X[i+dx])*0.5) > cut) {
      if (spikeStart < 0) {
        spikeStart = i-border;
        if (spikeStart < 0) {
          spikeStart = 0;
        }
      }
    } else {
      if (spikeStart >= 0) {
        i += 4*border-1;
        if (i >= N) {
          i = N-1;
        }
        for (int j=spikeStart; j<=i; j++) {
          Y[j] = lastGood;
        }
        spikeStart = -1;
      } else {
        lastGood = Y[i] = X[i];
      }
    }
  }

  // do a 2 point difference for last dx points
  for (i=N-dx; i<N; i++) {
    if (fabs(X[i-dx] - X[i]) > cut) {
      if (spikeStart < 0) {
        spikeStart = i-border;
        if (spikeStart < 0) {
          spikeStart = 0;
        }
      }
    } else {
      if (spikeStart >= 0) {
        i += 4*border-1;
        if (i >= N) {
          i = N-1;
        }
        for (int j=spikeStart; j<=i; j++) {
          Y[j] = lastGood;
        }
        spikeStart = -1;
      } else {
        lastGood = Y[i] = X[i];
      }
    }
  }

  return 0;
}
