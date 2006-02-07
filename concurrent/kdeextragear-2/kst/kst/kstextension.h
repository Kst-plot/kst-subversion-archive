/***************************************************************************
                                kstextension.h
                             -------------------
    begin                : Feb 09 2004
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

#ifndef KSTEXTENSION_H
#define KSTEXTENSION_H

#include <qdom.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qtextstream.h>

class KstApp;

class KstExtension : public QObject {
  public:
    KstExtension(QObject *parent, const char *name, const QStringList&);
    virtual ~KstExtension();

    // To save state
    virtual void load(QDomElement& e);
    virtual void save(QTextStream& ts);

    // Clear internal state
    virtual void clear();

    KstApp *app() const;
};

#endif

// vim: ts=2 sw=2 et
