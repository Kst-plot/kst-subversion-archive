/***************************************************************************
                          kstscalar.cpp  -  the base scalar type
                             -------------------
    begin                : March 24, 2003
    copyright            : (C) 2003 by cbn
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

#include "kstscalar.h"
#include "kstdatacollection.h"
#include "kstdoc.h"
#include <klocale.h>

/** Create the base scalar */
KstScalar::KstScalar(const QString& in_tag, double val) : KstObject() {
  _tag = in_tag;
  if (_tag.isEmpty()) {
    QString nt = i18n("Anonymous Scalar %1");
    int i = 0;
    // FIXME: make me more efficient
    do {
      _tag = nt.arg(i++);
    } while (KST::vectorTagNameNotUnique(_tag, false));
  } else {
    while (KST::vectorTagNameNotUnique(_tag, false)) {
      _tag += "'";
    }
  }
  _value = val;
  KST::scalarList.lock().writeLock();
  KST::scalarList.append(this);
  KST::scalarList.lock().writeUnlock();
}


KstScalar::~KstScalar() {
}


KstObject::UpdateType KstScalar::update(int updateCounter) {
  Q_UNUSED(updateCounter);
return NO_CHANGE;
}


void KstScalar::save(QTextStream &ts) {
  Q_UNUSED(ts);
}


KstScalar& KstScalar::operator=(double v) {
  _value = v;
return *this;
}


bool KstScalar::isGlobal() const {
  KST::scalarList.lock().readLock();
  // can't use find() due to constness issues
  bool rc = KST::scalarList.findTag(tagName()) != KST::scalarList.end();
  KST::scalarList.lock().readUnlock();
  return rc;
}

// vim: et ts=2 sw=2
