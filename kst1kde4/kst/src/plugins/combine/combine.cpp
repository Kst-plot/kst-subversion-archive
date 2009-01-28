/***************************************************************************
                          combine.cpp
                             -------------------
    begin                : 05/13/08
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
#include "combine.h"

#include <kgenericfactory.h>

static const QString& COMBINED_ARRAY = KGlobal::staticQString("Combined Array");
static const QString& FIRST_ARRAY = KGlobal::staticQString("First Array");
static const QString& SECOND_ARRAY = KGlobal::staticQString("Second Array");

KST_KEY_DATAOBJECT_PLUGIN( combine )

K_EXPORT_COMPONENT_FACTORY( kstobject_combine,
    KGenericFactory<Combine>( "kstobject_combine" ) )

Combine::Combine( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
}


Combine::~Combine() {
}


bool Combine::algorithm() {
  KstVectorPtr combined  = outputVector(COMBINED_ARRAY);
  KstVectorPtr first  = inputVector(FIRST_ARRAY);
  KstVectorPtr second = inputVector(SECOND_ARRAY);

  int lengthFirst = first->length();
  int lengthSecond = second->length();
  int index = 0;

  combined->resize(lengthFirst + lengthSecond, false);

  for (int i = 0; i < lengthFirst; i++, index++) {
    combined->value()[index] = first->value()[i];
  }
  for (int i = 0; i < lengthSecond; i++, index++) {
    combined->value()[index] = second->value()[i];
  }

  return true;
}


QStringList Combine::inputVectorList() const {
  return QStringList( FIRST_ARRAY ) << SECOND_ARRAY;
}


QStringList Combine::inputScalarList() const {
  return QStringList();
}


QStringList Combine::inputStringList() const {
  return QStringList();
}


QStringList Combine::outputVectorList() const {
  return QStringList( COMBINED_ARRAY );
}


QStringList Combine::outputScalarList() const {
  return QStringList();
}


QStringList Combine::outputStringList() const {
  return QStringList();
}

#include "combine.moc"
