/***************************************************************************
                              kstprimitive.cpp
                             -------------------
    begin                : Tue Jun 20, 2006
    copyright            : Copyright (C) 2006, The University of Toronto
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *   Permission is granted to link with any opensource library             *
 *                                                                         *
 ***************************************************************************/

#include "kstprimitive.h"

#include <assert.h>

#include "ksdebug.h"


KstPrimitive::KstPrimitive(KstObject *provider)
: KstObject(), _provider(provider), _lockMutex(true), _unlockMutex(true), _inReadLock(false), _inWriteLock(false), _inUnlock(false) {
}


KstPrimitive::~KstPrimitive() {
}


KstObject::UpdateType KstPrimitive::update(int update_counter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  KstObject::UpdateType providerRC = NO_CHANGE;

  if (update_counter > 0) {
    KstObjectPtr prov = KstObjectPtr(_provider);
    if (prov) {
      // provider is already locked
      providerRC = prov->update(update_counter);
      if (!force && providerRC == KstObject::NO_CHANGE) {
        return setLastUpdateResult(providerRC);
      }
    }
  }

  KstObject::UpdateType rc = internalUpdate(providerRC);
  setDirty(false);
  return rc;
}


KstObject::UpdateType KstPrimitive::internalUpdate(KstObject::UpdateType providerRC) {
  Q_UNUSED(providerRC)
  return setLastUpdateResult(NO_CHANGE);
}


void KstPrimitive::readLock() const {
  KstObjectPtr prov = KstObjectPtr(_provider);
  if (prov) {
    _lockMutex.lock();
    if (!_inReadLock) {
      _inReadLock = true;
      prov->readLock();
      _inReadLock = false;
    }
    _lockMutex.unlock();
  } else {
    KstObject::readLock();
  }
}


void KstPrimitive::writeLock() const {
  KstObjectPtr prov = KstObjectPtr(_provider);
  if (prov) {
    _lockMutex.lock();
    if (!_inWriteLock) {
      _inWriteLock = true;
      prov->writeLock();
      _inWriteLock = false;
    }
    _lockMutex.unlock();
  } else {
    KstObject::writeLock();
  }
}


void KstPrimitive::unlock() const {
  KstObjectPtr prov = KstObjectPtr(_provider);
  if (prov) {
    _unlockMutex.lock();
    if (!_inUnlock) {
      _inUnlock = true;
      prov->unlock();
      _inUnlock = false;
    }
    _unlockMutex.unlock();
  } else {
    KstObject::unlock();
  }
}


// vim: et sw=2 ts=2
