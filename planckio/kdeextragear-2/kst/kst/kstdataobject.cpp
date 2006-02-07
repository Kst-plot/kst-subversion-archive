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

#include "kstdataobject.h"
#include "kstdatacollection.h"
#include <qtimer.h>
#include <kdebug.h>

KstDataObject::KstDataObject() : KstObject() {
  //kdDebug() << "+++ CREATING DATA OBJECT: " << (void*)this << endl;
}

KstDataObject::KstDataObject(QDomElement& e) : KstObject() {
Q_UNUSED(e);
  //kdDebug() << "+++ CREATING DATA OBJECT: " << (void*)this << endl;
}

KstDataObject::~KstDataObject() {
  // Remove our slave vectors and scalars
  for (KstScalarMap::Iterator it = _outputScalars.begin();
                               it != _outputScalars.end();
                                                      ++it) {
    KST::scalarList.remove(it.data());
  }

  for (KstVectorMap::Iterator it = _outputVectors.begin();
                               it != _outputVectors.end();
                                                      ++it) {
    KST::vectorList.remove(it.data());
  }
  //kdDebug() << "+++ DESTROYING DATA OBJECT: " << (void*)this << endl;
}

double *KstDataObject::vectorRealloced(KstVectorPtr v, double *memptr, int newSize) const {
  if (v.data() == 0L) {
    return 0L;
  }

  return v->realloced(memptr, newSize);
}

void KstDataObject::save(QTextStream& ts) {
Q_UNUSED(ts);
}

void KstDataObject::loadInputs() {
  QValueList<QPair<QString,QString> >::Iterator i;
  for (i = _inputVectorLoadQueue.begin(); i != _inputVectorLoadQueue.end(); ++i) {
    KstVectorList::Iterator it = KST::vectorList.findTag((*i).second);
    if (it != KST::vectorList.end()) {
      _inputVectors.insert((*i).first, *it);
    } else {
      kdWarning() << "Unable to find required vector: " << (*i).second << endl;
    }
  }

  for (i = _inputScalarLoadQueue.begin(); i != _inputScalarLoadQueue.end(); ++i) {
    KstScalarList::Iterator it = KST::scalarList.findTag((*i).second);
    if (it != KST::scalarList.end()) {
      _inputScalars.insert((*i).first, *it);
    } else {
      kdWarning() << "Unable to find required scalar: " << (*i).second << endl;
    }
  }

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

#include "kstdataobject.moc"

// vim: ts=2 sw=2 et
