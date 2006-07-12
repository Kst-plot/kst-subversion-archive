/***************************************************************************
                    kststring.cpp  -  the base string type
                             -------------------
    begin                : Sept 29, 2004
    copyright            : (C) 2004 by The University of Toronto
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

#include <qstylesheet.h>

#include <klocale.h>

static int anonymousStringCounter = 0;

KstString::KstString(const QString& in_tag, KstObject *provider, const QString& val, bool orphan, bool doLock)
: KstPrimitive(provider), _value(val), _orphan(orphan) {
  QString _tag = in_tag;
  if (_tag.isEmpty()) {
    QString nt = i18n("Anonymous String %1");

    do {
      _tag = nt.arg(anonymousStringCounter++);
    } while (KstData::self()->vectorTagNameNotUniqueInternal(_tag));
  } else {
    while (KstData::self()->vectorTagNameNotUniqueInternal(_tag)) {
      _tag += '\'';
    }
  }
  setTagName(_tag);

  if (doLock) {
    KST::stringList.lock().writeLock();
  }
  KST::stringList.append(this);
  if (doLock) {
    KST::stringList.lock().writeUnlock();
  }
}


KstString::KstString(QDomElement& e)
: KstPrimitive(), _orphan(false) {
  QDomNode n = e.firstChild();

  while (!n.isNull()) {
    QDomElement e = n.toElement();
    if (!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "orphan") {
        _orphan = true;
      } else if (e.tagName() == "value") {
        setValue(e.text());
      }
    }
    n = n.nextSibling();
  }
  KST::stringList.append(this);
}


KstString::~KstString() {
}


void KstString::save(QTextStream &ts, const QString& indent) {
  ts << indent << "<tag>" << QStyleSheet::escape(tagName()) << "</tag>" << endl;
  if (_orphan) {
    ts << indent << "<orphan/>" << endl;
  }
  ts << indent << "<value>" << QStyleSheet::escape(value()) << "</value>" << endl;
}


KstObject::UpdateType KstString::update(int updateCounter) {
  bool force = dirty();
  setDirty(false);

  if (KstObject::checkUpdateCounter(updateCounter) && !force) {
    return lastUpdateResult();
  }

  QString v = value();
  if (_provider) {
    _provider->update(updateCounter);
  }
  
  return setLastUpdateResult(v == value() ? NO_CHANGE : UPDATE);
}


KstString& KstString::operator=(const QString& v) {
  setValue(v);
  return *this;
}


KstString& KstString::operator=(const char *v) {
  setValue(v);
  return *this;
}


void KstString::setValue(const QString& inV) {
  setDirty();
  _value = inV;
  emit trigger();
}

#include "kststring.moc"
// vim: ts=2 sw=2 et
