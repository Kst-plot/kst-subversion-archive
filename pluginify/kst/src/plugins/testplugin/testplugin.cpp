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

K_EXPORT_COMPONENT_FACTORY( kstobject_testplugin,
    KGenericFactory<TestPlugin>( "kstobject_testplugin" ) )

TestPlugin::TestPlugin( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstDataObject() {
}

TestPlugin::~TestPlugin() {
}

#include "testplugin.moc"
