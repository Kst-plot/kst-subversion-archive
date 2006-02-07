/***************************************************************************
                          pparser.h  -  Part of KST
                             -------------------
    begin                : Thu Oct 24 2003
    copyright            : (C) 2003 The University of Toronto
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

#ifndef _KST_PLANCKPARSER_H
#define _KST_PLANCKPARSER_H

#include <qmap.h>
#include <qstring.h>

class QFile;

namespace Planck {

class Database {
  friend class Parser;

  public:
    class Object {
      public:
        QString type, datatype;
        long begin, end, beginring, endring;
    };
    
    class Group {
      friend class Parser;
      public:
        void clear() { _objects.clear(); }
        const QMap<QString,Object>& objects() const { return _objects; }

      private:
        QMap<QString,Object> _objects;
    };
    
    void clear() { _groups.clear(); _path = QString::null; }
    const QMap<QString,Group>& groups() const { return _groups; }
    const QString& path() const { return _path; }

  private:
    QMap<QString,Group> _groups;
    QString _path;
};


class Parser {
  public:
    static int parse(QFile *f, Planck::Database& db);
};

}

#endif

// vim: ts=2 sw=2 et
