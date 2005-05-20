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

/** The list of Scalars which have been generated */
KstScalarList KST::scalarList;

/** The list of Strings */
KstStringList KST::stringList;

/** The list of data objects which are in use */
KstDataObjectList KST::dataObjectList;

void KST::addVectorToList(KstVectorPtr v) {
  KST::vectorList.lock().writeLock();
  KST::vectorList.append(v);
  KST::vectorList.lock().writeUnlock();
}

void KST::addDataObjectToList(KstDataObjectPtr d) {
  KST::dataObjectList.lock().writeLock();
  KST::dataObjectList.append(d);
  KST::dataObjectList.lock().writeUnlock();
}

static QMutex bigLock;

void *KST::realloc(void *ptr, size_t size) {
#ifdef HAVE_LINUX
  QMutexLocker ml(&bigLock);
  meminfo();
  unsigned long bFree = S(kb_main_free + kb_main_buffers + kb_main_cached);
  if (size > bFree) {
    qDebug("Tried to allocate too much memory! (Wanted %u, had %lu)", size, bFree);
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
    qDebug("Tried to allocate too much memory! (Wanted %u, had %lu)", size, bFree);
    return 0L;
  }
#endif
  return ::malloc(size);
}


bool KST::vectorTagNameNotUniqueInternal(const QString& tag) {
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

// vim: ts=2 sw=2 et
