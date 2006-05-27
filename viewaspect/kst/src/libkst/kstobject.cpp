/***************************************************************************
              kstobject.cpp: abstract base class for all Kst objects
                             -------------------
    begin                : May 25, 2003
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

#include "kstobject.h"

class KstObjectPrivate {
  public:
    KstObjectPrivate() {
      _dirty = false;
      _lastUpdate = KstObject::NO_CHANGE;
    }

    bool _dirty;
    KstObject::UpdateType _lastUpdate;
};

static int i = 0;

KstObject::KstObject()
: KstShared(), QObject(), KstRWLock(), _lastUpdateCounter(0), d(new KstObjectPrivate) {
  _tag = "Object" + QString::number(++i);
}


KstObject::~KstObject() {
  delete d;
  d = 0L;
}


int KstObject::operator==(const QString& tag) const {
  return (tag == _tag) ? 1 : 0;
}


// Returns true if update has already been done
bool KstObject::checkUpdateCounter(int update_counter) {
  if (update_counter == _lastUpdateCounter) {
    return true;
  } else if (update_counter > 0) {
    _lastUpdateCounter = update_counter;
  }
  return false;
}


void KstObject::virtual_hook(int id, void *data) {
  Q_UNUSED(id)
  Q_UNUSED(data)
  // id=0 v1.2.0 setDirty() was called.  data == 0 means false, else true
}


const QString& KstObject::tagName() const {
  return _tag;
}


void KstObject::setTagName(const QString& newTag) {
  _tag = newTag;
  setName(_tag.local8Bit().data());
}


QString KstObject::tagLabel() const {
  return QString("[%1]").arg(_tag);
}


int KstObject::getUsage() const {
  return _KShared_count() - 1;
}


bool KstObject::deleteDependents() {
  return false;
}


KstObject::UpdateType KstObject::setLastUpdateResult(UpdateType result) {
  return d->_lastUpdate = result;
}


KstObject::UpdateType KstObject::lastUpdateResult() const {
  return d->_lastUpdate;
}


void KstObject::setDirty(bool dirty) {
  d->_dirty = dirty;
  virtual_hook(0, (void*)dirty);
}


void KstObject::setDirty() {
  setDirty(true);
}


bool KstObject::dirty() const {
  return d->_dirty;
}

// vim: ts=2 sw=2 et
