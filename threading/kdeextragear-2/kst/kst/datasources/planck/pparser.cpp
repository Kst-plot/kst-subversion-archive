/***************************************************************************
                           pparser.cpp 
    begin                : Thu Oct 23 2003
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

#include "pparser.h"
#include <qdom.h>
#include <qfile.h>
#include <qmessagebox.h>

int Planck::Parser::parse(QFile *f, Planck::Database& db) {
  QDomDocument doc;
  int errline = 0, errcol = 0;
  QString err;
  if (!doc.setContent(f, &err, &errline, &errcol)) {
    return -1;
  }

  for (QDomNode d = doc.firstChild(); !d.isNull(); d = d.nextSibling()) {
    if (!d.isElement() || d.toElement().tagName().lower() != "database") {
      continue;
    }

    db.clear();

    QDomElement de = d.toElement();
    db._path = de.attribute("path");

    for (QDomNode n = de.firstChild(); !n.isNull(); n = n.nextSibling()) {
      if (n.isElement()) {
        QDomElement e = n.toElement();
        if (e.tagName().lower() != "group") {
          continue;
        }

        Planck::Database::Group g;

        for (QDomNode o = e.firstChild(); !o.isNull(); o = o.nextSibling()) {
          if (o.isElement()) {
            QDomElement oe = o.toElement();
            if (oe.tagName().lower() != "object") {
              continue;
            }

            Planck::Database::Object onode;
            onode.type = oe.attribute("type");
            onode.datatype = oe.attribute("datatype");
            onode.begin = oe.attribute("begin").toLong();
            onode.end = oe.attribute("end").toLong();
            onode.beginring = oe.attribute("beginring").toLong();
            onode.endring = oe.attribute("endring").toLong();
            g._objects[oe.attribute("name")] = onode;
          }
        }

        db._groups[e.attribute("name")] = g;
      }
    }

    break; // don't parse any more databases
  }
  return 0;
}

// vim: ts=2 sw=2 et
