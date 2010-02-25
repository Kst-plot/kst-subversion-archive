/***************************************************************************
                   reverse.cpp
                             -------------------
    begin                : 01/13/10
    copyright            : (C) 2010 The University of British Columbia
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
#include "reverse.h"

#include <kgenericfactory.h>

//in
static const QString& INPUT = KGlobal::staticQString("Input Vector");

//out
static const QString& OUTPUT = KGlobal::staticQString("Output Vector");

KST_KEY_DATAOBJECT_PLUGIN( reverse )

K_EXPORT_COMPONENT_FACTORY( kstobject_reverse,
    KGenericFactory<Reverse>( "kstobject_reverse" ) )

Reverse::Reverse( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
}

Reverse::Reverse( QObject */*parent*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
}

Reverse::~Reverse() {
}

//swap input vector
bool Reverse::algorithm() {

  KstVectorPtr input = inputVector(INPUT);
  KstVectorPtr output  = outputVector(OUTPUT);
  output->resize(input->length());
  int length = input->length();

  for (int i = 0; i < length; i++){
    output->value()[length-i-1] = input->value()[i];
  }

  return true;
}



QStringList Reverse::inputVectorList() const {
  return QStringList( INPUT );
}


QStringList Reverse::inputScalarList() const {
  return QStringList();
}


QStringList Reverse::inputStringList() const {
  return QStringList();
}


QStringList Reverse::outputVectorList() const {
  return QStringList( OUTPUT );
}


QStringList Reverse::outputScalarList() const {
  return QStringList();
}


QStringList Reverse::outputStringList() const {
  return QStringList();
}

#include "reverse.moc"
