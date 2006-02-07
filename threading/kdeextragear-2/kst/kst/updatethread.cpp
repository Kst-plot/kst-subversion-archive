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

#include <kdebug.h>

#include "kstdatacollection.h"
#include "kstdoc.h"
#include "kstrvector.h"
#include "threadevents.h"


UpdateThread::UpdateThread(KstDoc *doc)
: QThread(), _paused(false), _done(false), _statusMutex(false), _doc(doc) {

  // Update variables
  _updateCounter = 0;
  _force = false;
}


UpdateThread::~UpdateThread() {
}


void UpdateThread::run() {
  // FIXME XXXX remove me when the rest of the code is threadsafe
  QThread::sleep(5);

  // FIXME: make msleep() configurable here
  for (_done = false; !_done; QThread::msleep(50)) {
    if (_paused) {
      continue;
    }

    bool force = _force;

    _statusMutex.lock();
    _force = false;
    _statusMutex.unlock();

    if (doUpdates(force)) {
      QApplication::postEvent(_doc, new ThreadEvent(ThreadEvent::UpdateDialogs));
    }
  }

  QApplication::postEvent(_doc, new ThreadEvent(ThreadEvent::Done));
}


#define CHECK_DONE() do { if (_done || _paused) return false; } while(0)

bool UpdateThread::doUpdates(bool force) {
  KstObject::UpdateType tU, U = KstObject::NO_CHANGE;
  unsigned i;
  int lastFrame = -1;

  _updateCounter++;
  if (_updateCounter < 1) {
    _updateCounter = 1; // check for wrap around
  }

  //kdDebug() << "UPDATE: counter=" << _updateCounter << endl;

  // Update the files
  KST::dataSourceList.lock().readLock();
  for (i = 0;;) {
    unsigned cnt = KST::dataSourceList.count();
    if (i >= cnt) {
      break;
    }

    KstDataSourcePtr dsp = KST::dataSourceList[i];

    dsp->update();

    if (_done || _paused) {
      KST::dataSourceList.lock().readUnlock();
      return false;
    }
    ++i;
  }
  KST::dataSourceList.lock().readUnlock();


  { // scope to destroy the temporary list
    KstRVectorList rvl = kstObjectSubList<KstVector,KstRVector>(KST::vectorList);

    // Update data vectors
    for (i = 0;;) {
      unsigned cnt = rvl.count();
      if (i >= cnt) {
        break;
      }

      KstRVectorPtr rv = rvl[i];
      rv->writeLock();

      tU = rv->update(_updateCounter);

      if (tU != KstObject::NO_CHANGE) {
        CHECK_DONE();

        lastFrame = rv->numFrames() + rv->startFrame();

        if (tU == KstObject::READ) {
          U = KstObject::READ;
        } else if (U == KstObject::NO_CHANGE) {
          U = KstObject::UPDATE;
        }
      }
      rv->writeUnlock();

      CHECK_DONE();
      ++i;
    }
  }

  if (lastFrame > 0) {
    // FIXME: change to an event
    //emit newFrameMsg(lastFrame);
  }

  // Update all data objects that are not curves
  KST::dataObjectList.lock().readLock();
  for (i = 0; i < KST::dataObjectList.count(); ++i) {
    KstDataObjectPtr dop = KST::dataObjectList[i];

    dop->writeLock();
    if (!dynamic_cast<KstBaseCurve*>(dop.data())) {
      dop->update(_updateCounter);
    }
    dop->writeUnlock();

    if (_done || _paused) {
      KST::dataObjectList.lock().readUnlock();
      return false;
    }
  }

  // Give someone else a chance
  KST::dataObjectList.lock().readUnlock();
  KST::dataObjectList.lock().readLock();

  // Update all curves
  if (U == KstObject::UPDATE || force) {
    for (i = 0; i < KST::dataObjectList.count(); ++i) {
      KstDataObjectPtr dop = KST::dataObjectList[i];

      dop->writeLock();
      if (dynamic_cast<KstBaseCurve*>(dop.data())) {
        dop->update(_updateCounter);
      }
      dop->writeUnlock();

      if (_done || _paused) {
        KST::dataObjectList.lock().readUnlock();
        return false;
      }
      ++i;
    }
  }

  KST::dataObjectList.lock().readUnlock();

  return U == KstObject::UPDATE;
}

#undef CHECK_DONE

void UpdateThread::setPaused(bool paused) {
  QMutexLocker ml(&_statusMutex);
  _paused = paused;
}


void UpdateThread::setFinished(bool finished) {
  QMutexLocker ml(&_statusMutex);
  _done = finished;
}


void UpdateThread::forceUpdate() {
  QMutexLocker ml(&_statusMutex);
  _force = true;
}

// vim: ts=2 sw=2 et
