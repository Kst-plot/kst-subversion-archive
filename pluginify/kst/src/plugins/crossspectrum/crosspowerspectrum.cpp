/***************************************************************************
                   crosspowerspectrum.cpp
                             -------------------
    begin                : 09/08/06
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
#include "crosspowerspectrum.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY( kst_crossspectrum,
    KGenericFactory<CrossPowerSpectrum>( "kst_crossspectrum" ) )

CrossPowerSpectrum::CrossPowerSpectrum( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstDataObject() {
}

CrossPowerSpectrum::~CrossPowerSpectrum() {
}

KstObject::UpdateType CrossPowerSpectrum::update(int)
{
  return UPDATE;
}

QString CrossPowerSpectrum::propertyString() const
{
  return "";
}

KstDataObjectPtr CrossPowerSpectrum::makeDuplicate(KstDataObjectDataObjectMap&)
{
  return 0;
}

void CrossPowerSpectrum::_showDialog()
{
  KMessageBox::information( 0, "insert testplugin config widget here :)", "testpluginconfig" );
}

#include "crosspowerspectrum.moc"
