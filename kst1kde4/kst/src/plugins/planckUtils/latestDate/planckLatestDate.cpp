/***************************************************************************
                   planckLatestDate.cpp
                             -------------------
    begin                : Sept 24, 2009
    copyright            : (C) 2009 The University of British Columbia
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

#include <qdir.h>
#include <qstring.h>
#include <qstringlist.h>

#include "planckLatestDate.h"

#include <kgenericfactory.h>

static const QString& TEMPLATE = KGlobal::staticQString("File template");
static const QString& LATEST_DATE = KGlobal::staticQString("File of latest date");

KST_KEY_DATAOBJECT_PLUGIN( planckLatestDate )

K_EXPORT_COMPONENT_FACTORY( kstobject_planckLatestDate,
    KGenericFactory<PlanckLatestDate>( "kstobject_planckLatestDate" ) )

PlanckLatestDate::PlanckLatestDate( QObject */*parent*/, const char */*name*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
}


PlanckLatestDate::PlanckLatestDate( QObject */*parent*/, const QStringList &/*args*/ )
    : KstBasicPlugin() {
}


PlanckLatestDate::~PlanckLatestDate() {
}


bool PlanckLatestDate::algorithm() {
  KstStringPtr kstrTemplate = inputString(TEMPLATE);
  KstStringPtr kstrName = outputString(LATEST_DATE);
  QString strTemplate = kstrTemplate->value();
  QStringList files;
  QString strFile;
  QString strDir;
  int iRetVal = false;
  int iIndex;

  kstrName->setValue(QString());

  iIndex = strTemplate.lastIndexOf( QDir::separator() );
  if (iIndex >= 0) {
    strDir = strTemplate.left( iIndex + 1 );
    if (strTemplate.length() > 0) {
      strFile = strTemplate.right( strTemplate.length() - iIndex - 1 );
    }
  } else {
    strFile = strTemplate;
  }

  {
    QDir dir(strDir, strFile, QDir::Name, QDir::Files );

    files = dir.entryList( );

    if (!files.empty()) {
      kstrName->setValue( dir.absoluteFilePath( files.back() ) );
    }

    iRetVal = true;
  }

  return iRetVal;
}


QStringList PlanckLatestDate::inputVectorList() const {
  return QStringList( );
}


QStringList PlanckLatestDate::inputScalarList() const {
  return QStringList( );
}


QStringList PlanckLatestDate::inputStringList() const {
  return QStringList( TEMPLATE );
}


QStringList PlanckLatestDate::outputVectorList() const {
  return QStringList( );
}


QStringList PlanckLatestDate::outputScalarList() const {
  return QStringList( );
}


QStringList PlanckLatestDate::outputStringList() const {
  return QStringList( LATEST_DATE );
}

#include "planckLatestDate.moc"
