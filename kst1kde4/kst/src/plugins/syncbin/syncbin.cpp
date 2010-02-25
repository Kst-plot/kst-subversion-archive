/***************************************************************************
                             syncbin.cpp
                             -------------------
    begin                : 12/07/06
    copyright            : (C) 2006 The University of Toronto
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
#include "syncbin.h"

#include <kgenericfactory.h>

// macros to find the top, bottom, and middle of a bin
#define BINMID(x) XMin+(XMax-XMin)*(double(x)+0.5)/double(nbins)
#define BIN( x ) int(double(nbins)*(x-XMin)/(XMax-XMin))

const double NOPOINT = 0.0/0.0; // NaN

static const QString& X_IN = KGlobal::staticQString("X in");
static const QString& Y_IN = KGlobal::staticQString("Y in");
static const QString& N_BINS = KGlobal::staticQString("Number of Bins");
static const QString& X_MIN = KGlobal::staticQString("X min");
static const QString& X_MAX = KGlobal::staticQString("X max");
static const QString& X_OUT = KGlobal::staticQString("X out");
static const QString& Y_OUT = KGlobal::staticQString("Y out");
static const QString& Y_ERROR = KGlobal::staticQString("Y error");
static const QString& N = KGlobal::staticQString("N");

KST_KEY_DATAOBJECT_PLUGIN( syncbin )

K_EXPORT_COMPONENT_FACTORY( kstobject_syncbin,
    KGenericFactory<Syncbin>( "kstobject_syncbin" ) )

Syncbin::Syncbin( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  //
  // set a reasonable number of bins as the default - 
  //  any value is better than zero...
  //
  _inputScalarDefaults.insert(N_BINS, 5000.0);
}

Syncbin::Syncbin( QObject */*parent*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  //
  // set a reasonable number of bins as the default - 
  //  any value is better than zero...
  //
  _inputScalarDefaults.insert(N_BINS, 5000.0);
}

Syncbin::~Syncbin() {
}


bool Syncbin::algorithm() {
  KstVectorPtr xIn     = inputVector(X_IN);
  KstVectorPtr yIn     = inputVector(Y_IN);
  KstScalarPtr bins    = inputScalar(N_BINS);
  KstScalarPtr xMin    = inputScalar(X_MIN);
  KstScalarPtr xMax    = inputScalar(X_MAX);
  KstVectorPtr xOut    = outputVector(X_OUT);
  KstVectorPtr yOut    = outputVector(Y_OUT);
  KstVectorPtr yError  = outputVector(Y_ERROR);
  KstVectorPtr n       = outputVector(N);

  double XMin = xMin->value();
  double XMax = xMax->value();
  double *Xout, *Yout, *Yerr, *N;
  int nbins = int(bins->value());
  int bin;
  int nIn;
  int i;

  // check for ill defined inputs...
  if (xIn->length() < 1 ||
      yIn->length() != xIn->length() ||
      nbins < 2)  {
    return -1;
  }

  //resize the output arrays
  xOut->resize(nbins, true);
  yOut->resize(nbins, true);
  yError->resize(nbins, true);
  n->resize(nbins, true);

  // convenience definitions
  const double *Xin = xIn->value();
  const double *Yin = yIn->value();
  nIn = int(xIn->length());
  Xout = xOut->value();
  Yout = yOut->value();
  Yerr = yError->value();
  N    = n->value();

  //
  // determine the automatic limits of the bins if necessary...
  //
  if (XMax <= XMin) {
    XMax = XMin = Xin[0];
    for (i=1; i<nIn; i++ ) {
      if (XMax > Xin[i]) {
        XMax = Xin[i];
      }
      if (XMin < Xin[i]) {
        XMin = Xin[i];
      }
    }

    //
    // make sure end points are included
    //
    double d = (XMax - XMin)/double(nbins*100.0);
    XMax += d;
    XMin -= d;
  }

  // don't want divide by zero...
  if (XMax == XMin) {
    XMax += 1.0;
    XMin -= 1.0;
  }

  //
  // initialize the output arrays
  //
  for (i=0; i<nbins; i++) {
    Xout[i] = BINMID( i );
    Yout[i] = Yerr[i] = 0.0;
    N[i] = 0.0;
  }

  //
  // bin the data, using Yout to store the sum of the values in a bin and
  //  using Yerr to store the sum of the squares of the values in a bin...
  //
  for (i=0; i<nIn; i++) {
    bin = BIN(Xin[i]);
    if (bin >= 0 && bin < nbins) {
      Yout[bin] += Yin[i];
      Yerr[bin] += Yin[i]*Yin[i];
      N[bin]++;
    }
  }

  //
  // now we determine the mean of each bin and the bias-corrected variance
  // (aka the standard deviation) of each bin...
  //
  for (i = 0; i<nbins; i++) {
    if (N[i] > 1.0) {
      //
      // we have more than one point in the bin so was can calculate the
      //  both the mean value and the bias-corrected variance...
      //
      Yout[i] /= N[i];
      Yerr[i] = sqrt((Yerr[i] - Yout[i]*Yout[i]*N[i]) / (N[i] - 1.0));
    } else if (N[i] == 1.0) {
      //
      // if we have only one data point in a bin then we have no information on the error
      //  and the value itself is also the mean, i.e. Yout[i] / N[i] = Yout[i] as N[i] == 1...
      //
      Yerr[i] = NOPOINT;
    } else {
      //
      // if we have no data points in a bin then set the y-value and associated error as NaN...
      //
      Yout[i] = NOPOINT;
      Yerr[i] = NOPOINT;
    }
  }

  return true;
}


QStringList Syncbin::inputVectorList() const {
  return QStringList( X_IN ) << Y_IN;
}


QStringList Syncbin::inputScalarList() const {
  return QStringList( N_BINS ) << X_MIN << X_MAX;
}


QStringList Syncbin::inputStringList() const {
  return QStringList();
}


QStringList Syncbin::outputVectorList() const {
  return QStringList( X_OUT ) << Y_OUT << Y_ERROR << N;
}


QStringList Syncbin::outputScalarList() const {
  return QStringList();
}


QStringList Syncbin::outputStringList() const {
  return QStringList();
}

#include "syncbin.moc"
