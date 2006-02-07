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

#include <qtimer.h>

#include <kdebug.h>

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

  // Needs special locking
  v->writeLock();
  double *rc = v->realloced(memptr, newSize);
  v->writeUnlock();

  return rc;
}

void KstDataObject::save(QTextStream& ts) {
Q_UNUSED(ts);
}

void KstDataObject::loadInputs() {
  QValueList<QPair<QString,QString> >::Iterator i;
  KST::vectorList.lock().writeLock();
  for (i = _inputVectorLoadQueue.begin(); i != _inputVectorLoadQueue.end(); ++i) {
    KstVectorList::Iterator it = KST::vectorList.findTag((*i).second);
    if (it != KST::vectorList.end()) {
      _inputVectors.insert((*i).first, *it);
    } else {
      kdWarning() << "Unable to find required vector: " << (*i).second << endl;
    }
  }
  KST::vectorList.lock().writeUnlock();

  KST::scalarList.lock().writeLock();
  for (i = _inputScalarLoadQueue.begin(); i != _inputScalarLoadQueue.end(); ++i) {
    KstScalarList::Iterator it = KST::scalarList.findTag((*i).second);
    if (it != KST::scalarList.end()) {
      _inputScalars.insert((*i).first, *it);
    } else {
      kdWarning() << "Unable to find required scalar: " << (*i).second << endl;
    }
  }
  KST::scalarList.lock().writeUnlock();

  _inputVectorLoadQueue.clear();
  _inputScalarLoadQueue.clear();
  update();
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
  for (KstScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    (*i)->readUnlock();
  }
  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    (*i)->readUnlock();
  }
  for (KstVectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    (*i)->readUnlock();
  }
  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    (*i)->readUnlock();
  }
  KstObject::readUnlock();
}


void KstDataObject::writeLock() const {
  KstObject::writeLock();
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
  for (KstScalarMap::ConstIterator i = _inputScalars.begin(); i != _inputScalars.end(); ++i) {
    (*i)->writeUnlock();
  }
  for (KstScalarMap::ConstIterator i = _outputScalars.begin(); i != _outputScalars.end(); ++i) {
    (*i)->writeUnlock();
  }
  for (KstVectorMap::ConstIterator i = _inputVectors.begin(); i != _inputVectors.end(); ++i) {
    (*i)->writeUnlock();
  }
  for (KstVectorMap::ConstIterator i = _outputVectors.begin(); i != _outputVectors.end(); ++i) {
    (*i)->writeUnlock();
  }
  KstObject::writeUnlock();
}

#include "kstdataobject.moc"
// vim: ts=2 sw=2 et
