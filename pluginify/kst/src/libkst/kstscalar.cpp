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
#include <klocale.h>
#include "ksdebug.h"

#include <qstylesheet.h>

static int iAnonymousScalarCounter = 0;

static bool dirtyScalars = false;

bool KstScalar::scalarsDirty() {
  // Should use a mutex, but let's play with fire to be fast
  return dirtyScalars;
}


void KstScalar::clearScalarsDirty() {
  // Should use a mutex, but let's play with fire to be fast
  dirtyScalars = false;
}

/** Create the base scalar */
KstScalar::KstScalar(const QString& in_tag, KstObject *provider, double val, bool orphan, bool displayable, bool doLock, bool editable)
: KstPrimitive(provider), _value(val), _orphan(orphan), _displayable(displayable), _editable(editable) {
  QString _tag = in_tag;
  if (_tag.isEmpty()) {
    QString nt = i18n("Anonymous Scalar %1");

    do {
      _tag = nt.arg(iAnonymousScalarCounter++);
    } while (KstData::self()->vectorTagNameNotUniqueInternal(_tag));
  } else {
    while (KstData::self()->vectorTagNameNotUniqueInternal(_tag)) {
      _tag += '\'';
    }
  }
  setTagName(_tag);

  // FIXME: passing in a lock variable indicates a design problem
  if (doLock) {
    KST::scalarList.lock().writeLock();
  }
  KST::scalarList.append(this);
  if (doLock) {
    KST::scalarList.lock().unlock();
  }
}


KstScalar::KstScalar(const QDomElement& e)
: KstPrimitive(), _orphan(false), _displayable(true), _editable(false) {
  QDomNode n = e.firstChild();
  bool ok;

  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "orphan") {
        _orphan = true;
      } else if (e.tagName() == "value") {
        setValue(e.text().toDouble());
      } else if (e.tagName() == "editable") {
        _editable = true;
      }
    }
    n = n.nextSibling();
  }

  if (tagName().toDouble(&ok) == value() && ok) {
    _displayable = false;
  }

  KST::scalarList.append(this);
}


KstScalar::~KstScalar() {
}


KstObject::UpdateType KstScalar::update(int updateCounter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(updateCounter) && !force) {
    return lastUpdateResult();
  }

  double v = value();
  if (_provider) {
    _provider->update(updateCounter);
  } else if (force) {
    return setLastUpdateResult(UPDATE);
  }

  return setLastUpdateResult(v == value() ? NO_CHANGE : UPDATE);
}


void KstScalar::save(QTextStream &ts, const QString& indent) {
  ts << indent << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  if (_orphan) {
    ts << indent << "<orphan/>" << endl;
  }
  if (_editable) {
    ts << indent << "<editable/>" << endl;
  }
  ts << indent << "<value>" << value() << "</value>" << endl;
}


KstScalar& KstScalar::operator=(double v) {
  setValue(v);
  return *this;
}


bool KstScalar::isGlobal() const {
  KST::scalarList.lock().readLock();
  // can't use find() due to constness issues
  bool rc = KST::scalarList.findTag(tagName()) != KST::scalarList.end();
  KST::scalarList.lock().unlock();
  return rc;
}


void KstScalar::setValue(double inV) {
  if (_value != inV) {
    setDirty();
    dirtyScalars = true;
    _value = inV;
    emit trigger();
  }
}


QString KstScalar::label() const {
  return QString::number(_value);
}


double KstScalar::value() const {
  return _value;
}


bool KstScalar::orphan() const {
  return _orphan;
}


void KstScalar::setOrphan(bool orphan) {
  _orphan = orphan;
}


bool KstScalar::displayable() const {
  return _displayable;
}


void KstScalar::setDisplayable(bool displayable) {
  _displayable = displayable;
}


bool KstScalar::editable() const {
  return _editable;
}


void KstScalar::setEditable(bool editable) {
  _editable = editable;
}


#include "kstscalar.moc"
// vim: et ts=2 sw=2
