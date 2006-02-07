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

 
PlanckSource::PlanckSource(const QString& filename)
: KstDataSource(filename, "PLANCK I/O") {

  _planckObject = new Planck::Object;
  if (!_planckObject->setGroup(filename)) {
    _planckObject = 0L; // deref
  }

  if (_planckObject && _planckObject->isValid()) {
    _fieldList = _planckObject->fields();
    update();
    _valid = true;
  }
}


PlanckSource::~PlanckSource() {
}


KstObject::UpdateType PlanckSource::update(int u) {
  Q_UNUSED(u)
  if (_valid && _planckObject && _planckObject->updated()) {
    return KstObject::UPDATE;
  }
  return KstObject::NO_CHANGE;
}


int PlanckSource::readField(double *v, const QString& field, int s, int n) {
  kdDebug() << "Planck read field " << field << " - " << n << " samples from " << s << endl;
  if (field.lower() == "index") {
    if (n < 0) {
      v[0] = double(s);
      return 1;
    }
    for (int i = 0; i < n; ++i) {
      v[i] = double(s + i);
    }
    return n > 0 ? n : 0;
  }

  if (!_valid || !_planckObject || !_planckObject->isValid()) {
    kdDebug() << "tried to read from an invalid planck source" << endl;
    kdDebug() << "plugin is valid? " << _valid << endl;
    kdDebug() << "Object object is non-null? " << (_planckObject.data() != 0) << endl;
    return -1;
  }

  QSize sz = _planckObject->range(field);
  long start = sz.width(), end = sz.height(), count = end - start + 1;

  if (s >= count) { // start past the end
    return 0;
  }

  if (n < 0) { // reading less than 0 -> read only one sample!
    n = 1;
  }

  if (s + n > count) { // trying to read past the end
    n = count - s;
  }

  if (s + start > end) {
    kdDebug() << "Nothing to read: (" << start << "," << end << ") " << s << endl;
    return 0;
  }

  return _planckObject->readObject(field, v, start + s, start + s + n - 1);
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

  // FIXME: can't use the last field!!
  QSize sz = _planckObject->range(_fieldList.last());
  return sz.height() - sz.width() + 1;
}


QString PlanckSource::fileType() const {
  return "PLANCK I/O";
}


void PlanckSource::save(QTextStream &ts) {
  KstDataSource::save(ts);
}


extern "C" {
KstDataSource *create_planck(const QString& filename, const QString& type) {
  Q_UNUSED(type)
  return new PlanckSource(filename);
}

QStringList provides_planck() {
  QStringList rc;
  rc += "PLANCK I/O";
  return rc;
}

bool understands_planck(const QString& filename) {
  bool rc = Planck::validDatabase(filename);
  kdDebug() << "-> Valid database? " << rc << endl;
  return rc;
}

}

// vim: ts=2 sw=2 et
