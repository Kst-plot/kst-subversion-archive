/***************************************************************************
                          converttime.cpp
                             -------------------
    begin                : July 2nd 2008
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
#include "converttime.h"

#include <kgenericfactory.h>
#include <kstplotdefines.h>

static const QString& INPUT_VECTOR = KGlobal::staticQString("Input vector");
static const QString& INPUT_FORMAT = KGlobal::staticQString("Input time format");
static const QString& OUTPUT_FORMAT = KGlobal::staticQString("Output time format");
static const QString& OUTPUT_VECTOR = KGlobal::staticQString("Output vector");

KST_KEY_DATAOBJECT_PLUGIN( converttime )

K_EXPORT_COMPONENT_FACTORY( kstobject_converttime,
    KGenericFactory<ConvertTime>( "kstobject_converttime" ) )

ConvertTime::ConvertTime( QObject *parent, const char *name, const QStringList &args )
    : KstBasicPlugin() {
  Q_UNUSED(parent)
  Q_UNUSED(name)
  Q_UNUSED(args)
}


ConvertTime::~ConvertTime() {
}


bool ConvertTime::algorithm() {
  KstVectorPtr vectorInput = inputVector(INPUT_VECTOR);
  KstScalarPtr scalarInputFormat = inputScalar(INPUT_FORMAT);
  KstScalarPtr scalarOutputFormat = inputScalar(OUTPUT_FORMAT);
  KstVectorPtr vectorOutput = outputVector(OUTPUT_VECTOR);
  KstAxisInterpretation formatInput;
  KstAxisInterpretation formatOutput;
  double value;

  formatInput = (KstAxisInterpretation)floor( scalarInputFormat->value( ) );
  formatOutput = (KstAxisInterpretation)floor( scalarOutputFormat->value( ) );

  if ((unsigned int)formatInput >= numAxisInterpretations) {
    return false;
  }

  if ((unsigned int)formatOutput >= numAxisInterpretations) {
    return false;
  }

  if (formatInput == formatOutput) {
    return false;
  }

  if (vectorOutput->length() != vectorInput->length()) {
    vectorOutput->resize(vectorInput->length(), false);
  }

  for (int i = 0; i < vectorInput->length(); i++) {
    value = vectorInput->value()[i];

    //
    // convert time to intermediate JD1970...
    //
    switch( formatInput ) {
      case AXIS_INTERP_YEAR:
        value -= 1900.0;
        value *= 365.25;
        value += JD1900 + 0.5;
        break;
      case AXIS_INTERP_CTIME:
        value /= 24.0 * 60.0 * 60.0;
        value += JD1970;
        break;
      case AXIS_INTERP_JD:
        break;
      case AXIS_INTERP_MJD:
        value += JD_MJD;
        break;
      case AXIS_INTERP_RJD:
        value += JD_RJD;
        break;
      case AXIS_INTERP_AIT:
        value -= 86400.0 * (365.0 * 12.0 + 3.0);
        // current difference (seconds) between UTC and AIT
        // refer to the following for more information:
        // http://hpiers.obspm.fr/eop-pc/earthor/utc/TAI-UTC_tab.html
        value -= 32.0;
        value /= 24.0 * 60.0 * 60.0;
        value += JD1970;
        break;
      case AXIS_INTERP_AIT_NANO:
        value /= 1000000000.0;
        value -= 86400.0 * (365.0 * 12.0 + 3.0);
        // current difference (seconds) between UTC and AIT
        // refer to the following for more information:
        // http://hpiers.obspm.fr/eop-pc/earthor/utc/TAI-UTC_tab.html
        value /= 24.0 * 60.0 * 60.0;
        value += JD1970;
        break;
      case AXIS_INTERP_AIT_PLANCK_IDEF:
        value /= 65536.0; // 2^16
        value -= 86400.0 * (365.0 * 12.0 + 3.0);
        // current difference (seconds) between UTC and AIT
        // refer to the following for more information:
        // http://hpiers.obspm.fr/eop-pc/earthor/utc/TAI-UTC_tab.html
        value /= 24.0 * 60.0 * 60.0;
        value += JD1970;
        break;
      default:
        break;
    }

    //
    // convert time to output format...
    //
    switch( formatOutput ) {
      case AXIS_INTERP_YEAR:
        value -= JD1900 + 0.5;
        value /= 365.25;
        value += 1900.0;
        break;
      case AXIS_INTERP_CTIME:
        value -= JD1970;
        value *= 24.0 * 60.0 * 60.0;
        break;
      case AXIS_INTERP_JD:
        break;
      case AXIS_INTERP_MJD:
        value -= JD_MJD;
        break;
      case AXIS_INTERP_RJD:
        value -= JD_RJD;
        break;
      case AXIS_INTERP_AIT:
        value -= JD1970;
        value *= 24.0 * 60.0 * 60.0;
        // current difference (seconds) between UTC and AIT
        // refer to the following for more information:
        // http://hpiers.obspm.fr/eop-pc/earthor/utc/TAI-UTC_tab.html
        value += 32.0;
        value += 86400.0 * (365.0 * 12.0 + 3.0);
        break;
      case AXIS_INTERP_AIT_NANO:
        value -= JD1970;
        value *= 24.0 * 60.0 * 60.0;
        // current difference (seconds) between UTC and AIT
        // refer to the following for more information:
        // http://hpiers.obspm.fr/eop-pc/earthor/utc/TAI-UTC_tab.html
        value += 86400.0 * (365.0 * 12.0 + 3.0);
        value *= 1000000000.0;
        break;
      case AXIS_INTERP_AIT_PLANCK_IDEF:
        value -= JD1970;
        value *= 24.0 * 60.0 * 60.0;
        // current difference (seconds) between UTC and AIT
        // refer to the following for more information:
        // http://hpiers.obspm.fr/eop-pc/earthor/utc/TAI-UTC_tab.html
        value += 86400.0 * (365.0 * 12.0 + 3.0);
        value *= 65536.0; // 2^16
        break;
      default:
        break;
    }

    vectorOutput->value()[i] = value;
  }

  return true;
}


QStringList ConvertTime::inputVectorList() const {
  return QStringList( INPUT_VECTOR );
}


QStringList ConvertTime::inputScalarList() const {
  return QStringList( INPUT_FORMAT ) << OUTPUT_FORMAT;
}


QStringList ConvertTime::inputStringList() const {
  return QStringList();
}


QStringList ConvertTime::outputVectorList() const {
  return QStringList( OUTPUT_VECTOR );
}


QStringList ConvertTime::outputScalarList() const {
  return QStringList();
}


QStringList ConvertTime::outputStringList() const {
  return QStringList();
}

#include "converttime.moc"
