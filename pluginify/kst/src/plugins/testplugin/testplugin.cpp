/***************************************************************************
                   testplugin.cpp
                             -------------------
    begin                : 09/04/06
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
#include "testplugin.h"

#include <kdebug.h>
#include <kgenericfactory.h>

static const QString& VECTOR_IN = KGlobal::staticQString("Vector In");
static const QString& SCALAR_IN = KGlobal::staticQString("Scalar In");
static const QString& STRING_IN = KGlobal::staticQString("String In");
static const QString& VECTOR_OUT = KGlobal::staticQString("Vector Out");
static const QString& SCALAR_OUT = KGlobal::staticQString("Scalar Out");
static const QString& STRING_OUT = KGlobal::staticQString("String Out");

K_EXPORT_COMPONENT_FACTORY( kstobject_testplugin,
    KGenericFactory<TestPlugin>( "kstobject_testplugin" ) )

TestPlugin::TestPlugin( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
}


TestPlugin::~TestPlugin() {
}


bool TestPlugin::algorithm() {
   //Do nothing
  return true;
}


QStringList TestPlugin::inputVectors() const {
  return QStringList( VECTOR_IN );
}


QStringList TestPlugin::inputScalars() const {
  return QStringList( SCALAR_IN );
}


QStringList TestPlugin::inputStrings() const {
  return QStringList( STRING_IN );
}


QStringList TestPlugin::outputVectors() const {
  return QStringList( VECTOR_OUT );
}


QStringList TestPlugin::outputScalars() const {
  return QStringList( SCALAR_OUT );
}


QStringList TestPlugin::outputStrings() const {
  return QStringList( STRING_OUT );
}

#include "testplugin.moc"
