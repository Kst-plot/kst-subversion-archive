/***************************************************************************
                          kstsvector.cpp - a vector from x0 to x1 with n pts
                             -------------------
    begin                : March, 2005
    copyright            : (C) 2005 by cbn
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

#include <QTextDocument>

#include <kdebug.h>

#include "kstsvector.h"

KstSVector::KstSVector(const QDomElement &e) : KstVector(e) {
  double x0 = 0.0;
  double x1 = 1.0;
  int num = 2;

  QDomNode n = e.firstChild();

  while (!n.isNull()) {
    QDomElement e = n.toElement();

    if (!e.isNull()) {
      if (e.tagName() == "N") {
        num = e.text().toInt();
      } else if (e.tagName() == "min") {
        x0 = e.text().toDouble();
      } else if (e.tagName() == "max") {
        x1 = e.text().toDouble();
      }
    }
    n = n.nextSibling();
  }

  _saveable = true;
  _saveData = false;
  changeRange(x0,  x1,  num);
}


KstSVector::KstSVector(double x0, double x1, int n, KstObjectTag tag) : KstVector(tag, n) {
  _saveable = true;
  _saveData = false;
  changeRange(x0, x1, n);
}


void KstSVector::save(QTextStream &ts, const QString& indent, bool saveAbsolutePosition) {
  ts << indent << "<svector>" << endl;
  KstVector::save(ts, indent + "  ", saveAbsolutePosition);
  ts << indent << "  <min>" << min() << "</min>" << endl;
  ts << indent << "  <max>" << max() << "</max>" << endl;
  ts << indent << "  <N>" << length() << "</N>" << endl;
  ts << indent << "</svector>" << endl;
}


void KstSVector::changeRange(double x0, double x1, int n) {
  if (n < 2) {
    n = 2;
  }

  if (n != length()) {
    resize(n, false);
  }

  if (x0 > x1) {
    double tx;

    tx = x0;
    x0 = x1;
    x1 = tx;
  } else if (x0 == x1) {
    x1 = x0 + 0.1;
  }

  for (int i = 0; i < n; i++) {
    _v[i] = x0 + double(i) * (x1 - x0) / double(n - 1);
  }

  _scalars["min"]->setValue(x0);
  _scalars["max"]->setValue(x1);

  internalUpdate(KstObject::UPDATE);

  setDirty(true);
}


KstObject::UpdateType KstSVector::update(int update_counter) {
  Q_ASSERT(myLockStatus() == KstRWLock::WRITELOCKED);

  bool force = dirty();

  KstObject::UpdateType baseRC = KstVector::update(update_counter);
  if (force) {
    baseRC = UPDATE;
  }

  return baseRC;
}


void KstSVector::setSaveData(bool save) {
  Q_UNUSED(save)
}
