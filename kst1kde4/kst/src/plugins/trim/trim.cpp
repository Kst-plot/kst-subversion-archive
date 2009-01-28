/***************************************************************************
                          trim.cpp
                             -------------------
    begin                : 06/16/08
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
#include "trim.h"

#include <kgenericfactory.h>

static const QString& INPUT = KGlobal::staticQString("Input Vector");
static const QString& REMOVE = KGlobal::staticQString("Remove");
static const QString& CUT = KGlobal::staticQString("Cut");

KST_KEY_DATAOBJECT_PLUGIN( trim )

K_EXPORT_COMPONENT_FACTORY( kstobject_trim,
    KGenericFactory<Trim>( "kstobject_trim" ) )

Trim::Trim( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
  _inputScalarDefaults.insert(REMOVE, 1);
}


Trim::~Trim() {
}


// Remove some elements from the vector starting from vector[0]
bool Trim::algorithm() {
  KstVectorPtr input = inputVector(INPUT);
  KstScalarPtr remove = inputScalar(REMOVE);
  KstVectorPtr cut = outputVector(CUT);
  bool rc = false;

  if (input->length() > remove->value()) {
    int cutSize = (int)input->length() - (int)remove->value();
    cut->resize( cutSize, false );
    for (int j=0; j<cutSize; j++) {
      cut->value()[j] = input->value()[(int)remove->value()+j];
    }

    rc = true;
  }

  return rc;
}


QStringList Trim::inputVectorList() const {
  return QStringList( INPUT );
}


QStringList Trim::inputScalarList() const {
  return QStringList( REMOVE );
}


QStringList Trim::inputStringList() const {
  return QStringList();
}


QStringList Trim::outputVectorList() const {
  return QStringList( CUT );
}


QStringList Trim::outputScalarList() const {
  return QStringList();
}


QStringList Trim::outputStringList() const {
  return QStringList();
}

#include "trim.moc"
