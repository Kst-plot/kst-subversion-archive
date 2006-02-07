/***************************************************************************
                    planck.cpp  -  data source - planck
                             -------------------
    begin                : Wed Oct 22 2003
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

#include "planck.h"

#include <kdebug.h>

#include <qdir.h>
#include <qfile.h>

 
PlanckSource::PlanckSource(const QString& filename, Planck::Database& db)
: KstDataSource(filename, "PLANCK I/O"), _db(db) {

  for (QMap<QString,Planck::Database::Group>::ConstIterator i = db.groups().begin(); i != db.groups().end(); ++i) {
    for (QMap<QString,Planck::Database::Object>::ConstIterator j = i.data().objects().begin(); j != i.data().objects().end(); ++j) {
      _fieldList += (i.key() + ":" + j.key());
    }
  }

  _planckTOI = new Planck::TOI;
  if (!_planckTOI->setSource(db.path())) {
    _planckTOI = 0L; // deref
  }

  if (_planckTOI && _planckTOI->isValid()) {
    update();
    _valid = true;
  }
}


PlanckSource::~PlanckSource() {
}


KstObject::UpdateType PlanckSource::update(int u) {
  Q_UNUSED(u)
  // We have to update:
  //      - group list - a group disappears
  //      - object list - an object disappears from a group being watched
  //      - ranges - range of an object in a group being watched is changed
  return KstObject::NO_CHANGE;
}


int PlanckSource::readField(double *v, const QString& field, int s, int n) {

  if (field.lower() == "index") {
    for (int i = 0; i < n; ++i) {
      v[i] = double(s + i);
    }
    return n > 0 ? n : 0;
  }

  if (!_valid || !_planckTOI || !_planckTOI->isValid()) {
    kdDebug() << "tried to read from an invalid planck source" << endl;
    kdDebug() << "plugin is valid? " << _valid << endl;
    kdDebug() << "TOI object is non-null? " << (_planckTOI.data() != 0) << endl;
    return -1;
  }

  QStringList split = QStringList::split(':', field);
  if (split.count() != 2) {
    kdDebug() << "invalid field: [" << field << "]" << endl;
    return -1;
  }

  Planck::Database::Group grp;
  bool foundGroup = false;
  for (QMap<QString,Planck::Database::Group>::ConstIterator i = _db.groups().begin(); i != _db.groups().end(); ++i) {
    if (i.key() == split[0]) {
      grp = *i;
      foundGroup = true;
      break;
    }
  }
  
  if (!foundGroup) {
    kdDebug() << "couldn't find that group" << endl;
    return -1;
  }

  if (!grp.objects().contains(split[1])) {
    kdDebug() << "couldn't find that object" << endl;
    return -1;
  }

  QSize sz = _planckTOI->range(split[0], split[1]);

  if (s > sz.height() - sz.width()) {
    return 0;
  }

  if (s + n > sz.height() - sz.width()) {
    n = sz.height() - sz.width() - s;
  }

  if (n < 0) {
    n = 1;
  }

  if (s + sz.width() >= sz.height() - 1) {
    kdDebug() << "Nothing to read: (" << sz.width() << "," << sz.height() << ") " << s << endl;
    return 0;
  }

  return _planckTOI->readObject(split[0], split[1], v, sz.width() + s, sz.width() + s + n - 1);
}


bool PlanckSource::isValidField(const QString& field) const {
  return field.lower() == "index" || _fieldList.contains(field);
}


int PlanckSource::samplesPerFrame(const QString &field) {
  Q_UNUSED(field)
  return 1;
}


int PlanckSource::frameCount() const {
  if (!_valid) {
    return 0;
  }

  // FIXME
  QStringList split = QStringList::split(':', _fieldList.last());
  if (split.count() != 2) {
    kdDebug() << "invalid field: [" << _fieldList.last() << "]" << endl;
    return 0;
  }
  QSize sz = _planckTOI->range(split[0], split[1]);
  // piolib seems to lie.  it says the range is x..y but it's actually x..y-1
  return sz.height() - sz.width();
}


QString PlanckSource::fileType() const {
  return "PLANCK I/O";
}


void PlanckSource::save(QTextStream &ts) {
  KstDataSource::save(ts);
}


static bool parseXML(const QString& filename, Planck::Database& db) {
  if (!QFile::exists(filename)) {
    return false;
  }

  QFile f(filename);
  if (!f.open(IO_ReadOnly)) {
    return false;
  }

  if (0 != Planck::Parser::parse(&f, db)) {
    return false;
  }

  if (!QDir(db.path()).exists()) {
    //return false;  FIXME
  }
  return true;
}

extern "C" {
KstDataSource *create_planck(const QString& filename, const QString& type) {
  Q_UNUSED(type)
  Planck::Database db;
  if (!parseXML(filename, db)) {
    return 0L;
  }
  return new PlanckSource(filename, db);
}

QStringList provides_planck() {
  QStringList rc;
  rc += "PLANCK I/O";
  return rc;
}

bool understands_planck(const QString& filename) {
  Planck::Database db;
  if (!parseXML(filename, db)) {
    return false;
  }
  return true;
}

}

// vim: ts=2 sw=2 et
