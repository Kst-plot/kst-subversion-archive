/***************************************************************************
                              updatethread.cpp
                              ----------------
    begin                : Dec 29 2003
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

#include "updatethread.h"

#include <assert.h>

#include <kdebug.h>

#include "kstdatacollection.h"
#include "kstdoc.h"
#include "kstrvector.h"
#include "threadevents.h"

// 0 - none, 1 - some, 2 - lots, 3 - too much
#define UPDATEDEBUG 0

UpdateThread::UpdateThread(KstDoc *doc)
: QThread(), _paused(false), _done(false), _statusMutex(false), _doc(doc) {

  // Update variables
  _updateCounter = 0;
  _force = false;
}


UpdateThread::~UpdateThread() {
}


void UpdateThread::run() {
  bool force;
  int  updateTime;
  
  _done = false;
  
  while (!_done) {
    _statusMutex.lock();
    updateTime = _updateTime;
    _statusMutex.unlock();
    
    // FIXME: cannot ship this, waitcondition is broken on most(all?) unixes
    if (_waitCondition.wait(_updateTime)) {
#if UPDATEDEBUG > 0
      kdDebug() << "Update timer" << _updateTime << endl;
#endif
      break;
    }
    
    if (_paused) {
#if UPDATEDEBUG > 0
      kdDebug() << "Update thread paused..." << endl;
#endif
      continue;
    }

    force = _force;

    _statusMutex.lock();
    _force = false;
    _statusMutex.unlock();

    if (doUpdates(force)) {
#if UPDATEDEBUG > 1
      kdDebug() << "Update resulted in: TRUE!" << endl;
#endif
      QApplication::postEvent(_doc, new ThreadEvent(ThreadEvent::UpdateDialogs));
    }
  }

  QApplication::postEvent(_doc, new ThreadEvent(ThreadEvent::Done));
}


bool UpdateThread::doUpdates(bool force) {
  KstObject::UpdateType tU, U = KstObject::NO_CHANGE;
  unsigned i;
  int lastFrame = -1;

#if UPDATEDEBUG > 0
  if (force) {
    kdDebug() << "Forced update!" << endl;
  }
#endif

  _updateCounter++;
  if (_updateCounter < 1) {
    _updateCounter = 1; // check for wrap around
  }

#if UPDATEDEBUG > 2
  kdDebug() << "UPDATE: counter=" << _updateCounter << endl;
#endif

  // Update the files
  KST::dataSourceList.lock().readLock();
  unsigned cnt = KST::dataSourceList.count();
  for (i = 0; i < cnt; ++i) {
    KstDataSourcePtr dsp = KST::dataSourceList[i];

    dsp->update();

    if (_done || _paused) {
      KST::dataSourceList.lock().readUnlock();
#if UPDATEDEBUG > 1
      kdDebug() << "1 Returning from scan with U=" << (int)U << endl;
#endif
      return U == KstObject::UPDATE;
    }
  }
  KST::dataSourceList.lock().readUnlock();


  { // scope to destroy the temporary list
    KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);

    unsigned cnt = rvl.count();

    // Update data vectors
    for (i = 0; i < cnt; ++i) {
      KstRVectorPtr rv = rvl[i];
      rv->writeLock();

      tU = rv->update(_updateCounter);

      if (tU != KstObject::NO_CHANGE) {
        if (_done || _paused) {
          rv->writeUnlock();
#if UPDATEDEBUG > 1
          kdDebug() << "2 Returning from scan with U=" << (int)U << endl;
#endif
          return U == KstObject::UPDATE;
        }

        lastFrame = rv->numFrames() + rv->startFrame();

        if (U == KstObject::NO_CHANGE) {
          U = KstObject::UPDATE;
        }
      }
      rv->writeUnlock();

      if (_done || _paused) {
#if UPDATEDEBUG > 1
        kdDebug() << "3 Returning from scan with U=" << (int)U << endl;
#endif
        return U == KstObject::UPDATE;
      }
    }
  }

  if (lastFrame > 0) {
    // FIXME: change to an event
    //emit newFrameMsg(lastFrame);
  }

  {
    // Must make a copy to avoid deadlock
    KstBaseCurveList cl;
    KstDataObjectList ncl;
    kstObjectSplitList<KstDataObject,KstBaseCurve>(KST::dataObjectList, cl, ncl);
  
    // Update all data objects that are not curves
    for (i = 0; i < ncl.count(); ++i) {
      KstDataObjectPtr dop = ncl[i];
      assert(dop.data());
#if UPDATEDEBUG > 0
      kdDebug() << "updating non-curve: " << (void*)dop << " - " << dop->tagName() << endl;
#endif
      dop->writeLock();
      dop->update(_updateCounter);
      dop->writeUnlock();

      if (_done || _paused) {
#if UPDATEDEBUG > 1
        kdDebug() << "4 Returning from scan with U=" << (int)U << endl;
#endif
        return U == KstObject::UPDATE;
      }
    }

    // Update all curves
    if (U == KstObject::UPDATE || force) {
      for (i = 0; i < cl.count(); ++i) {
        KstBaseCurvePtr bcp = cl[i];
        assert(bcp.data());
#if UPDATEDEBUG > 0
        kdDebug() << "updating curve: " << (void*)bcp << " - " << bcp->tagName() << endl;
#endif
        bcp->writeLock();
        bcp->update(_updateCounter);
        bcp->writeUnlock();

        if (_done || _paused) {
#if UPDATEDEBUG > 1
          kdDebug() << "5 Returning from scan with U=" << (int)U << endl;
#endif
          return U == KstObject::UPDATE;
        }
      }    
    }
  }

#if UPDATEDEBUG > 1
  kdDebug() << "6 Returning from scan with U=" << (int)U << endl;
#endif
  return U == KstObject::UPDATE;
}


void UpdateThread::setUpdateTime(int updateTime) {
  QMutexLocker ml(&_statusMutex);
  _updateTime = updateTime;
}


void UpdateThread::setPaused(bool paused) {
  QMutexLocker ml(&_statusMutex);
  _paused = paused;
}


void UpdateThread::setFinished(bool finished) {
  QMutexLocker ml(&_statusMutex);
  _done = finished;
  _waitCondition.wakeOne();
}


void UpdateThread::forceUpdate() {
  QMutexLocker ml(&_statusMutex);
  _force = true;
}

// vim: ts=2 sw=2 et
