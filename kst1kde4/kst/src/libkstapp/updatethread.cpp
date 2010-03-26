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

#ifdef MULTICORE_UPDATES
#include "updatethread-multicore.cpp"
#else
#include <assert.h>

#include "kstdatacollection.h"
#include "kstdataobjectcollection.h"
#include "kstdoc.h"
#include "kstrvector.h"
#include "kstvcurve.h"
#include "threadevents.h"

UpdateThread::UpdateThread(KstDoc *doc)
: QThread(), _statusMutex(QMutex::NonRecursive), _doc(doc), _paused(false), _done(false) {
  _updateCounter = 0;
  _updateImmediate = false;
  _updateRunning = false;
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
    if (_updateImmediate) {
      updateTime = 0;
    } else {
      updateTime = _updateTime;
    }
    _statusMutex.unlock();

    if (_waitCondition.wait(updateTime)) {
      if (!_force) {
        break;
      }
    }

    _statusMutex.lock();
    if (_done) {
      _statusMutex.unlock();
      break;
    }
    force = _force;
    _force = false;

    //
    // if we have requested an immediate update and are awaiting its completion
    //  then set the necessary flags so we can monitor the start and end of the
    //  update routine...
    //
    if (_updateImmediate) {
      _updateImmediate = false;
      _updateRunning = true;
    }
    _statusMutex.unlock();

    if (paused() && !force) {
      continue;
    }

    bool gotData = false;
    if (doUpdates(force, &gotData) && !_done) {
      if (gotData) {
        ThreadEvent *e = new ThreadEvent(ThreadEvent::UpdateDataDialogs);

        e->_curves = _updatedCurves;
        e->_counter = _updateCounter;

        QApplication::postEvent(_doc, e);
        // this event also triggers an implicit repaint
      } else {
        QApplication::postEvent(_doc, new ThreadEvent(ThreadEvent::Repaint));
      }
      // Wait for UI thread to finish events.  If we don't wait
      // 1: the UI thread could get flooded with events
      // 2: the update thread could change vectors during a paint, causing
      //    inconsistent curves to be plotted ie, the X vector updated
      //    and the Y vector not yet updated...

      // Race warning: Synchronization of updating() is not assured,
      // but updating() will always return a valid answer which was
      // true 'close' to when we asked.  This will safely keep the
      // update thread from over filling the UI thread.  The usleeps
      // will hopefully give the UI thread a chance to set itself...
      usleep(1000); // 1 ms on 2.6 kernel. 10ms on 2.4 kernel

      while (!_done && _doc->updating()) {  // wait for the UI to finish old events
        usleep(1000);
      }
      usleep(1000);
      // check again... not strictly needed given implicit repaint below,
      // but it should just return false, so no harm done.
      while (!_done && _doc->updating()) {
        usleep(1000);
      }
    } else {
      QApplication::postEvent(_doc, new ThreadEvent(ThreadEvent::NoUpdate));
    }

    _statusMutex.lock();
    _updateRunning = false;
    _statusMutex.unlock();
  }

  _statusMutex.lock();
  _updateImmediate = false;
  _updateRunning = false;
  _statusMutex.unlock();

  QApplication::postEvent(_doc, new ThreadEvent(ThreadEvent::Done));
}


bool UpdateThread::doUpdates(bool force, bool *gotData) {
  KstObject::UpdateType U = KstObject::NO_CHANGE;

  _updatedCurves.clear(); // HACK

  if (gotData) {
    *gotData = false;
  }

  _updateCounter++;
  if (_updateCounter < 1) {
    _updateCounter = 1; // check for wrap around
  }

  {
    // Must make a copy to avoid deadlock
    KstBaseCurveList cl;
    KstDataObjectList dol;
    kstObjectSplitList<KstDataObject, KstBaseCurve>(KST::dataObjectList, cl, dol);
    qHeapSort(cl);
    qHeapSort(dol);

    //
    // update all curves
    //
    for (uint i = 0; i < cl.count(); ++i) {
      KstBaseCurvePtr bcp = cl[i];

      bcp->writeLock();
      assert(bcp.data());

      KstObject::UpdateType ut = bcp->update(_updateCounter);
      bcp->unlock();

      if (ut == KstObject::UPDATE) { // HACK
        _updatedCurves.append(bcp);
      }

      if (U != KstObject::UPDATE) {
        U = ut;
        if (U == KstObject::UPDATE) {
        }
      }

      if (_done || (_paused && !force)) {
        return U == KstObject::UPDATE;
      }
    }

    //
    // update all data objects
    //
    for (uint i = 0; i < dol.count(); ++i) {
      KstDataObjectPtr dp = dol[i];

      dp->writeLock();
      assert(dp.data());

      dp->update(_updateCounter);
      dp->unlock();

      if (_done || (_paused && !force)) {
        return U == KstObject::UPDATE;
      }
    }
  }

  //
  // update the files
  //
  if (!_paused) { // don't update even if paused && force
    KST::dataSourceList.lock().readLock();
    unsigned cnt = KST::dataSourceList.count();
    for (uint i = 0; i < cnt; ++i) {
      KstDataSourcePtr dsp = KST::dataSourceList[i];
      if (dsp) {
        dsp->writeLock();
        dsp->update(_updateCounter);
        dsp->unlock();
      }
      if (_done) {
        KST::dataSourceList.lock().unlock();
        return false;
      }
    }
    KST::dataSourceList.lock().unlock();
  }

  //
  // update all data vectors
  //
  {
    KST::vectorList.lock().readLock();
    KstVectorList vl = QDeepCopy<KstVectorList>(KST::vectorList.list()); // avoid deadlock on exit
    KST::vectorList.lock().unlock();
    for (KstVectorList::ConstIterator i = vl.begin(); i != vl.end(); ++i) {
      KstRVectorPtr rv = kst_cast<KstRVector>(*i);
      if (rv) {
        rv->writeLock();
        rv->update(_updateCounter);
        rv->unlock();
      }
      if (_done) {
        return false;
      }
    }
  }

  //
  // update all scalars
  //
  if (KstScalar::scalarsDirty()) {
    KstScalar::clearScalarsDirty();
    KST::scalarList.lock().readLock();
    KstScalarList sl = QDeepCopy<KstScalarList>(KST::scalarList.list()); // avoid deadlock on exit
    KST::scalarList.lock().unlock();
    for (KstScalarList::ConstIterator i = sl.begin(); i != sl.end(); ++i) {
      KstScalarPtr sp = *i;

      sp->writeLock();
      KstObject::UpdateType ut = sp->update(_updateCounter);
      sp->unlock();

      if (ut == KstObject::UPDATE) {
        U = KstObject::UPDATE;
      }

      if (_done) {
        return false;
      }
    }
  }

  if (U == KstObject::UPDATE) {
    if (gotData) { // FIXME: do we need to consider all the other exit points?
      *gotData = true;
    }
  }

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
  _statusMutex.lock();
  _done = finished;
  _force = false;
  _statusMutex.unlock();
  _waitCondition.wakeOne();
}


void UpdateThread::forceUpdate() {
  if (_done) {
    return;
  }

  _statusMutex.lock();
  _updateImmediate = true;
  _force = true;
  _statusMutex.unlock();
  _waitCondition.wakeOne();
}


void UpdateThread::waitForUpdate() {
  bool updateImmediateLocal;
  bool updateRunningLocal;

  _statusMutex.lock();
  _updateImmediate = true;
  _force = true;
  _statusMutex.unlock();

  //
  // the update thread might be in the wait condition,
  //  so we try and kick the update thread out of the wait condition...
  //
  _waitCondition.wakeOne();

  //
  // now we need to wait until we enter the main body of the update thread...
  //

  while(true) {
    {
      QMutexLocker ml(&_statusMutex);
      updateImmediateLocal  = _updateImmediate;
    }
	
    if (updateImmediateLocal) {
      usleep(1);
    } else {
      break;
    }
  }

  //
  // now we need to wait until we leave the main body of the update thread...
  //
  while(true) {
    {
      QMutexLocker ml(&_statusMutex);
      updateRunningLocal = _updateRunning;
    }

    if (updateRunningLocal) {
      usleep(1);
    } else {
      break;
    }
  }
}


bool UpdateThread::paused() const {
  QMutexLocker ml(&_statusMutex);
  return _paused;
}

#endif
