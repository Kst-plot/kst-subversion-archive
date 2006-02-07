/***************************************************************************
                                 jsproxies.h
                             -------------------
    begin                : Feb 10 2004
    copyright            : (C) 2004 The University of Toronto
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

#ifndef JSPROXIES_H
#define JSPROXIES_H

#include <qobject.h>

#include <kjsembed/jsfactory.h>

#include <kstdatacollection.h>

using namespace KJSEmbed;

class KstJSDataProxy : public QObject {
  Q_OBJECT
  public:
    KstJSDataProxy(JSFactory *factory, QObject *parent, const char *name);
    virtual ~KstJSDataProxy();

  public slots:
    QStringList scalars();
    QObject *scalar(const QString& name);
    QStringList vectors();
    QObject *vector(const QString& name);
    QStringList dataObjects();
    QObject *dataObject(const QString& name);
};


#endif

// vim: ts=2 sw=2 et
