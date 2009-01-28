/***************************************************************************
                          normalization.cpp
                             -------------------
    begin                : 08/15/08
    copyright            : (C) 2008 The University of British Columbia
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

#include "normalization.h"
#include <stdio.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_spline.h>
#include <math.h>
#include <kgenericfactory.h>

#ifdef NAN
const double NOPOINT = NAN;
#else
const double NOPOINT = 0.0/0.0; // NaN
#endif

static const QString& VECTOR_IN = KGlobal::staticQString("Vector In");
static const QString& VECTOR_OUT = KGlobal::staticQString("Vector Out");

KST_KEY_DATAOBJECT_PLUGIN( normalization )

K_EXPORT_COMPONENT_FACTORY(kstobject_normalization,
         KGenericFactory<Normalization>("kstobject_normalizaion"))

Normalization::Normalization( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
}


Normalization::~Normalization() {
}


bool Normalization::algorithm() {
  KstVectorPtr vectorIn = inputVector(VECTOR_IN);
  KstVectorPtr vectorOut = outputVector(VECTOR_OUT);
  double *arr;
  double *Yi;
  int iLength = vectorIn->length();
  int w = 1;

  arr = new double[iLength];
  Yi = new double[iLength];

  for(int i=0; i<iLength; i++)
  {
    Yi[i] = vectorIn->value()[i];
  }

  //
  // exclude peak values
  //
  for(int loop=0; loop<2; loop++)
  {
    for(int i=0; i<iLength; i++)
    {
      arr[i] = Yi[i];
    }

    for(int i=0; i<iLength; i++)
    {
      if(isMin(Yi, i, iLength) || isMax(Yi, i, iLength))
      {
        excludePts(arr, i, w, iLength);
      }
    }

    searchHighPts(arr, iLength);
    interpolate(Yi, arr, iLength);
  }

  //
  // do a piecewise linear fit
  //
  vectorOut->resize(iLength, false);

  int L = 3;
  double cof[2] = { 0.0, 0.0 };

  for(int i=0; i<iLength; i=i+L)
  {
    fit(i, L, iLength, Yi, cof, vectorOut);
  }

  //
  // normalize
  //
  for(int i=0; i<iLength; i++)
  {
    vectorOut->value()[i] = vectorIn->value()[i] / vectorOut->value()[i];
  }

  //
  // exclude off points
  //
  for(int i=0; i<iLength; i++)
  {
    if(vectorOut->value()[i] < 0.0 || vectorOut->value()[i] > 1.2)
    {
      vectorOut->value()[i] = NOPOINT;
    }
  }

  delete[] arr;
  delete[] Yi;

  return true;
}


void Normalization::fit(int k, int p, int iLength, double arr[], double cof[], KstVectorPtr vector_out)
{
  if(k+p < iLength)
  {
    double v1[p];
    double v2[p];
    int j=0;

    for(int i=k; i<k+p; i++)
    {
      v1[j] = (double)i;
      v2[j] = arr[i];
      j++;
    }

    double c0, c1, cov00, cov01, cov11, chisq;

    gsl_fit_linear(v1, 1, v2, 1, p, &c0, &c1, &cov00, &cov01, &cov11, &chisq);
    cof[0] = c0;
    cof[1] = c1;
    for(int i=k; i<k+p; i++)
    {
      vector_out->value()[i] = cof[0]+cof[1]*i;
    }
  }
  else
  {
    for(int i=k; i<iLength; i++)
    {
      vector_out->value()[i] = cof[0]+cof[1]*i;
    }
  }
}


bool Normalization::isMax(double arr[], int pos, int iLength)
{
  bool result = false;
  double l = NOPOINT;
  double r = NOPOINT;

  if(!isnan(arr[pos]))
  {
    int p = pos-1;

    while(p >= 0)
    {
      if(!isnan(arr[p]))
      {
        l = arr[p];
        break;
      }
      p--;
    }

    int n = pos+1;

    while(n <= iLength-1)
    {
      if(!isnan(arr[n]))
      {
        r = arr[n];
        break;
      }
      n++;
    }
  }

  if(!isnan(r) && !isnan(l))
  {
    result = arr[pos]-l > 0.0 && arr[pos]-r > 0.0;
  }

  return result;
}


bool Normalization::isMin(double arr[], int pos, int iLength)
{
  bool result = false;
  double l = NOPOINT;
  double r = NOPOINT;

  if(!isnan(arr[pos]))
  {
    int p = pos-1;

    while(p >= 0)
    {
      if(!isnan(arr[p]))
      {
        l = arr[p];
        break;
      }
      p--;
    }

    int n = pos+1;

    while(n <= iLength-1)
    {
      if(!isnan(arr[n]))
      {
        r = arr[n];
        break;
      }
      n++;
    }
  }

  if(!isnan(r) && !isnan(l))
  {
    result = arr[pos]-l < 0.0 && arr[pos]-r < 0.0;
  }
  return result;
}


void Normalization::excludePts(double arr[], int i, int w, int iLength)
{
  if(i-w >= 0 && i+w <= iLength-1)
  {
    for(int j=i-w; j<i+w; j++)
    {
      arr[j] = NOPOINT;
    }
  }
}


void Normalization::searchHighPts(double arr[], int iLength)
{
  double *y;
  int *x;
  int numPts = 0;

  x = new int[iLength];
  y = new double[iLength];

  for(int i=0; i<iLength; i++)
  {
    if(!isnan(arr[i]))
    {
      x[numPts] = i;
      y[numPts] = arr[i];
      numPts++;
    }
  }

  for(int i=0; i<numPts; i++)
  {
    if(y[i]-y[i-1] >= 0.0 && y[i]-y[i+1] >= 0.0)
    {
      arr[x[i]] = y[i];
    }
    else
    {
      arr[x[i]] = NOPOINT;
    }
  }

  delete[] x;
  delete[] y;
}


void Normalization::interpolate(double Yi[], double arr[], int iLength)
{
  // interpolate remaining pts
  int N = 0; //# remaining pts

  for(int i=0; i<iLength; i++)
  {
    if(!isnan(arr[i]))
    {
      N++;
    }
  }

  double *x;
  double *y;
  int j = 0;

  x = new double[N];
  y = new double[N];

  for(int i=0; i<iLength; i++)
  {
    if(!isnan(arr[i]))
    {
      x[j] = i;
      y[j] = arr[i];
      j++;
    }
  }

  gsl_interp_accel *acc = gsl_interp_accel_alloc();
  gsl_spline *spline = gsl_spline_alloc(gsl_interp_akima, N);

  gsl_spline_init(spline, x, y, N);

  for(int i=0; i<iLength; i++)
  {
    Yi[i] = gsl_spline_eval (spline, i, acc);
  }

  gsl_spline_free(spline);
  gsl_interp_accel_free(acc);

  delete[] x;
  delete[] y;
}


QStringList Normalization::inputVectorList() const {
  return QStringList( VECTOR_IN );
}


QStringList Normalization::inputScalarList() const {
  return QStringList();
}


QStringList Normalization::inputStringList() const {
  return QStringList();
}


QStringList Normalization::outputVectorList() const {
  return QStringList( VECTOR_OUT);
}


QStringList Normalization::outputScalarList() const {
  return QStringList();
}


QStringList Normalization::outputStringList() const {
  return QStringList();
}

#include "normalization.moc"
