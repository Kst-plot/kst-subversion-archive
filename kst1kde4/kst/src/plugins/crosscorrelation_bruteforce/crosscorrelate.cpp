/***************************************************************************
                   crosscorrelate.cpp
                             -------------------
    begin                : 12/06/08
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

#include "crosscorrelate.h"
#include "kstmath.h"
#include <kgenericfactory.h>

static const QString& STEP_SIZE = KGlobal::staticQString("Inter-step size");
static const QString& SKIP_SIZE = KGlobal::staticQString("Skip size");
static const QString& ARRAY_ONE = KGlobal::staticQString("Array One");
static const QString& ARRAY_TWO = KGlobal::staticQString("Array Two");
static const QString& STEP_VALUE = KGlobal::staticQString("Step value");
static const QString& CORRELATED = KGlobal::staticQString("Correlated");

KST_KEY_DATAOBJECT_PLUGIN( crosscorrelate_bruteforce )

K_EXPORT_COMPONENT_FACTORY( kstobject_crosscorrelate_bruteforce,
    KGenericFactory<CrossCorrelate>( "kstobject_crosscorrelate_bruteforce" ) )

CrossCorrelate::CrossCorrelate( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  _inputScalarDefaults.insert(STEP_SIZE, 10.0);
  _inputScalarDefaults.insert(SKIP_SIZE, 10.0);
}


CrossCorrelate::CrossCorrelate( QObject */*parent*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  _inputScalarDefaults.insert(STEP_SIZE, 10.0);
  _inputScalarDefaults.insert(SKIP_SIZE, 10.0);
}


CrossCorrelate::~CrossCorrelate() {
}


bool CrossCorrelate::algorithm() {
  KstScalarPtr stepSize   = inputScalar(STEP_SIZE);
  KstScalarPtr skipSize   = inputScalar(SKIP_SIZE);
  KstVectorPtr arrayOne   = inputVector(ARRAY_ONE);
  KstVectorPtr arrayTwo   = inputVector(ARRAY_TWO);
  KstVectorPtr stepValue  = outputVector(STEP_VALUE);
  KstVectorPtr correlated = outputVector(CORRELATED);

  if (arrayOne->length() <= 0 || arrayTwo->length() <= 0 ||
      arrayOne->length() != arrayTwo->length()) {
    return false;
  }

  double  dX;
  double  dY;
  int     iStepSize = (int)stepSize->value();
  int     iSkipSize = (int)skipSize->value();
  int     iActualLength;
  int     iHalfLength;
  int     iLength;
  int     iStep;
  int     i;

  if (iStepSize <= 0) {
    iStepSize = 1;
  }

  if (iSkipSize <= 0) {
    iSkipSize = 1;
  }

  iActualLength = arrayOne->length();
  iHalfLength = iActualLength / 2;
  iHalfLength -= iHalfLength % iStepSize;
  iLength = ( ( iHalfLength /iStepSize ) * 2 ) + 1;

  stepValue->resize(iLength, false);
  correlated->resize(iLength, false);

  for (i=-iHalfLength; i<=iHalfLength; i+=iStepSize) {
    double dSumXSquared = 0.0;
    double dSumYSquared = 0.0;
    double dSumXY = 0.0;
    double dSumX = 0.0;
    double dSumY = 0.0;
    double dMeanXY;
    double dMeanX;
    double dMeanY;
    double dCoefficient;
    int iN = 0;
    int j;

    iStep = (i + iHalfLength)/iStepSize;

    for (j=0; j<iActualLength - abs( i ); j+=iSkipSize) {
      if (i < 0) {
        dX = arrayOne->value()[j + abs( i )];
        dY = arrayTwo->value()[j];
      } else {
        dX = arrayOne->value()[j];
        dY = arrayTwo->value()[j + abs( i )];
      }

      if (dX == dX && dY == dY) {
        dSumX += dX;
        dSumY += dY;
        dSumXY += dX* dY;
        dSumXSquared += dX * dX;
        dSumYSquared += dY * dY;
        iN++;
      }
    }

    if (iN > 0) {
      dMeanX = dSumX / (double)iN;
      dMeanY = dSumY / (double)iN;
      dMeanXY = dSumXY / (double)iN;

      dCoefficient = dMeanXY - ( dMeanX * dMeanY );
      dCoefficient /= sqrt( ( dSumXSquared - ( dSumX * dSumX / (double)iN ) ) / (double)iN );
      dCoefficient /= sqrt( ( dSumYSquared - ( dSumY * dSumY / (double)iN ) ) / (double)iN );

      correlated->value()[iStep] = dCoefficient;
    } else {
      correlated->value()[iStep] = NAN;
    }

    stepValue->value()[iStep] = (double)i;
  }

  return true;
}


QStringList CrossCorrelate::inputVectorList() const {
  return QStringList( ARRAY_ONE ) << ARRAY_TWO;
}


QStringList CrossCorrelate::inputScalarList() const {
  return QStringList( STEP_SIZE ) << SKIP_SIZE;
}


QStringList CrossCorrelate::inputStringList() const {
  return QStringList();
}


QStringList CrossCorrelate::outputVectorList() const {
  return QStringList( STEP_VALUE ) << CORRELATED;
}


QStringList CrossCorrelate::outputScalarList() const {
  return QStringList();
}


QStringList CrossCorrelate::outputStringList() const {
  return QStringList();
}

#include "crosscorrelate.moc"
