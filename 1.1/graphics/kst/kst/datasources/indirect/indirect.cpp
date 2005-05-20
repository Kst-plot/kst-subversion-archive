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
 

IndirectSource::IndirectSource(KConfig *cfg, const QString& filename, KstDataSourcePtr child)
: KstDataSource(cfg, filename, QString::null), _child(child) {
  if (child) {
    _valid = true;
    _fieldList = child->fieldList();
  } else {
    _valid = false;
  }
}


IndirectSource::~IndirectSource() {
}


KstObject::UpdateType IndirectSource::update(int u) {
  if (KstObject::checkUpdateCounter(u)) {
    return lastUpdateResult();
  }

  // recheck the indirect file for a changed filename
  QFile f(_filename);
  if (f.open(IO_ReadOnly)) {
    QString ifn;
    if (0 < f.readLine(ifn, 1000)) {
      if (!_child || ifn.stripWhiteSpace() != _child->fileName()) {
        _child = 0L; // release
        KstDataSourcePtr p = KstDataSource::loadSource(ifn.stripWhiteSpace());
        if (p) {
          _child = p;
          _fieldList = p->fieldList();
          _valid = true;
        } else {
          _valid = false;
        }
      }
    }
  }

  return setLastUpdateResult(_child ? _child->update(u) : KstObject::NO_CHANGE);
}


int IndirectSource::readField(double *v, const QString& field, int s, int n) {
  return _child ? _child->readField(v, field, s, n) : -1;
}


bool IndirectSource::isValidField(const QString& field) const {
  return _child ? _child->isValidField(field) : false;
}


bool IndirectSource::reset() {
  return _child ? _child->reset() : KstDataSource::reset();
}


int IndirectSource::samplesPerFrame(const QString &field) {
  return _child ? _child->samplesPerFrame(field) : 0;
}


int IndirectSource::frameCount(const QString& field) const {
  return _child ? _child->frameCount(field) : 0;
}


QString IndirectSource::fileType() const {
  return "Indirect";
}


void IndirectSource::save(QTextStream &ts, const QString& indent) {
  KstDataSource::save(ts, indent);
}


bool IndirectSource::isValid() const {
  return KstDataSource::isValid() && _child && _child->isValid();
}


bool IndirectSource::isEmpty() const {
  return _child ? _child->isEmpty() : true;
}


extern "C" {
KstDataSource *create_indirect(KConfig *cfg, const QString& filename, const QString& type) {
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

  return new IndirectSource(cfg, filename, p);
}


QStringList provides_indirect() {
  QStringList rc;
  rc += "Indirect";
  return rc;
}


int understands_indirect(KConfig*, const QString& filename) {
  int percent = 50;

  if (filename.endsWith(".cur")) { // Do we really have to do this?
    percent = 100;
  }

  QFile f(filename);
  if (!f.open(IO_ReadOnly)) {
    return 0;
  }

  QString ifn;
  if (0 >= f.readLine(ifn, 1000)) {
    return 0;
  }

  return QFile::exists(ifn.stripWhiteSpace()) ? percent : 0;
}


QStringList fieldList_indirect(KConfig *cfg, const QString& filename, const QString& type, QString *typeSuggestion, bool *complete) {
  if ((!type.isEmpty() && !provides_indirect().contains(type)) || !understands_indirect(cfg, filename)) {
      return QStringList();
  }

  QFile f(filename);
  if (!f.open(IO_ReadOnly)) {
    return QStringList();
  }

  QString ifn;
  if (0 >= f.readLine(ifn, 1000)) {
    return QStringList();
  }

  return KstDataSource::fieldListForSource(ifn.stripWhiteSpace(), type, typeSuggestion, complete);
}

}

// vim: ts=2 sw=2 et
