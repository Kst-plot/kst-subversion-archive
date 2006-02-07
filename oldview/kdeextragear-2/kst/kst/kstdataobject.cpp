/***************************************************************************
                  kstdataobject.cpp: base class for data objects
                             -------------------
    begin                : May 20, 2003
    copyright            : (C) 2003 by C. Barth Netterfield
                           (C) 2003 The University of Toronto
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

#include "kstdatacollection.h"
#include "kstdataobject.h"
#include "kstdebug.h"

#include <qthread.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>

//#define LOCKTRACE

KstDataObject::KstDataObject() : KstObject() {
  //kdDebug() << "+++ CREATING DATA OBJECT: " << (void*)this << endl;
}

KstDataObject::KstDataObject(QDomElement& e) : KstObject() {
  Q_UNUSED(e)
  //kdDebug() << "+++ CREATING DATA OBJECT: " << (void*)this << endl;
}

KstDataObject::~KstDataObject() {
  // Remove our slave vectors and scalars
  KST::scalarList.lock().writeLock();
  for (KstScalarMap::Iterator it = _outputScalars.begin();
                               it != _outputScalars.end();
                                                      ++it) {
    KST::scalarList.remove(it.data());
  }
  KST::scalarList.lock().writeUnlock();

  KST::vectorList.lock().writeLock();
  for (KstVectorMap::Iterator it = _outputVectors.begin();
                               it != _outputVectors.end();
                                                      ++it) {
    KST::vectorList.remove(it.data());
  }
  KST::vectorList.lock().writeUnlock();
  //kdDebug() << "+++ DESTROYING DATA OBJECT: " << (void*)this << endl;
}

double *KstDataObject::vectorRealloced(KstVectorPtr v, double *memptr, int newSize) const {
  if (v.data() == 0L) {
    return 0L;
  }

  // One would think this needs special locking, but it results in deadlock
  // in complicated object hierarchies such as filtered vectors.  Therefore if
  // you call vectorRealloced() and v is not locked by you already, you'd
  // better lock it!
  double *rc = v->realloced(memptr, newSize);

  return rc;
}

void KstDataObject::save(QTextStream& ts) {
  Q_UNUSED(ts)
}

bool KstDataObject::loadInputs() {
  bool rc = true;
  QValueList<QPair<QString,QString> >::Iterator i;
  KST::vectorList.lock().readLock();
  for (i = _inputVectorLoadQueue.begin(); i != _inputVectorLoadQueue.end(); ++i) {
    KstVectorList::Iterator it = KST::vectorList.findTag((*i).second);
    if (it != KST::vectorList.end()) {
      _inputVectors.insert((*i).first, *it);
    } else {
      KstDebug::self()->log(i18n("Unable to find required vector [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
      rc = false;
    }
  }
  KST::vectorList.lock().readUnlock();

  KST::scalarList.lock().readLock();
  for (i = _inputScalarLoadQueue.begin(); i != _inputScalarLoadQueue.end(); ++i) {
    KstScalarList::Iterator it = KST::scalarList.findTag((*i).second);
    if (it != KST::scalarList.end()) {
      _inputScalars.insert((*i).first, *it);
    } else {
      KstDebug::self()->log(i18n("Unable to find required scalar [%1] for data object %2.").arg((*i).second).arg(tagName()), KstDebug::Error);
      rc = false;
    }
  }
  KST::scalarList.lock().readUnlock();

  _inputVectorLoadQueue.clear();
  _inputScalarLoadQueue.clear();
  update();
  return rc;
}

int KstDataObject::getUsage() const {
  int rc = 0;

  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    if (i.data().data()) {
      rc += i.data()->getUsage() - 1;
    }
  }

  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    if (i.data().data()) {
      rc += i.data()->getUsage() - 1;
    }
  }

  return KstObject::getUsage() + rc;
}


void KstDataObject::showDialog() {
  QTimer::singleShot(0, this, SLOT(_showDialog()));
}


void KstDataObject::readLock() const {
  KstObject::readLock();
  #ifdef LOCKTRACE
  kdDebug() << "Read lock by tid=" << (int)QThread::currentThread() << endl;
  #endif
  for (KstScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    (*i)->readLock();
  }
  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    (*i)->readLock();
  }
  for (KstVectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    (*i)->readLock();
  }
  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    (*i)->readLock();
  }
}


void KstDataObject::readUnlock() const {
  #ifdef LOCKTRACE
  kdDebug() << "Read unlock by tid=" << (int)QThread::currentThread() << endl;
  #endif
  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    (*i)->readUnlock();
  }
  for (KstVectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    (*i)->readUnlock();
  }
  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    (*i)->readUnlock();
  }
  for (KstScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    (*i)->readUnlock();
  }
  KstObject::readUnlock();
}


// Note to self, this is nasty.  Some objects may lock while others don't,
// leading to very complex deadlocks or loss of consistency.
// Might want to guard these with another lock yet.
void KstDataObject::writeLock() const {
  #ifdef LOCKTRACE
  kdDebug() << (void*) this << " Write lock by tid=" << (int)QThread::currentThread() << endl;
  #endif
  KstRWLock::writeLock();
  for (KstScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    (*i)->writeLock();
  }
  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    (*i)->writeLock();
  }
  for (KstVectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    (*i)->writeLock();
  }
  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    (*i)->writeLock();
  }
}


void KstDataObject::writeUnlock() const {
  #ifdef LOCKTRACE
  kdDebug() << (void*)this << "Write unlock by tid=" << (int)QThread::currentThread() << endl;
  #endif
  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    (*i)->writeUnlock();
  }
  for (KstVectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    (*i)->writeUnlock();
  }
  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    (*i)->writeUnlock();
  }
  for (KstScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    (*i)->writeUnlock();
  }
  KstRWLock::writeUnlock();
}

#include "kstdataobject.moc"
// vim: ts=2 sw=2 et
