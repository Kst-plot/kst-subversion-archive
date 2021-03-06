/***************************************************************************
                            kstdatacollection.cpp
                             -------------------
    begin                : June 12, 2003
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

#include <stdlib.h>

#include "kstdatacollection.h"
#include "psversion.h"
#include "sysinfo.h"

KstDataSourceList KST::dataSourceList;
KstVectorCollection KST::vectorList;
KstMatrixCollection KST::matrixList;
KstScalarCollection KST::scalarList;
KstStringCollection KST::stringList;

static QMutex lockMeminfo;

bool KST::memfree(unsigned long& mem, bool wait) {
  bool bRetVal = false;

#ifdef HAVE_LINUX
  if (wait) {
    lockMeminfo.lock();

    meminfo();
    mem = S(kb_main_free + kb_main_buffers + kb_main_cached);
    bRetVal = true;

    lockMeminfo.unlock();
  } else {
    if (lockMeminfo.tryLock()) {
      meminfo();
      mem = S(kb_main_free + kb_main_buffers + kb_main_cached);
      bRetVal = true;

      lockMeminfo.unlock();
    }
  }
#else
  Q_UNUSED(mem)
  Q_UNUSED(wait)
#endif

  return bRetVal;
}

void *KST::realloc(void *ptr, size_t size) {
#ifdef HAVE_LINUX
  unsigned long bFree;

  memfree(bFree, true);
  if (size > bFree) {
    const unsigned long sz = size;
    qDebug("Tried to allocate too much memory! (Wanted %lu, had %lu)", sz, bFree);
    return 0L;
  }
#endif
  return ::realloc(ptr, size);
}

void *KST::malloc(size_t size) {
#ifdef HAVE_LINUX
  unsigned long bFree;

  memfree(bFree, true);
  if (size > bFree) {
    const unsigned long sz = size;
    qDebug("Tried to allocate too much memory! (Wanted %lu, had %lu)", sz, bFree);
    return 0L;
  }
#endif
  return ::malloc(size);
}


KstData *KstData::_self = 0L;

KstData *KstData::self() {
  if (!_self) {
    _self = new KstData;
  }
  return _self;
}


void KstData::replaceSelf(KstData *newInstance) {
  delete _self;
  _self = 0L;
  _self = newInstance;
}


KstData::KstData() {
}


KstData::~KstData() {
}


bool KstData::vectorTagNameNotUniqueInternal(const QString& tag) {
  bool rc = false;

  //
  // verify that the tag name is not empty
  //
  
  if (tag.trimmed().isEmpty()) {
    rc = true;
  } else {
    //
    // verify that the tag name is not used by a data object
    //
    
    KST::vectorList.lock().readLock();
    rc = KST::vectorList.tagExists(tag);
    KST::vectorList.lock().unlock();
    if (!rc) {
      KST::scalarList.lock().readLock();
      rc = KST::scalarList.tagExists(tag);
      KST::scalarList.lock().unlock();
    }
  }

  return rc;
}


bool KstData::matrixTagNameNotUniqueInternal(const QString& tag) {
  bool rc = false;
 
  //
  // verify that the tag name is not empty
  //
  
  if (tag.trimmed().isEmpty()) {
    rc = true;
  } else {
    //
    // verify that the tag name is not used by a data object
    //
  
    KstReadLocker ml(&KST::matrixList.lock());
    KstReadLocker ml2(&KST::scalarList.lock());
    
    if (KST::matrixList.tagExists(tag) || KST::scalarList.tagExists(tag)) {
      rc = true;
    }
  }

  return rc;
}


bool KstData::tagNameNotUnique(const QString& tag, bool warn, void *p) {
  Q_UNUSED(p)

  return dataTagNameNotUnique(tag, warn) || vectorTagNameNotUnique(tag, warn);
}


bool KstData::dataTagNameNotUnique(const QString& tag, bool warn, void *parent) {
  Q_UNUSED(tag)
  Q_UNUSED(warn)
  Q_UNUSED(parent)

  return false;
}


bool KstData::vectorTagNameNotUnique(const QString& tag, bool warn, void *p) {
  Q_UNUSED(p)
  Q_UNUSED(warn)

  bool rc = false;

  //
  // verify that the tag name is not empty...
  //
  
  if (tag.trimmed().isEmpty()) {
    rc = true;
  } else {
    //
    // verify that the tag name is not used by a data object...
    //
    
    KstReadLocker ml(&KST::vectorList.lock());
    KstReadLocker ml2(&KST::scalarList.lock());
  
    if (KST::vectorList.tagExists(tag) || KST::scalarList.tagExists(tag)) {
      rc = true;
    }
  }

  return rc;
}


bool KstData::matrixTagNameNotUnique(const QString& tag, bool warn, void *p) {
  Q_UNUSED(p)
  Q_UNUSED(warn)

  bool rc = false;

  //
  // verify that the tag name is not empty...
  //
  
  if (tag.trimmed().isEmpty()) {
    rc = true;
  } else {
    //
    // verify that the tag name is not used by a data object...
    //
    
    KstReadLocker ml(&KST::matrixList.lock());
    KstReadLocker ml2(&KST::scalarList.lock());
  
    if (KST::matrixList.tagExists(tag) || KST::scalarList.tagExists(tag)) {
      rc = true;
    }
  }

  return rc;
}


bool KstData::dataSourceTagNameNotUnique(const QString& tag, bool warn, void *p) {
  Q_UNUSED(p)
  Q_UNUSED(warn)

  bool rc = false;

  //
  // verify that the tag name is not empty...
  //
  
  if (tag.trimmed().isEmpty()) {
    rc = true;
  } else {
    //
    // verify that the tag name is not used by a data source...
    //
    
    KstReadLocker l(&KST::dataSourceList.lock());
  
    if (KST::dataSourceList.findTag(tag) != KST::dataSourceList.end()) {
      rc = true;
    }
  }

  return rc;
}


QStringList KstData::plotList(const QString& window) {
  Q_UNUSED(window)

  return QStringList();
}


void KstData::removeCurveFromPlots(KstBaseCurve *c) {
  Q_UNUSED(c)

  // meaningless in no GUI: no plots!
}


bool KstData::viewObjectNameNotUnique(const QString& tag) {
  Q_UNUSED(tag)

  // meaningless in no GUI: no view objects!

  return false;
}


int KstData::vectorToFile(KstVectorPtr v, QFile *f) {
  Q_UNUSED(v);
  Q_UNUSED(f);

  return 0;
}


int KstData::vectorsToFile(const KstVectorList& l, QFile *f, bool interpolate) {
  Q_UNUSED(l);
  Q_UNUSED(f);
  Q_UNUSED(interpolate);

  return 0;
}


int KstData::columns(const QString& window) {
  Q_UNUSED(window)

  return 0;
}


void KstData::newWindow(QWidget *dialogParent) {
  Q_UNUSED(dialogParent)
}


QStringList KstData::windowList() {
  return QStringList();
}


QString KstData::currentWindow() {
  return QString::null;
}
