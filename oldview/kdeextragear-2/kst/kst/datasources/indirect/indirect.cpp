/***************************************************************************
                    indirect.cpp  -  indirect data source
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

#include "indirect.h"

#include <qfile.h>
 

IndirectSource::IndirectSource(const QString& filename, KstDataSourcePtr child)
: KstDataSource(filename, QString::null), _child(child) {
  _valid = true;
  _fieldList = child->fieldList();
}


IndirectSource::~IndirectSource() {
}


KstObject::UpdateType IndirectSource::update(int u) {
  // recheck the indirect file for a changed filename
  QFile f(_filename);
  if (f.open(IO_ReadOnly)) {
    QString ifn;
    if (0 < f.readLine(ifn, 1000)) {
      if (ifn.stripWhiteSpace() != _child->fileName()) {
        KstDataSourcePtr p = KstDataSource::loadSource(ifn.stripWhiteSpace());
        if (p) {
          _child = p;
          _fieldList = p->fieldList();
        } else {
          _valid = false;
        }
      }
    }
  }

  return _child->update(u);
}


int IndirectSource::readField(double *v, const QString& field, int s, int n) {
  return _child->readField(v, field, s, n);
}


bool IndirectSource::isValidField(const QString& field) const {
  return _child->isValidField(field);
}


int IndirectSource::samplesPerFrame(const QString &field) {
  return _child->samplesPerFrame(field);
}


int IndirectSource::frameCount() const {
  return _child->frameCount();
}


QString IndirectSource::fileType() const {
  return "Indirect";
}


void IndirectSource::save(QTextStream &ts) {
  KstDataSource::save(ts);
}


bool IndirectSource::isValid() const {
  return KstDataSource::isValid() && _child->isValid();
}


extern "C" {
KstDataSource *create_indirect(const QString& filename, const QString& type) {
  if (!type.isEmpty() && type != "Indirect") {
    return 0L;
  }

  QFile f(filename);
  if (!f.open(IO_ReadOnly)) {
    return 0L;
  }

  QString ifn;
  if (0 >= f.readLine(ifn, 1000)) {
    return 0L;
  }

  KstDataSourcePtr p = KstDataSource::loadSource(ifn.stripWhiteSpace());
  f.close();

  if (!p) {
    return 0L;
  }

  return new IndirectSource(filename, p);
}

QStringList provides_indirect() {
  QStringList rc;
  rc += "Indirect";
  return rc;
}

bool understands_indirect(const QString& filename) {
  if (!filename.endsWith(".cur")) { // Do we really have to do this?
    return false;
  }

  QFile f(filename);
  if (!f.open(IO_ReadOnly)) {
    return false;
  }

  QString ifn;
  if (0 >= f.readLine(ifn, 1000)) {
    return false;
  }

  return QFile::exists(ifn.stripWhiteSpace());
}

}

// vim: ts=2 sw=2 et
