/***************************************************************************
                              lfidifference.cpp
                             -------------------
    begin                : March 6 2008
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

/***************************************************************************
 LFI reference loads are at a different temperature then the sky signal, 
 therefore in order to reduce the 1/f noise it is necessary to include 
 in the difference a scale factor r (sky - r * ref).

 The default LFI differencing procedure is to take the mean of 1 hour of 
 sky and ref, compute the r factor as mean_sky/mean_ref, and then produce 
 the differenced timestream by computing sky - r * ref.

 Inputs:
   sky - datavector
   ref - datavector (of the same length and sampling rate)
   sampling_freq - the same for sky and ref
   timespan (hours) - where to compute the r factor, default is 1, 
           if empty the complete vectors are taken into account

 Outputs:
   r (vector containing computed r for each timespan, dimension is
        the number of timespans in the sky and ref datavectors)
   diff (datavector of the same dimensions of sky and ref) 
 ***************************************************************************/

#include "lfidifference.h"
#include <kgenericfactory.h>

static const QString& SKY_IN = KGlobal::staticQString("Sky");
static const QString& REF_IN = KGlobal::staticQString("Reference load");
static const QString& FREQ = KGlobal::staticQString("Sampling frequency (Hz)");
static const QString& SPAN = KGlobal::staticQString("Timespan (hours)");
static const QString& R_OUT = KGlobal::staticQString("R out");
static const QString& R_OUT_INDEX = KGlobal::staticQString("R out index");
static const QString& DIFF_OUT = KGlobal::staticQString("Difference out");

KST_KEY_DATAOBJECT_PLUGIN( lfidifference )

K_EXPORT_COMPONENT_FACTORY( kstobject_lfidifference,
    KGenericFactory<LFIDifference>( "kstobject_lfidifference" ) )

LFIDifference::LFIDifference( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  _inputScalarDefaults.insert(FREQ, 1.0);
  _inputScalarDefaults.insert(SPAN, 1.0);
}

LFIDifference::LFIDifference( QObject */*parent*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  _inputScalarDefaults.insert(FREQ, 1.0);
  _inputScalarDefaults.insert(SPAN, 1.0);
}

LFIDifference::~LFIDifference() {
}


bool LFIDifference::algorithm() {
  KstVectorPtr skyIn      = inputVector(SKY_IN);
  KstVectorPtr refIn      = inputVector(REF_IN);
  KstScalarPtr freqHz     = inputScalar(FREQ);
  KstScalarPtr spanHours  = inputScalar(SPAN);
  KstVectorPtr rOut       = outputVector(R_OUT);
  KstVectorPtr rOutIndex  = outputVector(R_OUT_INDEX);
  KstVectorPtr diffOut    = outputVector(DIFF_OUT);

  if (skyIn->length() != refIn->length() ||
      skyIn->length() < 2) {
    return -1;
  }

  const double *dSkyIn = skyIn->value();
  const double *dRefIn = refIn->value();
  double *dROut;
  double *dROutIndex;
  double *dDiffOut;
  unsigned numInSpan;
  unsigned numSpans;

  numInSpan = (unsigned)floor((spanHours->value() * 60.0 * 60.0) * freqHz->value());
  if (numInSpan < 1) {
    numInSpan = 1;
  }
  numSpans = skyIn->length() / numInSpan;

  //
  // if the remainder time span has more than half the samples
  //  per full time span then treat it on its own, else it is
  //  treated in the preceeding full time span
  //
  if (skyIn->length() - (numSpans * numInSpan) > numInSpan / 2) {
    numSpans++;
  }

  //
  // resize the output vectors
  //
  rOut->resize(numSpans, true);
  rOutIndex->resize(numSpans, true);
  diffOut->resize(skyIn->length(), true);

  dROut = rOut->value();
  dROutIndex = rOutIndex->value();
  dDiffOut = diffOut->value();

  for (unsigned span=0; span<numSpans; ++span) {
    double sumSky = 0.0;
    double sumRef = 0.0;
    unsigned spanStart = numInSpan * span;
    unsigned spanEnd = numInSpan * (span + 1);

    //
    // if this is the last span ensure we capture all the data points
    //  and ensure we don't fall off the end of the array
    //
    if (span == numSpans-1) {
      spanEnd = (unsigned)skyIn->length();
    } else if (spanEnd > (unsigned)skyIn->length()) {
      spanEnd = (unsigned)skyIn->length();
    }

    for (unsigned i=spanStart; i<spanEnd; ++i) {
      sumSky += dSkyIn[i];
      sumRef += dRefIn[i];
    }

    if (sumRef != 0.0) {
      dROut[span] = sumSky / sumRef;
    } else {
      dROut[span] = 1.0;
    }
    dROutIndex[span] = (double)span;

    for (unsigned i=spanStart; i<spanEnd; ++i) {
      dDiffOut[i] = dSkyIn[i] - dROut[span] * dRefIn[i];
    }
  }

  return true;
}


QStringList LFIDifference::inputVectorList() const {
  return QStringList( SKY_IN ) << REF_IN;
}


QStringList LFIDifference::inputScalarList() const {
  return QStringList( FREQ ) << SPAN;
}


QStringList LFIDifference::inputStringList() const {
  return QStringList();
}


QStringList LFIDifference::outputVectorList() const {
  return QStringList( R_OUT ) << R_OUT_INDEX << DIFF_OUT;
}


QStringList LFIDifference::outputScalarList() const {
  return QStringList();
}


QStringList LFIDifference::outputStringList() const {
  return QStringList();
}

#include "lfidifference.moc"
