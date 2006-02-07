/***************************************************************************
                         kstdatacollection-nogui.cpp
                             -------------------
    begin                : July 15, 2003
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

#include "kstdatacollection.h"
#include <qmutex.h>

bool KST::dataTagNameNotUnique(const QString &tag, bool warn) {
  Q_UNUSED(warn)
  /* verify that the tag name is not empty */
  if (tag.stripWhiteSpace().isEmpty()) {
      return true;
  }

  /* verify that the tag name is not used by a data object */
  KstReadLocker ml(&KST::dataObjectList.lock());
  if (KST::dataObjectList.findTag(tag) != KST::dataObjectList.end()) {
      return true;
  }

  return false;
}

bool KST::vectorTagNameNotUnique(const QString &tag, bool warn) {
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

