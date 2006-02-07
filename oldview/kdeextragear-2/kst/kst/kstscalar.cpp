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

#include <qstylesheet.h>

static int iAnonymousScalarCounter = 0;

/** Create the base scalar */
KstScalar::KstScalar(const QString& in_tag, double val, bool orphan) : KstObject(), _orphan(orphan) {
  QString _tag = in_tag;
  if (_tag.isEmpty()) {
    QString nt = i18n("Anonymous Scalar %1");
    
    do {
      _tag = nt.arg(iAnonymousScalarCounter++);
    } while (KST::vectorTagNameNotUnique(_tag, false));
  } else {
    while (KST::vectorTagNameNotUnique(_tag, false)) {
      _tag += "'";
    }
  }
  setTagName(_tag);
  _value = val;
  KST::scalarList.lock().writeLock();
  KST::scalarList.append(this);
  KST::scalarList.lock().writeUnlock();
}


KstScalar::KstScalar(QDomElement& e) : KstObject(), _orphan(false) {
  QDomNode n = e.firstChild();

  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "value") {
        setValue(e.text().toDouble());
      }
    }
    n = n.nextSibling();
  }
  KST::scalarList.append(this);
}


KstScalar::~KstScalar() {
}


KstObject::UpdateType KstScalar::update(int updateCounter) {
  Q_UNUSED(updateCounter)
    return NO_CHANGE;
}


void KstScalar::save(QTextStream &ts) {
  ts << "    <tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  ts << "    <value>" << value() << "</value>" << endl;
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


void KstScalar::setValue(double inV) {
  _value = inV;
  emit trigger();
}

#include "kstscalar.moc"
// vim: et ts=2 sw=2
