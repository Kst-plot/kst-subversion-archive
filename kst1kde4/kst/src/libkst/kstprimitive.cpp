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

#include <assert.h>

#include <kdebug.h>

#include "kstprimitive.h"

KstPrimitive::KstPrimitive(KstObject *provider)
: KstObject(), _provider(provider) {
}


KstPrimitive::KstPrimitive(const KstPrimitive& primitive)
: KstObject(), _provider(primitive._provider) {
}


KstPrimitive::~KstPrimitive() {
}


KstObject::UpdateType KstPrimitive::update(int update_counter) {
#ifdef UPDATEDEBUG
  kstdDebug() << "Updating Primitive " << tag().displayString() << endl;
#endif
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(update_counter) && !force) {
    return lastUpdateResult();
  }

  KstObject::UpdateType providerRC = NO_CHANGE;

  if (update_counter > 0) {
    //
    // use a KstObjectPtr to prevent provider being deleted during update
    //
    KstObjectPtr prov = KstObjectPtr(_provider);
    if (prov) {
      prov->writeLock();

      providerRC = prov->update(update_counter);
      if (!force && providerRC == KstObject::NO_CHANGE) {
	prov->unlock();
	
	return setLastUpdateResult(providerRC);
      }
      
      prov->unlock();
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
