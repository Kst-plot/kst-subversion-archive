/***************************************************************************
                            kstfilteredvector.cpp
                             -------------------
    begin                : Mon Dec 15 2003
    copyright            : (C) 2003 by The University of Toronto
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

#include "kstfilteredvector.h"

#include "kstdatacollection.h"

#include <kdebug.h>


KstFilteredVector::KstFilteredVector(KstVectorPtr parent, KstFilterSetPtr filterSet)
: KstVector(parent->tagName() + "-" + filterSet->name(), parent->length()), _parent(parent), _filterSet(filterSet) {
  // FIXME: RACE: added to global list before constructed
  update();
}


KstFilteredVector::KstFilteredVector(QDomElement& e) : KstVector() {
  // FIXME: RACE: added to global list before constructed
  QDomNode n = e.firstChild();
  while(!n.isNull()) {
    QDomElement e = n.toElement();
    if(!e.isNull()) {
      if (e.tagName() == "tag") {
        setTagName(e.text());
      } else if (e.tagName() == "parent") {
        KST::vectorList.lock().readLock();
        _parent = *KST::vectorList.findTag(e.text());
        KST::vectorList.lock().readUnlock();
        if (_parent) {
          resize(_parent->length());
        }
      } else if (e.tagName() == "filterset") {
        KST::filterSetList.lock().readLock();
        _filterSet = KST::filterSetList.find(e.text());
        KST::filterSetList.lock().readUnlock();
      }
    }
    n = n.nextSibling();
  }
}


KstFilteredVector::~KstFilteredVector() {
}


KstObject::UpdateType KstFilteredVector::update(int update_counter) {
  if (_parent && _filterSet) {
    _parent->update(update_counter);
    _filterSet->apply(_parent, KstVectorPtr(this));
  }

  KstObject::UpdateType ut = KstVector::update(update_counter);
  return ut;
}


void KstFilteredVector::save(QTextStream& ts) {
  if (!_parent || !_filterSet) {
    return;
  }

  ts << "  <tag>" << _tag << "</tag>" << endl;
  ts << "  <parent>" << _parent->tagName() << "</parent>" << endl;
  ts << "  <filterset>" << _filterSet->name() << "</filterset>" << endl;
}


// vim: ts=2 sw=2 et
