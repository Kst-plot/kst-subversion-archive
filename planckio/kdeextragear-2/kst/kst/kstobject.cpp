/***************************************************************************
              kstobject.cpp: abstract base class for all Kst objects
                             -------------------
    begin                : May 25, 2003
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

#include "kstobject.h"

KstObject::KstObject()
: KShared(), QObject(), _lastUpdateCounter(0) {
}

KstObject::~KstObject() {
}

int KstObject::operator==(const QString& tag) const {
  return (tag == _tag) ? 1 : 0;
}

// Returns true if update has already been done
bool KstObject::checkUpdateCounter(int update_counter) {
  if (update_counter == _lastUpdateCounter) {
    return(true);
  } else if (update_counter > 0) {
    _lastUpdateCounter = update_counter;
  }
  return (false);
}


// vim: ts=2 sw=2 et

