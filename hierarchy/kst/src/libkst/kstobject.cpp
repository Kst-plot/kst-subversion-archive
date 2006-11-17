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

#include "ksdebug.h"
#include "kstobject.h"

/** Tag globals */
const QString KstObjectTag::tagSeparator = QString("/");
const QStringList KstObjectTag::globalTagContext = QStringList();
const QStringList KstObjectTag::constantTagContext = KstObjectTag::globalTagContext;


static int i = 0;

KstObject::KstObject()
: KstShared(), QObject(), KstRWLock(), _lastUpdateCounter(0) {
  _dirty = false;
  _lastUpdate = KstObject::NO_CHANGE;
  _tag.setTag(tr("Object %1").arg(++i));
}


KstObject::~KstObject() {
}


int KstObject::operator==(const QString& tag) const {
  return (tag == _tag.tagString()) ? 1 : 0;
}


// Returns true if update has already been done
bool KstObject::checkUpdateCounter(int update_counter) {
  if (update_counter == _lastUpdateCounter) {
    return true;
  } else if (update_counter > 0) {
    _lastUpdateCounter = update_counter;
  }
  return false;
}


inline QString KstObject::tagName() const {
  return _tag.tagString();
}


inline KstObjectTag KstObject::tag() const {
  return _tag;
}


void KstObject::setTagName(KstObjectTag tag) {
  if (tag.tag().contains('/')) {
    kstdWarning() << "WARNING: setting KstObject tag name containing '/':\"" << tag.tag() << "\"" << endl;
    // TODO: parse out slashes
  }

  _tag = tag;
  setName(_tag.tagString().local8Bit().data());
}


void KstObject::setTagName(const QString& tag, QStringList context) {
  if (tag.contains('/')) {
    kstdWarning() << "WARNING: setting KstObject tag name containing '/':" << tag << endl;
    // TODO: parse out slashes
  }

  _tag.setTag(tag, context);
  setName(_tag.tagString().local8Bit().data());
}


QString KstObject::tagLabel() const {
  return QString("[%1]").arg(_tag.tagString());
}


int KstObject::getUsage() const {
  return _KShared_count() - 1;
}


bool KstObject::deleteDependents() {
  return false;
}


KstObject::UpdateType KstObject::setLastUpdateResult(UpdateType result) {
  return _lastUpdate = result;
}


KstObject::UpdateType KstObject::lastUpdateResult() const {
  return _lastUpdate;
}


void KstObject::setDirty(bool dirty) {
  _dirty = dirty;
}


bool KstObject::dirty() const {
  return _dirty;
}


KstObjectTreeNode::KstObjectTreeNode(const QString& tag) : _tag(tag),
                                                           _object(NULL),
                                                           _parent(NULL)
{
}

QStringList KstObjectTreeNode::fullTag() const {
  QStringList tag;
  const KstObjectTreeNode *p = this;

  while (p) {
    if (!p->_tag.isEmpty()) {
      tag.prepend(p->_tag);
    }
    p = p->_parent;
  }

  return tag;
}

KstObjectTreeNode *KstObjectTreeNode::child(const QString& tag) const {
  if (_children.contains(tag)) {
    return _children[tag];
  } else {
    return NULL;
  }
}

KstObjectTreeNode *KstObjectTreeNode::descendant(QStringList tag) {
  KstObjectTreeNode *currNode = this;
  for (QStringList::ConstIterator i = tag.begin(); i != tag.end(); ++i) {
    currNode = currNode->child(*i);
    if (!currNode) {
      return NULL;
    }
  }

  return currNode;
}

bool KstObjectTreeNode::addDescendant(KstObject *o) {
  if (!o) {
    return false;
  }

  QStringList tag = o->tag().fullTag();

  KstObjectTreeNode *currNode = this;
  for (QStringList::ConstIterator i = tag.begin(); i != tag.end(); ++i) {
    KstObjectTreeNode *nextNode = currNode->child(*i);
    if (!nextNode) {
      nextNode = new KstObjectTreeNode(*i);
      nextNode->_parent = currNode;
      currNode->_children[*i] = nextNode;
    }
    currNode = nextNode;
  }

  if (currNode->_object) {
    return false;
  } else {
    currNode->_object = o;
    kstdDebug() << "Added KstObject to naming tree:" << o->tag().tagString() << endl;
    return true;
  }
}

bool KstObjectTreeNode::removeDescendant(KstObject *o) {
  if (!o) {
    return false;
  }

  QStringList tag = o->tag().fullTag();

  KstObjectTreeNode *currNode = this;
  for (QStringList::ConstIterator i = tag.begin(); i != tag.end(); ++i) {
    KstObjectTreeNode *nextNode = currNode->child(*i);
    if (!nextNode) {
      return false;
    }
    currNode = nextNode;
  }

  if (currNode->_object != QGuardedPtr<KstObject>(o)) {
    return false;
  } else {
    currNode->_object = NULL;
    QStringList::ConstIterator i = tag.end();
    while (i != tag.begin() && currNode->_object.isNull()) {
      --i;
      KstObjectTreeNode *lastNode = currNode->_parent;
      lastNode->_children.remove(*i);
      delete currNode;
      currNode = lastNode;
    }
    return true;
  }
}


// vim: ts=2 sw=2 et
