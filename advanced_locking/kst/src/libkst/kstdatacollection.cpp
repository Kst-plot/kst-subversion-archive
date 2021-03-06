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

#include <config.h>

#include <stdlib.h>

#include "kstdatacollection.h"

#include "sysinfo.h"
#include "psversion.h"

/** The list of data sources (files) */
KstDataSourceList KST::dataSourceList;

/** The list of vectors that are being read */
KstVectorList KST::vectorList;

/** The list of matrices that are being read */
KstMatrixList KST::matrixList;

/** The list of Scalars which have been generated */
KstScalarList KST::scalarList;

/** The list of Strings */
KstStringList KST::stringList;

void KST::addVectorToList(KstVectorPtr v) {
  KST::vectorList.lock().writeLock();
  KST::vectorList.append(v);
  KST::vectorList.lock().unlock();
}

void KST::addMatrixToList(KstMatrixPtr m) {
  KST::matrixList.lock().writeLock();
  KST::matrixList.append(m);
  KST::matrixList.lock().unlock();  
}

static QMutex bigLock;

void *KST::realloc(void *ptr, size_t size) {
#ifdef HAVE_LINUX
  QMutexLocker ml(&bigLock);
  meminfo();
  unsigned long bFree = S(kb_main_free + kb_main_buffers + kb_main_cached);
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
  QMutexLocker ml(&bigLock);
  meminfo();
  unsigned long bFree = S(kb_main_free + kb_main_buffers + kb_main_cached);
  if (size > bFree) {
    const unsigned long sz = size;
    qDebug("Tried to allocate too much memory! (Wanted %lu, had %lu)", sz, bFree);
    return 0L;
  }
#endif
  return ::malloc(size);
}


static KStaticDeleter<KstData> sdData;
KstData *KstData::_self = 0L;

KstData *KstData::self() {
  if (!_self) {
    _self = sdData.setObject(_self, new KstData);
  }
  return _self;
}


void KstData::replaceSelf(KstData *newInstance) {
  delete _self;
  _self = 0L;
  _self = sdData.setObject(_self, newInstance);
}


KstData::KstData() {
}


KstData::~KstData() {
}


bool KstData::vectorTagNameNotUniqueInternal(const QString& tag) {
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
      return true;
  }

  /* verify that the tag name is not used by a data object */
  KST::vectorList.lock().readLock();
  bool vc = KST::vectorList.findTag(tag) != KST::vectorList.end();
  KST::vectorList.lock().unlock();
  if (!vc) {
    KST::scalarList.lock().readLock();
    vc = KST::scalarList.findTag(tag) != KST::scalarList.end();
    KST::scalarList.lock().unlock();
  }
  return vc;
}


bool KstData::matrixTagNameNotUniqueInternal(const QString& tag) {
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
    return true;
  }

  /* verify that the tag name is not used by a data object */
  KstReadLocker ml(&KST::matrixList.lock());
  KstReadLocker ml2(&KST::scalarList.lock());
  if (KST::matrixList.findTag(tag) != KST::matrixList.end() ||
      KST::scalarList.findTag(tag) != KST::scalarList.end()) {
    return true;
  }
  return false;  
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
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
      return true;
  }

  /* verify that the tag name is not used by a data object */
  KstReadLocker ml(&KST::vectorList.lock());
  KstReadLocker ml2(&KST::scalarList.lock());
  if (KST::vectorList.findTag(tag) != KST::vectorList.end() ||
      KST::scalarList.findTag(tag) != KST::scalarList.end()) {
      return true;
  }

  return false;
}


bool KstData::matrixTagNameNotUnique(const QString& tag, bool warn, void *p) {
  Q_UNUSED(p)
  Q_UNUSED(warn)
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
    return true;
  }

  /* verify that the tag name is not used by a data object */
  KstReadLocker ml(&KST::matrixList.lock());
  KstReadLocker ml2(&KST::scalarList.lock());
  if (KST::matrixList.findTag(tag) != KST::matrixList.end() ||
      KST::scalarList.findTag(tag) != KST::scalarList.end()) {
    return true;
  }
  return false;
}


QStringList KstData::plotList(const QString& window) {
  Q_UNUSED(window)
  return QStringList();
}


void KstData::removeCurveFromPlots(KstBaseCurve *c) {
  Q_UNUSED(c)
  // meaningless in no GUI: no plots!
}


int KstData::vectorToFile(KstVectorPtr v, QFile *f) {
  // FIXME: implement me (non-gui)
  return 0;
}


int KstData::vectorsToFile(const KstVectorList& l, QFile *f, bool interpolate) {
  // FIXME: implement me (non-gui)
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


// vim: ts=2 sw=2 et
