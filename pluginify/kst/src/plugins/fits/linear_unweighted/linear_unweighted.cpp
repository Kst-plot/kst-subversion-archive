/***************************************************************************
                   linear_unweighted.cpp
                             -------------------
    begin                : 11/15/06
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
#include "linear_unweighted.h"

#include <kgenericfactory.h>

static const QString& X_ARRAY = KGlobal::staticQString("X Array");
static const QString& Y_ARRAY = KGlobal::staticQString("Y Array");

static const QString& Y_FITTED = KGlobal::staticQString("Y Fitted");
static const QString& RESIDUALS = KGlobal::staticQString("Residuals");
static const QString& PARAMETERS = KGlobal::staticQString("Parameters");
static const QString& COVARIANCE = KGlobal::staticQString("Covariance");
static const QString& YLO = KGlobal::staticQString("Y Lo");
static const QString& YHI = KGlobal::staticQString("Y Hi");

static const QString& CHI2NU = KGlobal::staticQString("chi^2/nu");

KST_KEY_DATAOBJECT_PLUGIN( linear_unweighted )

K_EXPORT_COMPONENT_FACTORY( kstobject_linear_unweighted,
    KGenericFactory<LinearUnweighted>( "kstobject_linear_unweighted" ) )

LinearUnweighted::LinearUnweighted( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
    _isFit = true;
}


LinearUnweighted::~LinearUnweighted() {
}


bool LinearUnweighted::algorithm() {
  //FIXME Need to move the common functions to use the inputs/outputs directly
  return true;
}


QStringList LinearUnweighted::inputVectorList() const {
  return QStringList( X_ARRAY ) << Y_ARRAY;
}


QStringList LinearUnweighted::inputScalarList() const {
  return QStringList();
}


QStringList LinearUnweighted::inputStringList() const {
  return QStringList();
}


QStringList LinearUnweighted::outputVectorList() const {
  return QStringList( Y_FITTED ) << RESIDUALS << PARAMETERS << COVARIANCE << YLO << YHI;
}


QStringList LinearUnweighted::outputScalarList() const {
  return QStringList( CHI2NU );
}


QStringList LinearUnweighted::outputStringList() const {
  return QStringList();
}


QString LinearUnweighted::parameterName(int index) const {
  switch (index) {
  case 0:
    return "Intercept";
  case 1:
    return "Gradient";
  default:
    return QString::null;
  }
}

#include "linear_unweighted.moc"
