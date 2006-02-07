/***************************************************************************
                         planck.cpp  -  Part of KST
                             -------------------
    begin                : Mon Oct 06 2003
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


#include "planckdata.h"

using namespace Planck;


namespace Planck {
bool havePlanck() {
#ifdef KST_HAVE_PLANCK
  return true;
#else
  return false;
#endif
}
}

Source::Source() : KstShared() {
  _isValid = false;
}


Source::~Source() {
}


bool Source::setSource(const QString& src) {
  Q_UNUSED(src)
  return false;
}


void Source::reset() {
  _isValid = false;
}


bool Source::isValid() const {
  return _isValid;
}


QStringList Source::fields(const QString& group) const {
  Q_UNUSED(group)
  return QStringList();
}


int Source::readObject(const QString& group, const QString& object, double *buf, long start, long end) {
  Q_UNUSED(group)
  Q_UNUSED(object)
  Q_UNUSED(buf)
  Q_UNUSED(start)
  Q_UNUSED(end)
  return 0;
}


QSize Source::range(const QString& group, const QString& object) const {
  Q_UNUSED(group)
  Q_UNUSED(object)
  return QSize();
}

// vim: ts=2 sw=2 et
